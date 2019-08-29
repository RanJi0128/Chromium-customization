/*
	BrowseEdit.h

	-- Creating and using custom widgets:
	https://www.ultimatepp.org/srcdoc$CtrlLib$Tutorial$en-us.html#21
	-- Reacting to input events:
	https://www.ultimatepp.org/srcdoc$CtrlLib$Tutorial$en-us.html#5
*/
#ifndef BROWSE_EDIT_H
#define BROWSE_EDIT_H

#include <CtrlLib/CtrlLib.h>

namespace Upp {

// structure that contains the information used in order
// to set the proper states of all the toolbar components
struct ToolBarInfo {
  bool editable;
  char showhtml;

  bool undo;
  bool redo;
  bool cut;
  bool copy;
  bool paste;
  bool dele;

  bool bold;
  bool italic;
  bool underline;
  bool strikeout;
  bool subscript;
  bool superscript;

  short align;
  bool listBulleted;
  bool listNumbered;
  bool spellchecker;

  int colourText;
  int colourBack;
  int bockStyle;
  int fontFamily;
  int fontSize;

  int changes; // undo will -- , redo will ++

  // specifies what to do 'after' content is saved
  // TODO: but this inside BrowseOrEdit.cpp instead
  int after;
};

class BrowseEdit;

class CustomEdit : public EditField {
public:
	BrowseEdit* owner;
	CustomEdit() { owner=nullptr; }
	virtual bool Key (dword key, int count);
	// override the Key() method so to detect the
	// 'Enter' key so to then fetch the entry text
};

class BrowseEdit : public Ctrl {
public:
	void* window;      // the cef window (aka browser)
	void* buffer;      // record the pixel colour buffer of the cef_window
	int width, height; // record the width and height of the pixel buffer

	virtual void Layout();        // notify cef window of layout changes
	virtual void Paint (Draw& w); // callback method to paint 'buffer' to widget

	// below are widget Key and OnFocus events called from the Ctrl class
	virtual bool Key (dword key, int count);
	virtual void GotFocus() { OnFocus(true); }
	virtual void LostFocus() { OnFocus(false); }
	virtual void OnFocus (bool setFocus);

	// below are widget mouse-click events called from the Ctrl class
	virtual void MouseClick (Point p, dword keyflags, int button, bool is_up, int click_count);
	virtual void LeftUp      (Point p, dword keyflags) { MouseClick(p, keyflags, 0,1,1); }
	virtual void RightUp     (Point p, dword keyflags) { MouseClick(p, keyflags, 1,1,1); }
	virtual void MiddleUp    (Point p, dword keyflags) { MouseClick(p, keyflags, 2,1,1); }
	virtual void LeftDown    (Point p, dword keyflags) { MouseClick(p, keyflags, 0,0,1); }
	virtual void RightDown   (Point p, dword keyflags) { MouseClick(p, keyflags, 1,0,1); }
	virtual void MiddleDown  (Point p, dword keyflags) { MouseClick(p, keyflags, 2,0,1); }
	virtual void LeftDouble  (Point p, dword keyflags) { MouseClick(p, keyflags, 0,0,2); }
	virtual void RightDouble (Point p, dword keyflags) { MouseClick(p, keyflags, 1,0,2); }
	virtual void MiddleDouble(Point p, dword keyflags) { MouseClick(p, keyflags, 2,0,2); }
	virtual void LeftTriple  (Point p, dword keyflags) { MouseClick(p, keyflags, 0,0,3); }
	virtual void RightTriple (Point p, dword keyflags) { MouseClick(p, keyflags, 1,0,3); }
	virtual void MiddleTriple(Point p, dword keyflags) { MouseClick(p, keyflags, 2,0,3); }

	// below are widget mouse-motion events called from the Ctrl class
	virtual void MouseLeave  ();
	virtual void MouseMove   (Point p, dword keyflags);
	virtual void MouseWheel  (Point p, int zdelta, dword keyflags);

public:
	bool IsModified() { return info.changes?1:0; }

	// Load browser content. if 'html' is non-null and non-empty then 'url' is
	// a dummy url used to resolve paths. It could be say the current directory.
	// If 'html' is null or empty then url (could be a file path+name) is loaded.
	virtual void Load(const String& url, const String& html);

	// get HTML/Text content of the browser and call OnGetContent() with the result
	virtual void GetContent(bool getText, void* arg,
		void OnGetContent (void* arg, const char* content));

	virtual void Print(); // print current cef window rendered content
	virtual void PrintPDF(const String& filename, bool putHeaderFooter);

public:
	Event<> WhenRefreshBar; // has callback function for setting-up toolbar
	ToolBarInfo info;       // determines the state of toolbar components
	void UpdateToolBar();   // updates the toolbar components states
	void UpdateIconSave() { UpdateToolBar(); }

	// parse queryInfo string obtained from OnConsoleMessage()
	// and extract the necessary toolbar state information
	void OnToolbarInfo (const char* msg);

    // the methods below setup the given toolbar widget
	bool useraction;
	void UserAction();
	Event<> User(Event<>  cb);
	void DefaultBar(Bar& bar);

	void   UndoTool(Bar& bar, dword key = K_CTRL_Z);
	void   RedoTool(Bar& bar, dword key = K_SHIFT_CTRL_Z);
	void   CutTool(Bar& bar, dword key = K_CTRL_X);
	void   CopyTool(Bar& bar, dword key = K_CTRL_C);
	void   PasteTool(Bar& bar, dword key = K_CTRL_V);
	void   FindReplaceTool(Bar& bar, dword key = K_CTRL_F);
	void   SpellCheckTool(Bar& bar);
	void   LoadImageTool(Bar& bar, dword key = 0);

	void   BlockStyleTool(Bar& bar, int width = Zx(90));
	void   FontFaceTool(Bar& bar, int width = Zx(120));
	void   FontSizeTool(Bar& bar, int width = Zx(90));
	void   BoldTool(Bar& bar, dword key = K_CTRL_B);
	void   ItalicTool(Bar& bar, dword key = K_CTRL_I);
	void   UnderlineTool(Bar& bar, dword key = K_CTRL_U);
	void   StrikeoutTool(Bar& bar, dword key = 0);
	void   SuperscriptTool(Bar& bar, dword key = 0);
	void   SubscriptTool(Bar& bar, dword key = 0);

	void   LeftTool(Bar& bar, dword key = K_CTRL_L);
	void   RightTool(Bar& bar, dword key = K_CTRL_R);
	void   CenterTool(Bar& bar, dword key = K_CTRL_E);
	void   JustifyTool(Bar& bar, dword key = K_CTRL_J);
	void   BulletedTool(Bar& bar, dword key = 0);
	void   NumberedTool(Bar& bar, dword key = 0);
	void   IncIndentTool(Bar& bar, dword key = 0);
	void   DecIndentTool(Bar& bar, dword key = 0);
    // the methods ABOVE setup the given toolbar widget

public:
	FileSel imagefs;           // file selector used by InsertImage()
	ColorButton ink, paper;    // used for text-colour and background-colour
	DropList blockstyle, fontface, fontsize; // toolbar components
	CustomEdit url, hyperlink; // text fields placed in toolbar
	Button back, forward, reload, stop;

	// the below are used by the 'Find' dialog box
	void* findBox;
	bool findNext;
	void FindDo();
	void FindStop();
	void OpenFindBox(); // open the 'find' dialog box
	virtual void SetupFindBox(bool create); // create 'find' diablog box or else destroy it

	// callbacks on toolbar button press or component selection
	void Undo();
	void Redo();
	void Cut();
	void Copy();
	void Paste();
	void SpellCheck();
	void InsertImage();
	void Hyperlink();   // callback to process user-entered Hyperlink

	// callbacks on toolbar button press or component selection
	void BlockStyle();
	void FontFace();
	void FontSize();
	void Bold();
	void Italic();
	void Underline();
	void Strikeout();
	void SetScript(int script);
	void SetInk();
	void SetPaper();

	// callbacks on toolbar button press or component selection
	void AlignLeft();
	void AlignRight();
	void AlignCenter();
	void AlignJustify();
	void IncIndent();
	void DecIndent();
	void Bulleted();
	void Numbered();

	void SetEditMode(); // toggle between Browsing or Editing
	void OnEnterUrl();  // callback to process user-entered URL

	// callbacks on toolbar button press
	void OnBack();
	void OnForward();
	void OnReload();
	void OnStop();

	// execute a JavaScript command into the browser
	virtual void ExecuteJavaScript (const char* command);

	// using 'cmd' and 'arg', call document.execCommand()
	virtual void SendCommand (const char* cmd, const char* arg);

public:
	static bool CefInit();  // initialise CEF
	static void Shutdown(); // shutdown CEF
	void Close();           // close the cef window (this->window)
	TopWindow* topWindow;   // the TopWindow that owns this BrowseEdit

	typedef BrowseEdit CLASSNAME;

	BrowseEdit();          // class constructor (initialises class variables)
	virtual ~BrowseEdit(); // class destructor (free allocated resources)
};

}

#endif
