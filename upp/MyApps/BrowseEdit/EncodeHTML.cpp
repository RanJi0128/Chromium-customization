#include <RichText/RichText.h>

namespace Upp {

static String FormatClass(Index<String>& css, const String& fmt)
{
    //return " class=\"" + FormatIntAlpha(css.FindAdd(fmt) + 1) + "\"";
    return " style=\"" + fmt + "\"";
}

static String HtmlFontStyle(Font f, Font base)
{
	String style;
	if(f.GetFace() != base.GetFace())
		switch(f.GetFace()) {
		case Font::ARIAL: style = "font-family:sans-serif;"; break;
		case Font::ROMAN: style = "font-family:serif;"; break;
		case Font::COURIER: style = "font-family:monospace;"; break;
	}
	if(f.GetHeight() != base.GetHeight())
		style << Sprintf("font-size:%fpt;", f.GetHeight()/8.0);
	if(f.IsBold() != base.IsBold())
		style << (f.IsBold() ? "font-weight:bold;" : "font-weight:normal;");
	if(f.IsItalic() != base.IsItalic())
		style << (f.IsItalic() ? "font-style:italic;" : "font-style:normal;");
	if(f.IsUnderline() != base.IsUnderline())
		style << (f.IsUnderline() ? "text-decoration:underline;" : "text-decoration:none;");
	return style;
}

static String HtmlFontStyle(Font f)
{
	String style;
	switch(f.GetFace()) {
		case Font::ARIAL: style = "font-family:sans-serif;"; break;
		case Font::ROMAN: style = "font-family:serif;"; break;
		case Font::COURIER: style = "font-family:monospace;"; break;
	}
	style << Sprintf("font-size:%fpt;", f.GetHeight()/8.0);
	style << (f.IsBold() ? "font-weight:bold;" : "font-weight:normal;");
	style << (f.IsItalic() ? "font-style:italic;" : "font-style:normal;");
	style << (f.IsUnderline() ? "text-decoration:underline;" : "text-decoration:none;");
	return style;
}

static String HtmlDot(int q, Zoom z)
{
	return String().Cat() << ' ' << z * q << "px";
}

static String HtmlDotl(int q, Zoom z)
{
	return String().Cat() << ' ' << max((int)!!q, z * q) << "px";
}

static String HtmlStyleColor(Color c, const char *cl = "color")
{
	return Format(String(cl) + ":#%02x%02x%02x;", c.GetR(), c.GetG(), c.GetB());
}

static String HtmlCharStyle(const RichPara::CharFormat& cf, const RichPara::CharFormat& sf)
{
	String style;
	if(!IsNull(cf.ink) && cf.ink != sf.ink)
		style = HtmlStyleColor(cf.ink);
	if(!IsNull(cf.paper) && cf.paper != Color(0xFF,0xFF,0xFF))
		style << HtmlStyleColor(cf.paper, "background-color");
	return style + HtmlFontStyle(cf, sf);
}

static String HtmlParaStyle(const RichPara::Format& f, Zoom z)
{
	String style;
	int lm = z * f.lm;
	if(f.bullet && f.bullet != RichPara::BULLET_TEXT) {
		style << "display:list-item; list-style-type:";
		switch(f.bullet) {
		case RichPara::BULLET_NONE: style << "none"; break;
		case RichPara::BULLET_ROUND: style << "disc"; break;
		case RichPara::BULLET_ROUNDWHITE: style << "circle"; break;
		case RichPara::BULLET_BOX:
		case RichPara::BULLET_BOXWHITE: style << "square"; break;
		default: style << "disc"; break;
		}
		style << ';';
		lm += 20;
	}
	style << Sprintf("margin:%dpx %dpx %dpx %dpx;text-indent:%dpx;",
	                z * f.before,
	                z * f.rm,
	                z * f.after,
	                lm,
	                z * (f.bullet ? 0 : f.indent)
	         );
	style << "text-align:";
	switch(f.align) {
	case ALIGN_LEFT: style << "left;"; break;
	case ALIGN_RIGHT: style << "right;"; break;
	case ALIGN_CENTER: style << "center;"; break;
	case ALIGN_JUSTIFY: style << "justify;"; break;
	}
	style << decode(f.linespacing, RichPara::LSP15, "line-height:150%",
	                               RichPara::LSP20, "line-height:200%", "");
	style << HtmlStyleColor(f.ink) + HtmlFontStyle(f);
	return style;
}

static void TabBorder(String& style,
                      const char *txt,
                      int border,
                      Color bordercolor,
                      const RichTable::Format& tf,
                      Zoom z)
{
	style << "border-" << txt << ':' << HtmlDotl(border + tf.grid, z) << " solid "
	      << ColorToHtml(border ? bordercolor : tf.gridcolor) << ';';
}

struct ExtraInfo
{
	const String& outdir;
	const String& namebase;
	int count;
};


static String SaveObject (ExtraInfo& info, const RichObject& object)
{
	String data = object.Write();
	return "data:image/raw;base64, " + Base64Encode(data.Begin(), data.End());
}

static String SaveImage (ExtraInfo& info, const Image& image, bool saveToFile)
{
	if(saveToFile) {
		PNGEncoder png;
		String name; name << info.namebase << "_" << info.count++ << ".png";
		png.SaveFile(AppendFileName(info.outdir, name), image);
		return name;
	}
	else {
		// scale image by 5, not sure why this is needed
		Size sz = image.GetSize()*5;
		// get RichObject from Image then call SaveObject()
		RichObject obj = CreatePNGObject(image, sz.cx, sz.cy);
		return SaveObject(info, obj); // save object in BASE64 text
	}
}

static String doEncodeHTML(const RichTxt& text,
                           const RichStyles& styles,
                           Index<String>& css,
                           const VectorMap<String, String>& escape,
                           Zoom z, ExtraInfo& info)
{
	String html;
	for(int i = 0; i < text.GetPartCount(); i++)
	{
		if(text.IsTable(i))
		{
			const RichTable& t = text.GetTable(i);
			const RichTable::Format& tf = t.GetFormat();
			int nx = tf.column.GetCount();
			int ny = t.GetRows();

			String style;
			if(tf.before > 0) style << "margin-top:" << HtmlDot(tf.before, z) << ";";
			if(tf.after > 0) style << "margin-bottom:" << HtmlDot(tf.after, z) << ";";

			bool margin = tf.lm > 0 || tf.rm > 0;
			if(margin) {
				html << "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"";
				if(!style.IsEmpty()) { html << FormatClass(css, style); style=""; }
				html << "><tr>";
				if(tf.lm > 0) html << "<td width=" << HtmlDot(tf.lm, z) << "></td>";
				html << "<td style=\"padding:0;\">\r\n";
			}

			style << "border-collapse:collapse;table-layout:auto;"
			      << "border:" << HtmlDotl(tf.frame, z) << " solid " << ColorToHtml(tf.framecolor) << ';';
			html << "<table width=\"100%\"" << FormatClass(css, style) << ">\r\n";

			int sum = 0;
			for(int i = 0; i < nx; i++)
				sum += tf.column[i];
			html << "<colgroup>";
			for(int i = 0; i < nx; i++)
				html << "<col width=\"" << 100 * tf.column[i] / sum << "%\">";
			html << "</colgroup>";
			html << "\r\n";

			for(int i = 0; i < ny; i++) {
				const Array<RichCell>& r = t[i];
				html << "<tr>";
				for(int j = 0; j < r.GetCount(); j++) {
					if(t(i, j)) {
						const RichCell& c = r[j];
						const RichCell::Format& cf = c.format;
						String style;

						style << "padding:" << HtmlDot(cf.margin.top, z) << HtmlDot(cf.margin.right, z)
						                    << HtmlDot(cf.margin.bottom, z) << HtmlDot(cf.margin.left, z) << ';';

						#ifdef MULTITABLE
						TabBorder(style, "top"   , cf.border.top   , cf.bordercolorT, tf, z);
						TabBorder(style, "right" , cf.border.right , cf.bordercolorR, tf, z);
						TabBorder(style, "bottom", cf.border.bottom, cf.bordercolorB, tf, z);
						TabBorder(style, "left"  , cf.border.left  , cf.bordercolorL, tf, z);
						#else
						TabBorder(style, "top"   , cf.border.top   , cf.bordercolor, tf, z);
						TabBorder(style, "right" , cf.border.right , cf.bordercolor, tf, z);
						TabBorder(style, "bottom", cf.border.bottom, cf.bordercolor, tf, z);
						TabBorder(style, "left"  , cf.border.left  , cf.bordercolor, tf, z);
						#endif

						style << "background-color:" << ColorToHtml(cf.color) << ';';
						#ifdef MULTITABLE
						if(!cf.image.IsEmpty())
							style << "background-size:100\% 100\%; background-image:url('"
							      << SaveImage(info, cf.image, true) << "');";
						#endif

						style << "vertical-align:";
						switch(cf.align) {
						case ALIGN_TOP:    style << "top"; break;
						case ALIGN_CENTER: style << "middle"; break;
						case ALIGN_BOTTOM: style << "bottom"; break;
						}
						style << ';';

						if(cf.minheight>0) style << "height:" << HtmlDot(cf.minheight, z) << ";";

						html << "<td" << FormatClass(css, style);
						if(c.hspan) html << " colspan=" << c.hspan + 1;
						if(c.vspan) html << " rowspan=" << c.vspan + 1;
						html << '>';

						html << doEncodeHTML(c.text, styles, css, escape, z, info);
						html << "</td>";
					}
				}
				html << "</tr>\r\n";
			}
			html << "</table>\r\n";
			if(margin) {
				html << "</td>";
				if(tf.rm > 0) html << "<td width=" << HtmlDot(tf.rm, z) << "></td>";
				html << "</tr></table>\r\n";
			}
		}
		else
		if(text.IsPara(i))
		{
			RichPara p = text.Get(i, styles);

			if(p.format.ruler) html << "<hr>";

			bool bultext = false;
			if(p.format.bullet == RichPara::BULLET_TEXT)
				for(int i = 0; i < p.part.GetCount(); i++) {
					const RichPara::Part& part = p.part[i];
					if(part.text.Find(9) >= 0) {
						bultext = true;
						break;
					}
				}
			if(bultext) {
				html << "<table width=\"100%\" border=\"0\" "
				        "cellpadding=\"2\" cellspacing=\"2\">"
				        "<tr>";
				int q = z * p.format.lm - 8;
				if(q > 0)
					html << Format("<td width=\"%d\"></td>", q);
				html << Format("<td valign=\"top\" width=\"%d\" bgcolor=\"#F0F0F0\">\r\n",
				               max(z * p.format.indent, 0));
				p.format.ruler = p.format.after = p.format.before = p.format.indent = p.format.lm = 0;
			}

			String par = "<p" + FormatClass(css, HtmlParaStyle(p.format, z)) + ">";
			html << par;
			bool spc = true;

			for(int i = 0; i < p.part.GetCount(); i++)
			{
				const RichPara::Part& part = p.part[i];
				if(part.object) {
					spc = false;
					Size sz = part.object.GetSize();
					sz /= 5;
					html << "<img width=\"" << sz.cx << "\" height=\"" << sz.cy << "\" src=\""
					     << SaveObject(info, part.object) << "\">";
				}
				else {
					String endtag="";

					String str = ToUtf8(part.format.indexentry);
					if(!str.IsEmpty()) {
						html << "<a name=\"" << str << "\">";
						endtag = "</a>" + endtag;
					}
					str = part.format.link;
					if(!str.IsEmpty() && str[0] != ':') {
						html << "<a href=\"" << str << "\">";
						endtag = "</a>" + endtag;
					}
					String cs;
					if(part.text[0] != 9)
						cs = HtmlCharStyle(part.format, p.format);
					if(!cs.IsEmpty()) {
						html << "<span" << FormatClass(css, cs) << ">";
						endtag = "</span>" + endtag;
					}
					if(part.format.sscript == 1) {
						html << "<sup>";
						endtag = "</sup>" + endtag;
					}
					if(part.format.sscript == 2) {
						html << "<sub>";
						endtag = "</sub>" + endtag;
					}
					if(part.format.IsStrikeout()) {
						html << "<strike>";
						endtag = "</strike>" + endtag;
					}
					if(part.format.capitals) {
						html << "<span style=\"font-variant: small-caps;\">";
						endtag << "</span>";
					}

					const wchar *end = part.text.End();
					for(const wchar *s = part.text.Begin(); s != end; s++)
					{
						if(*s == ' ') {
							html.Cat(spc ? "&nbsp;" : " ");
							spc = true;
						}
						else {
							spc = false;
							if(*s == 160)  html.Cat("&nbsp;");
							else
							if(*s == '<')  html.Cat("&lt;");
							else
							if(*s == '>')  html.Cat("&gt;");
							else
							if(*s == '&')  html.Cat("&amp;");
							else
							if(*s == '\"') html.Cat("&quot;");
							else
							if(*s == 9) {
								if(bultext) {
									if(!cs.IsEmpty() && part.text[0] != 9)
										html << "</span>";
									html << "</p>";
									html << "</td>\r\n<td valign=\"top\" bgcolor=\"#F0F0F0\">\r\n";
									html << par;
									if(s[1]) {
										cs = HtmlCharStyle(part.format, p.format);
										if(!cs.IsEmpty())
											html << "<span" << FormatClass(css, cs) << ">";
									}
								}
								else html.Cat("&nbsp;&nbsp;&nbsp;&nbsp;");
							}
							else html.Cat(ToUtf8(*s));
						}
					}
					html << endtag;
				}
			}
			if(p.part.GetCount()==0			// if an empty paragraph
			&& i+1 < text.GetPartCount())	// and is not the last one
				html << "<br>";				// then mark as new line
			html << "</p>";
			if(bultext)
				html << "</td></tr></table>";
			html << "\r\n";
		}
	}
	return html;
}



String EncodeHTML(const RichText& richText, const char* filePathName)
{
	Index<String> css;
	VectorMap<String, String> escape;

	Zoom z = Zoom(8, 40);
	String outdir = GetFileDirectory(filePathName);
	String namebase = GetFileName(filePathName);
	ExtraInfo info = {outdir, namebase, 0};

	String body = doEncodeHTML(richText, richText.GetStyles(), css, escape, z, info);

	String style;
	for(int i = 0; i < css.GetCount(); i++) {
		style << "." + FormatIntAlpha(i + 1);
		style << "{" << css[i] << "}\r\n";
	}
	return "<!DOCTYPE html>\n<html>\n<head>\n"
	       "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
	       "<style>\n" + style + "</style>\n"
	       "</head>\n<body>\n" + body +
	       "</body>\n</html>\n";
}

}
