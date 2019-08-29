#include "BrowseEdit.h"
#include "libcef.h"

namespace Upp {

#define IMAGECLASS BrowseEditImg
#define IMAGEFILE <BrowseOrEdit/BrowseEdit.iml>
#include <Draw/iml.h>


void BrowseEdit::UserAction()
{
	useraction = true;
}

Event<>  BrowseEdit::User(Event<>  cb)
{
	cb << THISBACK(UserAction);
	return cb;
}

#define USERBACK(x) User(THISBACK(x))
#define USERBACK1(x, y) User(THISBACK1(x, y))


void BrowseEdit::BlockStyleTool(Bar& bar, int width)
{
	bar.Add(info.editable, // condition to be enabled
			blockstyle,    // target widget to put
			width);        // width of this widget
}

void BrowseEdit::FontFaceTool(Bar& bar, int width)
{
	bar.Add(info.editable, fontface, width);
}

void BrowseEdit::FontSizeTool(Bar& bar, int width)
{
	bar.Add(info.editable, fontsize, width);
}

void BrowseEdit::BoldTool(Bar& bar, dword key)
{
	bar.Add(info.editable,         // condition to be enabled
			t_("Bold"),            // hint text
	        BrowseEditImg::Bold(), // toolbar icon
	        USERBACK(Bold))        // callback method on user selection
	   .Check(info.bold)           // condition to be in checked state
	   .Key(key); // key combination that will also select this option
}

void BrowseEdit::ItalicTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Italic"),
            BrowseEditImg::Italic(),
	        USERBACK(Italic))
	   .Check(info.italic)
	   .Key(key);
}

void BrowseEdit::UnderlineTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Underline"),
	        BrowseEditImg::Underline(),
	        USERBACK(Underline))
	   .Check(info.underline)
	   .Key(key);
}

void BrowseEdit::StrikeoutTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Strikeout"),
	        BrowseEditImg::Strikeout(),
	        USERBACK(Strikeout))
	   .Check(info.strikeout)
	   .Key(key);
}

void BrowseEdit::SuperscriptTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Superscript"),
	        BrowseEditImg::SuperScript(),
			USERBACK1(SetScript, 1))
	   .Check(info.superscript);
}

void BrowseEdit::SubscriptTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Subscript"),
	        BrowseEditImg::SubScript(),
			USERBACK1(SetScript, 2))
	   .Check(info.subscript);
}


void BrowseEdit::LeftTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Left"), BrowseEditImg::Left(), USERBACK(AlignLeft))
	   .Check(info.align == ALIGN_LEFT)
	   .Key(key);
}

void BrowseEdit::RightTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Right"), BrowseEditImg::Right(), USERBACK(AlignRight))
	   .Check(info.align == ALIGN_RIGHT)
	   .Key(key); // key combination that will also select this option
}

void BrowseEdit::CenterTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Center"), BrowseEditImg::Center(), USERBACK(AlignCenter))
	   .Check(info.align == ALIGN_CENTER)
	   .Key(key);
}

void BrowseEdit::JustifyTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Justify"), BrowseEditImg::Justify(), USERBACK(AlignJustify))
	   .Check(info.align == ALIGN_JUSTIFY)
	   .Key(key);
}

void  BrowseEdit::IncIndentTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Increase Indent"), BrowseEditImg::Margin(), USERBACK(IncIndent))
	   .Key(key);
}

void  BrowseEdit::DecIndentTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Decrease Indent"), BrowseEditImg::Indent(), USERBACK(DecIndent))
	   .Key(key);
}

void  BrowseEdit::BulletedTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Bulleted List"), BrowseEditImg::bullet(), USERBACK(Bulleted))
	   .Check(info.listBulleted)
	   .Key(key);
}

void  BrowseEdit::NumberedTool(Bar& bar, dword key)
{
	bar.Add(info.editable, t_("Numbered List"), BrowseEditImg::numberbullet(), USERBACK(Numbered))
	   .Check(info.listNumbered)
	   .Key(key);
}


void BrowseEdit::UndoTool(Bar& bar, dword key)
{
	bar.Add(info.undo, t_("Undo"), CtrlImg::undo(), USERBACK(Undo))
	   .Repeat()
	   .Key(K_ALT_BACKSPACE)
	   .Key(key);
}

void BrowseEdit::RedoTool(Bar& bar, dword key)
{
	bar.Add(info.redo, t_("Redo"), CtrlImg::redo(), USERBACK(Redo))
	   .Repeat()
	   .Key(K_SHIFT|K_ALT_BACKSPACE)
	   .Key(key);
}

void BrowseEdit::CutTool(Bar& bar, dword key)
{
	bar.Add(info.cut, t_("Cut"), CtrlImg::cut(), USERBACK(Cut))
	   .Key(K_SHIFT_DELETE)
	   .Key(key);
}

void BrowseEdit::CopyTool(Bar& bar, dword key)
{
	bar.Add(info.copy, t_("Copy"), CtrlImg::copy(), USERBACK(Copy))
	   .Key(K_CTRL_INSERT)
	   .Key(key);
}

void BrowseEdit::PasteTool(Bar& bar, dword key)
{
	bar.Add(info.paste, t_("Paste"), CtrlImg::paste(), USERBACK(Paste))
	   .Key(K_SHIFT_INSERT)
	   .Key(key); // key combination that will also select this option
}


void BrowseEdit::FindReplaceTool(Bar& bar, dword key)
{
	bar.Add(t_("Find"), BrowseEditImg::FindReplace(), USERBACK(OpenFindBox))
	   .Key(key); // key combination that will also select this option
}

void BrowseEdit::SpellCheckTool(Bar& bar)
{
	bar.Add(info.editable,               // condition to be enabled
			t_("Show spelling errors"),  // hint text
			BrowseEditImg::SpellCheck(), // toolbar icon
			USERBACK(SpellCheck))        // callback method on user selection
	   .Check(info.spellchecker);        // condition to be in checked state
}

void BrowseEdit::LoadImageTool(Bar& bar, dword key)
{
	bar.Add(info.editable,                  // condition to be enabled
			t_("Insert image from file.."), // hint text
			BrowseEditImg::LoadImageFile(), // toolbar icon
			THISBACK(InsertImage));         // callback method on user selection
}


// setup the toolbar with its components
void BrowseEdit::DefaultBar(Bar& bar)
{
	CutTool(bar);
	CopyTool(bar);
	PasteTool(bar);
	bar.Gap();
	UndoTool(bar);
	RedoTool(bar);
	bar.Gap();

	FindReplaceTool(bar);
	bar.Gap(4);
	SpellCheckTool(bar);
	bar.Gap(4);
	LoadImageTool(bar);
	bar.Gap();
	bar.Add(info.editable, hyperlink, INT_MAX);
	bar.Break(); // break component listing onto a newline

	BlockStyleTool(bar);
	bar.Gap();
	FontFaceTool(bar);
	bar.Gap();
	FontSizeTool(bar);
	bar.Gap();
	BoldTool(bar);
	ItalicTool(bar);
	UnderlineTool(bar);
	StrikeoutTool(bar);
	bar.Gap();
	SuperscriptTool(bar);
	SubscriptTool(bar);
	bar.Gap();
	bar.Add(info.editable, ink);
	bar.Add(info.editable, paper);
	bar.Gap();

	LeftTool(bar);
	CenterTool(bar);
	RightTool(bar);
	JustifyTool(bar);
	bar.Gap();
	IncIndentTool(bar);
	DecIndentTool(bar);
	bar.Gap();
	BulletedTool(bar);
	NumberedTool(bar);
	bar.Break(); // break component listing onto a newline

    bool b = cef_browser_CanGoBack(window);
    bool f = cef_browser_CanGoForward(window);
	bar.Add(!info.editable && b, back, 60);
	bar.Add(!info.editable && f, forward, 60);
	bar.Add(!info.editable, reload, 60);
	bar.Add(false, stop, 60);
	bar.Add(!info.editable, url, INT_MAX);
}


struct ValueDisplayFont : public Display
{
	void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		int top = r.top + (r.Height() - StdFont().Info().GetHeight()) / 2;
		w.DrawRect(r, paper); // first draw the background colour
		w.DrawText(r.left, top, (String)q, StdFont(), ink); // now draw the text
	}
};

BrowseEdit::BrowseEdit() // class constructor (initialises class variables)
{
	// create a new cef window along with an initial URL
	window = cef_window_new(this, "http://www.google.com");

	// initialise below variables to all 0s
	buffer = NULL;
	width = height = 0;
	memset(&info, 0, sizeof(info));
	topWindow = NULL;
	findBox = NULL;
	useraction = false;

	blockstyle.NoWantFocus(); // will not get focus
	blockstyle.ValueDisplay(Single<ValueDisplayFont>());
	blockstyle.SetDisplay(Single<ValueDisplayFont>());
	blockstyle <<= THISBACK(BlockStyle); // callback method on selection
	blockstyle.Tip(t_("Block style"));
	blockstyle.Add("paragraph");
	blockstyle.Add("preformatted");
	blockstyle.Add("heading 1");
	blockstyle.Add("heading 2");
	blockstyle.Add("heading 3");
	blockstyle.Add("heading 4");
	blockstyle.Add("heading 5");
	blockstyle.Add("heading 6");
	blockstyle.SetIndex(0); // select one of the options

	fontface.NoWantFocus(); // will not get focus
	fontface.ValueDisplay(Single<ValueDisplayFont>());
	fontface.SetDisplay(Single<ValueDisplayFont>());
	fontface <<= THISBACK(FontFace); // callback method on selection
	fontface.Tip(t_("Font face"));
	fontface.Add("sans-serif");
	fontface.Add("serif");
	fontface.Add("monospace");
	fontface.Add("Arial");
	fontface.Add("Times New Roman");
	fontface.Add("Courier New");
	fontface.SetIndex(1); // select one of the options

	fontsize.NoWantFocus(); // will not get focus
	fontsize.ValueDisplay(Single<ValueDisplayFont>());
	fontsize.SetDisplay(Single<ValueDisplayFont>());
	fontsize <<= THISBACK(FontSize); // callback method on selection
	fontsize.Add("x-small");
	fontsize.Add("small size");
	fontsize.Add("normal size");
	fontsize.Add("medium size");
	fontsize.Add("large size");
	fontsize.Add("x-large");
	fontsize.Add("xx-large");
	fontsize.Tip(t_("Font size"));
	fontsize.SetIndex(2); // select one of the options

	hyperlink.owner = this; // specify the BrowseEdit that owns it
	hyperlink.Tip(t_("Hyperlink (type in and press enter to set)"));
	hyperlink.SetData("(Hyperlink)");
	url.owner = this; // specify the BrowseEdit that owns it
	url.Tip(t_("URL address"));
	url.SetData("http://www.google.com");

	// specify button names and their callback methods
	back.SetLabel("Back");
	forward.SetLabel("Forward");
	reload.SetLabel("Reload");
	stop.SetLabel("Stop");
	back <<= THISBACK(OnBack);
	forward <<= THISBACK(OnForward);
	reload <<= THISBACK(OnReload);
	stop <<= THISBACK(OnStop);

	ink.ColorImage(BrowseEditImg::InkColor())        // specify toolbar component icon
	   .NullImage(BrowseEditImg::NullInkColor())
	   .StaticImage(BrowseEditImg::ColorA());
	ink.NotNull(); // disable transparency
	paper.ColorImage(BrowseEditImg::PaperColor())    // specify toolbar component icon
	     .NullImage(BrowseEditImg::NullPaperColor())
	     .StaticImage(BrowseEditImg::ColorA());
	paper.NotNull(); // disable transparency
	ink <<= THISBACK(SetInk);     // specify callback method used upon color selected
	paper <<= THISBACK(SetPaper); // specify callback method used upon color selected
	ink.Tip(t_("Text color"));
	paper.Tip(t_("Background color"));

	// specify the file types targeted by the file descriptor used by InsetImage()
	imagefs.Type("Images (*.png *.gif *.jpg *.bmp)", "*.png *.gif *.jpg *.bmp");
}

BrowseEdit::~BrowseEdit() // class destructor (free allocated resources)
{
	Close(); // first make sure the cef window is closed
	cef_window_delete(window); window=NULL; // then delete the cef window
	free(buffer); buffer=NULL; // free the pixel colour buffer
	SetupFindBox(false);
}

// close the "this->window" browser
void BrowseEdit::Close() { cef_window_close(window); }

}
