#include "BrowseEdit.h"
extern std::string HTML2QTF (std::string input, std::string directory);

namespace Upp {

#define IMAGECLASS PdfImg // used by <Draw/iml.h> below
#define IMAGEFILE  <BrowseOrEdit/PDF.iml> // get the PrintPDF toolbar image
#include <Draw/iml.h> // make the PdfImg::pdf() become available


extern String EncodeHTML(const RichText& richText, const char* filePathName);
static bool IsQTF(const char *fn) { return ToLower(GetFileExt(fn)) == ".qtf"; }

FileSel& UWordFs() { static FileSel fs; return fs; } // file descriptor used by file open/save
FileSel& PdfFs()   { static FileSel fs; return fs; } // file descriptor used by PrintPDF


class UWord : public TopWindow { // custom TopWindow class
public:
	virtual void DragAndDrop(Point, PasteClip& d);
	virtual void FrameDragAndDrop(Point, PasteClip& d);

	// callback for saving file content to disc
	void DoSaveFile(const char* content);
	static void New0(); // create a new empty untitled file content

protected:
	BrowseEdit editor; // the main widget is of BrowseEdit class
	MenuBar    menubar;
	ToolBar    toolbar;
	StatusBar  statusbar;
	String     filename; // name of the currently opened file

	static LRUList& lrufile() { static LRUList l; return l; }

	int CheckIfSaved(int after);       // check if file is saved
	void Load(const String& filename); // load a new file into BrowseEdit
	void OpenFile(const String& fn);   // open file of given file name
	void New() { New0(); }
	void Open();        // query user to select HTML file from disc
	void Save0();       // get BrowseEdit HTML content to be saved
	void Save();        // save BrowseEdit HTML content into current filename
	void SaveAs();      // query user to specify destination file for saving
	void Print();       // print current BrowseEdit rendered content
	void PrintPDF();    // query user to specify destination file for printing
	void SetEditMode(); // toggle between Browsing or Editing
	void About();       // display the About dialog box
	void Destroy();     // close the TopWindow
	void SetBar();      // set the menubar and toolbar
	void FileBar(Bar& bar);    // set the file menu/tool bars
	void EditMenu(Bar& bar);   // set the edit menu
	void AboutMenu(Bar& bar);  // set the help/about menu
	void MainMenu(Bar& bar);   // set all the above menus
	void MainBar(Bar& bar);    // set the toolbar

public:
	typedef UWord CLASSNAME;

	static void SerializeApp(Stream& s);

	UWord(); // class default constructor (obviously!)
};


void UWord::FileBar(Bar& bar) // set the file menu/tool bars
{
	bar.Add("New", CtrlImg::new_doc(), THISBACK(New))
	   .Key(K_CTRL_N)              // provide the shortcut key combination.
	   .Help("Open new window");   // help message to display on status bar.
	bar.Add("Open..", CtrlImg::open(), THISBACK(Open))
	   .Key(K_CTRL_O)
	   .Help("Open existing document");
	bar.Add(editor.IsModified(), "Save", CtrlImg::save(), THISBACK(Save))
	   .Key(K_CTRL_S)
	   .Help("Save current document");
	bar.Add("SaveAs", CtrlImg::save_as(), THISBACK(SaveAs))
	   .Help("Save current document with a new name");
	bar.ToolGap();
	bar.MenuSeparator();
	bar.Add("Print..", CtrlImg::print(), THISBACK(Print))
	   .Key(K_CTRL_P)              // provide the shortcut key combination.
	   .Help("Print document");    // help message to display on status bar.
	bar.Add("Export to PDF..", PdfImg::pdf(), THISBACK(PrintPDF))
	   .Help("Export document to PDF file");
	if(bar.IsMenuBar()) {
		if(lrufile().GetCount())
			lrufile()(bar, THISBACK(OpenFile));
		bar.Separator();
		bar.Add("Exit", THISBACK(Destroy));
	}
}

void UWord::EditMenu(Bar& bar) // set the edit menu
{
	bar.Add("Edit Mode", // name of menu item
			// choose the menu item's icon based on the edit mode (Browse or Edit)
			editor.info.editable ? CtrlImg::MenuCheck1() : CtrlImg::MenuCheck0(),
			THISBACK(SetEditMode)) // specify the callback function
	   .Help("Toggle edit mode");  // help message to display on status bar
}

void UWord::AboutMenu(Bar& bar) // set the help/about menu
{
	bar.Add("About..", THISBACK(About)); // provide the callback function: About()
}

void UWord::MainMenu(Bar& bar)// set the main menu bar
{
	bar.Add("File", THISBACK(FileBar));       // set the file menu
	bar.Add("Edit", THISBACK(EditMenu));      // set the edit menu
	bar.Add("Window", callback(WindowsMenu)); // set the Window menu
	bar.Add("Help", THISBACK(AboutMenu));     // set the help/about menu
}

void UWord::New0() // create a new empty untitled file content
{
	UWord* uword = new UWord;              // create on a new window
	uword->editor.info.editable = true;    // this line must come before next
	uword->editor.Load("about:blank", ""); // "about:blank" creates a blank page
}

void UWord::Load(const String& name) // load file from disc into BrowseEdit
{
	lrufile().NewEntry(name);
	filename = name; // record the filename of currently opened file
	Title(name);
	String html="";  // =="" means load file's path+name as the URL
	if(IsQTF(name)){ // if true then do QTF to HTML conversion
		html = EncodeHTML(ParseQTF(LoadFile(name),0,NULL), name);
		editor.info.editable = true; // mark loaded content to be editable
	}
	editor.Load(name, html);
}

void UWord::OpenFile(const String& fn) // open file of given file name
{
	if(filename.IsEmpty() && !editor.IsModified())
		Load(fn); // load the file on the current window
	else
		(new UWord)->Load(fn); // create a new window for the file
}

void UWord::Open() // query user to select HTML file from disc
{
	FileSel& fs = UWordFs();
	if(fs.ExecuteOpen())
		OpenFile(fs); // given file's name, now go to open it
	else
		statusbar.Temporary("Loading aborted.");
}

void UWord::DragAndDrop(Point, PasteClip& d)
{
	if(IsAvailableFiles(d)) {
		Vector<String> fn = GetFiles(d);
		for(int open = 0; open < 2; open++) {
			for(int i = 0; i < fn.GetCount(); i++) {
				String ext = GetFileExt(fn[i]);
				if(FileExists(fn[i]) && (ext == ".html" || ext == ".qtf")) {
					if(open)
						OpenFile(fn[i]);
					else {
						if(d.Accept())
							break;
						return;
					}
				}
			}
			if(!d.IsAccepted())
				return;
		}
	}
}

void UWord::FrameDragAndDrop(Point p, PasteClip& d)
{
	DragAndDrop(p, d);
}

// callback for saving file content to disc
void UWord::DoSaveFile(const char* content) {
	if(SaveFile(filename, // save content to file in disc
				IsQTF(filename) ? HTML2QTF(content, filename.ToStd()) : content))
	{
		statusbar.Temporary("File " + filename + " was saved.");
		int after = editor.info.after;
		// 'after' refers to what to do after saving is successful
		editor.info.after = 0;
		if(after==1) New();     // after saving, create new empty content
		if(after==2) Destroy(); // after saving, destroy/close the window
		editor.info.changes = 0; // specify that content has been saved
		editor.UpdateIconSave(); // update the Save toolbar icon
	}
	else Exclamation("Error saving the file [* " + DeQtf(filename) + "]!");
}

static void OnSaveFile (void* arg, const char* content)
{ ((UWord*)arg)->DoSaveFile(content); }

void UWord::Save0() // get BrowseEdit HTML content to be saved
{
	lrufile().NewEntry(filename);
	if(filename.IsEmpty()) // if no file is currently opened
		SaveAs();
	else editor.GetContent(false, this, OnSaveFile);
	// specify OnSaveFile(file_content) as the callback function
}

void UWord::Save() // save BrowseEdit HTML content into current filename
{
	if(!editor.IsModified()) return;
	Save0();
}

void UWord::SaveAs() // query user to specify destination file for saving
{
	FileSel& fs = UWordFs(); // get the file selector to be used
	if(fs.ExecuteSaveAs()) {
		filename = fs; // set the new filename
		Title(filename);
		Save0(); // now go save to this file
	}
}

int UWord::CheckIfSaved(int after) // check if file is saved
{
	int r;
	if(!editor.IsModified()) r=2; // if content not modified since last saved
	else{
		r=PromptYesNoCancel("Do you want to save the changes to the document?");
		switch(r) {
		case 1: // if Yes
			editor.info.after = after; // used by DoSaveFile()
			Save();
			r=1; break;
		case 0: r=2; break; // if No
		case -1: // if Cancel
		default: r=0; break;
		}
	}
	return r;
}

void UWord::SetEditMode() {
	// make sure editor content is saved before switching to Browse mode
	if(editor.info.editable && CheckIfSaved(0)!=2) return;
	editor.SetEditMode();
}

void UWord::Print() { editor.Print(); } // print current BrowseEdit rendered content

void UWord::PrintPDF() // query user to specify destination file for printing
{
	FileSel& fs = PdfFs();
	if(!fs.ExecuteSaveAs("Output PDF file")) // provide the query-window's title
		return; // quit if no file was specified
	editor.PrintPDF(~fs, false);
}

void UWord::Destroy() // close the TopWindow
{
	if(CheckIfSaved(2)==2) // first check if file is saved
		delete this;
}

void UWord::About() // display the About dialog box
{
	PromptOK("[A5 BrowseOrEdit]&&Using [*^www://upp.sf.net^ Ultimate`+`+] technology and&Chromium Embedded Framework.");
}

void UWord::MainBar(Bar& bar) // set the toolbar
{
	FileBar(bar);             // set the file tools on toolbar
	bar.Separator();          // add a separator
	editor.DefaultBar(bar);   // set the editor tools on toolbar
}

void UWord::SetBar() // set the menubar and toolbar
{
	// when toolbar wants to refresh it will call MainBar()
	toolbar.Set(THISBACK(MainBar));
}

UWord::UWord() // class constructor
{
	// add the below widgets to the TopWindow container
	AddFrame(menubar);
	AddFrame(TopSeparatorFrame());
	AddFrame(toolbar);
	AddFrame(statusbar);
	Add(editor.SizePos());
	// specify function to call when wanting to set menubar
	menubar.Set(THISBACK(MainMenu));
	Sizeable().Zoomable();
	// specify function to call when wanting to close
	WhenClose = THISBACK(Destroy);
	// specify where status info should go to
	menubar.WhenHelp = toolbar.WhenHelp = statusbar;
	static int doc=0;
	Title(Format("Document%d", ++doc)); // set the TopWindow's title
	Icon(CtrlImg::File()); // set the TopWindow's icon
	SetBar();
	// tell editor to run SetBar() when it wants the toolbar to be updated
	editor.WhenRefreshBar = THISBACK(SetBar);
	editor.ClearModify();
	editor.topWindow = this; // tell BrowseEdit which TopWindow owns it
	OpenMain();
	ActiveFocus(editor); // give focus to the editor widget
}

void UWord::SerializeApp(Stream& s)
{
	int version = 1;
	s / version;
	s % UWordFs()
	  % PdfFs();
	if(version >= 1)
		s % lrufile();
}

static void UWord_Main()
{
	if(!BrowseEdit::CefInit()) return; // initialise CEF
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);

	UWordFs().Type("HTML files", "*.html") // add the ".html" file type to file selector
			 .Type("QTF files", "*.qtf")   // add the ".qtf" file type to file selector
	         .AllFilesType()               // allow user to select any file type
	         .DefaultExt("html");          // default file file type to ".html"

	PdfFs().Type("PDF files", "*.pdf") // add the ".pdf" file type to file selector
	       .AllFilesType()             // allow user to select any file type
	       .DefaultExt("pdf");         // default file type to ".pdf"

	LoadFromFile(callback(UWord::SerializeApp));
	UWord::New0();        // create a new top window
	Ctrl::EventLoop();    // this loops until application closes
	StoreToFile(callback(UWord::SerializeApp));
	BrowseEdit::Shutdown(); // shutdown CEF
}

}
GUI_APP_MAIN { Upp::UWord_Main(); }
