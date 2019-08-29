#include "ide.h"

#define LLOG(x) // DLOG(x)

void Ide::ResolveUvsConflict() {
	String result;
	editor.NextUndo();
	bool copy = true;
	for(int i = 0; i < editor.GetLineCount(); i++) {
		String ln = editor.GetUtf8Line(i);
		if(strncmp(ln, "$uvs: ", 6) == 0) {
			ln = ln.Mid(6);
			if(ln == "YOUR DELETE")
				copy = false;
			else
			if(ln == "END YOUR DELETE")
				copy = true;
			else
			if(ln == "REPOSITORY DELETE")
				copy = false;
			else
			if(ln == "END REPOSITORY DELETE")
				copy = true;
			else
			if(ln != "REPOSITORY INSERT" &&
			   ln != "YOUR INSERT" &&
			   ln != "END YOUR INSERT" &&
			   ln != "END REPOSITORY INSERT" &&
			   ln != "PENDING CONFLICT") {
				Exclamation("Cannot resolve uvs conflict -&conflicting modifications found");
				editor.SetCursor(editor.GetPos64(i));
				return;
			}
		}
		else
		if(copy)
			result << ln << "\r\n";
	}
	editor.SelectAll();
	editor.Paste(result.ToWString());
}

void Ide::GotoPos(String path, int line)
{
	LLOG("GotoPos " << path << ':' << line);
	if(path.GetCount()) {
		AddHistory();
		if(IsDesignerFile(path))
			DoEditAsText(path);
		EditFile(path);
	}
	editor.SetCursor(editor.GetPos64(line - 1));
	editor.TopCursor(4);
	editor.SetFocus();
	AddHistory();
}

void Ide::GotoCpp(const CppItem& pos)
{
	GotoPos(GetSourceFilePath(pos.file), pos.line);
}

void Ide::CheckCodeBase()
{
	InvalidateFileTimeCache();
	CodeBaseSync();
}

void Ide::RescanCode()
{
/*
	TimeStop tm;
	for(int i = 0; i < 10; i++)
		ReQualifyCodeBase();
	LOG(tm);
	PutConsole(AsString(tm));
//*/
//*
	SaveFile();
	TimeStop t;
	console.Clear();
	RescanCodeBase();
	SyncRefsShowProgress = true;
	SyncRefs();
	editor.SyncNavigator();
//*/
}

void Ide::OpenTopic(const String& topic, const String& createafter, bool before)
{
	TopicLink tl = ParseTopicLink(topic);
	if(tl) {
		EditFile(AppendFileName(PackageDirectory(tl.package), tl.group + ".tpp"));
		if(designer) {
			TopicEditor *te = dynamic_cast<TopicEditor *>(&designer->DesignerCtrl());
			if(te)
				te->GoTo(tl.topic, tl.label, createafter, before);
		}
	}
}

void Ide::OpenTopic(const String& topic)
{
	OpenTopic(topic, String(), false);
}

void Ide::OpenATopic()
{
	String t = doc.GetCurrent();
	if(!t.StartsWith("topic:"))
		return;
	OpenTopic(t);
}

void Ide::IdeFlushFile()
{
	FlushFile();
}

void Ide::IdeOpenTopicFile(const String& file)
{
	EditFile(GetFileFolder(file));
	if(designer) {
		TopicEditor *te = dynamic_cast<TopicEditor *>(&designer->DesignerCtrl());
		if(te)
			te->GoTo(GetFileTitle(file), "", "", false);
	}
}

struct FileStat {
	int  count;
	int  len;
	int  lines;
	int  oldest;
	int  newest;
	int  days;

	void Add(const FileStat& a) {
		count += a.count;
		len += a.len;
		lines += a.lines;
		oldest = max(a.oldest, oldest);
		newest = min(a.newest, newest);
		days += a.days;
	}

	FileStat() { count = 0; len = lines = 0; oldest = 0; newest = INT_MAX; days = 0; }
};

String StatLen(int len)
{
	return Format("%d.%d KB", len >> 10, (len & 1023) / 103);
}

String StatDate(int d)
{
	return String().Cat() << d << " days";
}

void sPut(const String& name, String& qtf, const FileStat& fs)
{
	qtf << "::@W " << DeQtf(Nvl(name, ".<none>"))
	    << ":: [> " << fs.count
	    << ":: " << fs.lines
	    << ":: " << (fs.count ? fs.lines / fs.count : 0)
	    << ":: " << StatLen(fs.len)
	    << ":: " << StatLen(fs.len ? fs.len / fs.count : 0)
	    << ":: " << StatDate(fs.oldest)
	    << ":: " << StatDate(fs.newest)
	    << ":: " << (fs.count ? fs.days / fs.count : 0) << " days]";
}

void sPut(String& qtf, ArrayMap<String, FileStat>& pfs, ArrayMap<String, FileStat>& all) {
	FileStat pall;
	for(int i = 0; i < pfs.GetCount(); i++) {
		FileStat& fs = pfs[i];
		sPut(pfs.GetKey(i), qtf, fs);
		pall.Add(fs);
		all.GetAdd(pfs.GetKey(i)).Add(fs);
	}
	sPut("All files", qtf, pall);
	qtf << "}}&&";
}


void ShowQTF(const String& qtf, const char *title)
{
	RichText txt = ParseQTF(qtf);
	ClearClipboard();
	AppendClipboard(ParseQTF(qtf));

	WithStatLayout<TopWindow> dlg;
	CtrlLayoutOK(dlg, title);
	dlg.stat = qtf;
	dlg.Sizeable().Zoomable();
	dlg.Run();
}

void Ide::Licenses()
{
	Progress pi;
	const Workspace& wspc = IdeWorkspace();
	pi.SetTotal(wspc.GetCount());
	VectorMap<String, String> license_package;
	for(int i = 0; i < wspc.GetCount(); i++) {
		String n = wspc[i];
		pi.SetText(n);
		if(pi.StepCanceled()) return;
		String l = LoadFile(SourcePath(n, "Copying"));
		if(l.GetCount())
			MergeWith(license_package.GetAdd(l), ", ", n);
	}
	if(license_package.GetCount() == 0) {
		Exclamation("No license files ('Copying') have been found.");
		return;
	}
	String qtf;
	for(int i = 0; i < license_package.GetCount(); i++) {
		bool m = license_package[i].Find(',') >= 0;
		qtf << (m ? "Packages [* \1" : "Package [* \1")
		    << license_package[i]
		    << (m ? "\1] have" : "\1] has")
		    << " following licence notice:&"
		    << "{{@Y [C1 " << DeQtf(license_package.GetKey(i)) << "]}}&&";
	}
	
	ShowQTF(qtf, "Licenses");
}

void Ide::Statistics()
{
	Vector< ArrayMap<String, FileStat> > stat;
	Progress pi;
	const Workspace& wspc = IdeWorkspace();
	pi.SetTotal(wspc.GetCount());
	Date now = GetSysDate();
	for(int i = 0; i < wspc.GetCount(); i++) {
		const Package& pk = wspc.GetPackage(i);
		String n = wspc[i];
		pi.SetText(n);
		if(pi.StepCanceled()) return;
		ArrayMap<String, FileStat>& pfs = stat.Add();
		for(int i = 0; i < pk.GetCount(); i++)
			if(!pk[i].separator) {
				String file = SourcePath(n, pk[i]);
				if(FileExists(file)) {
					FileStat& fs = pfs.GetAdd(GetFileExt(file));
					int d = minmax(now - FileGetTime(file), 0, 9999);
					fs.oldest = max(d, fs.oldest);
					fs.newest = min(d, fs.newest);
					String data = LoadFile(file);
					for(const char *s = data; *s; s++)
						if(*s == '\n')
							fs.lines++;
					fs.len += data.GetCount();
					fs.days += d;
					fs.count++;
				}
			}
	}
	String qtf = "[1 ";
	ArrayMap<String, FileStat> all;
	String tab = "{{45:20:25:20:35:30:30:30:30@L [* ";
	String hdr = "]:: [= Files:: Lines:: - avg.:: Length:: - avg.:: Oldest:: Newest:: Avg. age]";
	for(int i = 0; i < wspc.GetCount(); i++) {
		qtf << tab << DeQtf(wspc[i]) << hdr;
		sPut(qtf, stat[i], all);
	}

	qtf << tab << "All packages" << hdr;
	sPut(qtf, all, all);

	ShowQTF(qtf, "Statistics");
}

String FormatElapsedTime(double run)
{
	String rtime;
	double hrs = floor(run / 3600);
	if(hrs > 0)
		rtime << NFormat("%0n hours, ", hrs);
	int minsec = fround(run - 3600 * hrs);
	int min = minsec / 60, sec = minsec % 60;
	if(min || hrs)
		rtime << NFormat("%d min, ", min);
	rtime << NFormat("%d sec", sec);
	return rtime;
}

void Ide::AlterText(WString (*op)(const WString& in))
{
	if(designer || !editor.IsSelection() || editor.IsReadOnly())
		return;
	editor.NextUndo();
	WString w = editor.GetSelectionW();
	editor.RemoveSelection();
	int l = editor.GetCursor();
	editor.Paste((*op)(w));
	editor.SetSelection(l, editor.GetCursor64());
}

void Ide::TextToUpper()
{
	AlterText(UPP::ToUpper);
}

void Ide::TextToLower()
{
	AlterText(UPP::ToLower);
}

void Ide::TextToAscii()
{
	AlterText(UPP::ToAscii);
}

void Ide::TextInitCaps()
{
	AlterText(UPP::InitCaps);
}

static WString sSwapCase(const WString& s)
{
	WStringBuffer r;
	r.SetCount(s.GetCount());
	for(int i = 0; i < s.GetCount(); i++)
		r[i] = IsUpper(s[i]) ? ToLower(s[i]) : ToUpper(s[i]);
	return r;
}

void Ide::SwapCase()
{
	AlterText(sSwapCase);
}

static WString sCString(const WString& s)
{
	return AsCString(s.ToString()).ToWString();
}

void Ide::ToCString()
{
	AlterText(sCString);
}

static WString sComment(const WString& s) 
{
	return "/*" + s + "*/";
}

void Ide::ToComment()
{
	AlterText(sComment);
}

static WString sCommentLines(const WString& s)
{
	String r;
	StringStream ss(s.ToString());
	for(;;) {
		String line = ss.GetLine();
		if(ss.IsError())
			return s;
		else
		if(!line.IsVoid())
			r << "//" << line << "\n";
		if(ss.IsEof())
			break;
	}
	return r.ToWString();
}

void Ide::CommentLines()
{
	AlterText(sCommentLines);
}

static WString sUncomment(const WString& s) 
{
	WString h = s;
	h.Replace("/*", "");
	h.Replace("//", "");
	h.Replace("*/", "");
	return h;
}

void Ide::UnComment()
{
	AlterText(sUncomment);
}

void Ide::ReformatComment()
{
	editor.ReformatComment();
}

void Ide::Times()
{
	WithStatisticsLayout<TopWindow> statdlg;
	CtrlLayout(statdlg, "Elapsed times");
	statdlg.SetTimeCallback(-1000, statdlg.Breaker(IDRETRY), 50);
	do
	{
		int session_time = int(GetSysTime() - start_time);
		int idle_time = int(session_time - editor.GetStatEditTime() - stat_build_time);
		statdlg.session_time <<= FormatElapsedTime(session_time);
		statdlg.edit_time    <<= FormatElapsedTime(editor.GetStatEditTime());
		statdlg.build_time   <<= FormatElapsedTime(stat_build_time);
		statdlg.idle_time    <<= FormatElapsedTime(idle_time);
	}
	while(statdlg.Run() == IDRETRY);
}

INITBLOCK {
	RegisterGlobalConfig("svn-msgs");
}

void RepoSyncDirs(const Vector<String>& working)
{
	if(!CheckSvn())
		return;
	Ptr<Ctrl> f = Ctrl::GetFocusCtrl();
	RepoSync repo;
	String msgs;
	LoadFromGlobal(msgs, "svn-msgs");
	repo.SetMsgs(msgs);
	for(int i = 0; i < working.GetCount(); i++)
		repo.Dir(working[i]);
	repo.DoSync();
	msgs = repo.GetMsgs();
	StoreToGlobal(msgs, "svn-msgs");
	if(f)
		f->SetFocus();
}

void Ide::SyncRepoDirs(const Vector<String>& working)
{
	SaveFile();
	RepoSyncDirs(working);
	ScanWorkspace();
	SyncWorkspace();
}

void Ide::SyncRepo(){
	Vector<String> d = RepoDirs();
	if(d.GetCount())
		SyncRepoDirs(d);
	else
		SyncRepoDirs(RepoDirs(true));
}

void Ide::SyncRepoDir(const String& working)
{
	SyncRepoDirs(Vector<String>() << working);
}

void Ide::GotoDirDiffLeft(int line, DirDiffDlg *df)
{
	EditFile(df->GetLeftFile());
	editor.SetCursor(editor.GetPos64(line));
	editor.SetFocus();
}

void Ide::GotoDirDiffRight(int line, DirDiffDlg *df)
{
	EditFile(df->GetRightFile());
	editor.SetCursor(editor.GetPos64(line));
	editor.SetFocus();
}

void Ide::DoDirDiff()
{
	Index<String> dir;
	Vector<String> d = GetUppDirs();
	for(int i = 0; i < d.GetCount(); i++)
		dir.FindAdd(d[i]);
	FindFile ff(ConfigFile("*.bm"));
	while(ff) {
		VectorMap<String, String> var;
		LoadVarFile(ff.GetPath(), var);
		Vector<String> p = Split(var.Get("UPP", String()), ';');
		for(int i = 0; i < p.GetCount(); i++)
			dir.FindAdd(p[i]);
		ff.Next();
	}
	String n = GetFileFolder(editfile);
	if(n.GetCount())
		dir.FindAdd(n);
	SortIndex(dir);
	
	static DirDiffDlg dlg;
	dlg.diff.WhenLeftLine = THISBACK1(GotoDirDiffLeft, &dlg);
	dlg.diff.WhenRightLine = THISBACK1(GotoDirDiffRight, &dlg);
	for(int i = 0; i < dir.GetCount(); i++) {
		dlg.Dir1AddList(dir[i]);
		dlg.Dir2AddList(dir[i]);
	}
	if(d.GetCount())
		dlg.Dir1(d[0]);
	if(!dlg.IsOpen()) {
		dlg.SetFont(veditorfont);
		dlg.Maximize();
		dlg.Title("Compare directories");
		dlg.OpenMain();
	}
	else
		dlg.SetFocus();
}

void Ide::AsErrors()
{
	ClearErrorsPane();
	SetBottom(BERRORS);
	String s = editor.IsSelection() ? editor.GetSelection() : editor.Get();
	StringStream ss(s);
	while(!ss.IsEof())
		ConsoleLine(ss.GetLine(), true);
	SetErrorEditor();
}

void Ide::RemoveDs()
{
	if(designer || editor.IsReadOnly())
		return;
	static Index<String> ds = { "DLOG", "DDUMP", "DDUMPC", "DDUMPM", "DTIMING",
	                            "DLOGHEX", "DDUMPHEX", "DTIMESTOP", "DHITCOUNT" };
	editor.NextUndo();
	int l = 0;
	int h = editor.GetLineCount() - 1;
	int ll, hh;
	if(editor.GetSelection(ll, hh)) {
		l = editor.GetLine(ll);
		h = editor.GetLine(hh);
	}
	for(int i = h; i >= l; i--) {
		String ln = editor.GetUtf8Line(i);
		try {
			CParser p(ln);
			if(p.IsId()) {
				String id = p.ReadId();
				if(ds.Find(id) >= 0 && p.Char('(')) {
					int pos = editor.GetPos(i);
					int end = min(editor.GetLength(), editor.GetPos(i) + editor.GetLineLength(i) + 1);
					editor.Remove(editor.GetPos(i), end - pos);
				}
			}
		}
		catch(CParser::Error) {}
	}
	editor.GotoLine(l);
}

void Ide::LaunchAndroidSDKManager(const AndroidSDK& androidSDK)
{
	One<Host> host = CreateHost(false);
	IGNORE_RESULT(host->Execute(androidSDK.GetLauchSDKManagerCmd()));
}

void Ide::LaunchAndroidAVDManager(const AndroidSDK& androidSDK)
{
	One<Host> host = CreateHost(false);
	IGNORE_RESULT(host->Execute(androidSDK.GetLauchAVDManagerCmd()));
}

void Ide::LauchAndroidDeviceMonitor(const AndroidSDK& androidSDK)
{
	One<Host> host = CreateHost(false);
	IGNORE_RESULT(host->Execute(androidSDK.MonitorPath()));
}
