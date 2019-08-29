#include "BrowseEdit.h"
#include "libcef.h"

namespace Upp {

// execute a JavaScript command into the browser
void BrowseEdit::ExecuteJavaScript (const char* command)
{ cef_browser_ExecuteJavaScript(window, command); }

// using 'cmd' and 'arg', call document.execCommand()
void BrowseEdit::SendCommand (const char* cmd, const char* arg)
{
	char command[1000];
	sprintf(command, "document.execCommand('%s', false, '%s');", cmd, arg);
	ExecuteJavaScript(command);
}

void BrowseEdit::InsertImage()
{
	// query user to select image file from disc
	if(!imagefs.ExecuteOpen("Open image file")) return;
	const String& name = imagefs; // get the filename

	// prepare the JavaScript command to be executed, then do execute
	std::string command = "document.execCommand('insertImage', false, '"+name.ToStd()+"');";
	std::replace(command.begin(), command.end(), '\\', '/');
	ExecuteJavaScript(command.c_str());
}

void BrowseEdit::SetEditMode() // toggle between Browsing or Editing
{
	info.editable = !info.editable;
	if(info.editable)
		info.spellchecker=true;
	else {
		char showhtml = info.showhtml;
		memset(&info, 0, sizeof(info));
		info.showhtml = showhtml;
	}
	// prepare the JavaScript command to be executed, then do execute
	std::string command = "document.body.contentEditable = ";
	command += info.editable ? "true" : "false";
	ExecuteJavaScript(command.c_str());
	UpdateToolBar(); // update toolbar components states
}

void BrowseEdit::SpellCheck()
{
	if(!info.editable) return;
	info.spellchecker = !info.spellchecker;
	// prepare the JavaScript command to be executed, then do execute
	std::string command = "document.body.spellcheck = ";
	command += info.spellchecker ? "true" : "false";
	ExecuteJavaScript(command.c_str());
	UpdateToolBar(); // update toolbar components states
}

void BrowseEdit::BlockStyle()
{
	const char* arg=NULL;
	// prepare the argument to send to document.execCommand()
	std::string s = ((String)~blockstyle).ToStd();
	if(s=="paragraph") arg = "p";
	if(s=="preformatted") arg = "pre";
	if(s=="heading 1") arg = "h1";
	if(s=="heading 2") arg = "h2";
	if(s=="heading 3") arg = "h3";
	if(s=="heading 4") arg = "h4";
	if(s=="heading 5") arg = "h5";
	if(s=="heading 6") arg = "h6";
	if(arg) SendCommand("formatBlock", arg);
}

void BrowseEdit::FontFace()
{
	// prepare the argument to send to document.execCommand()
	std::string arg = ((String)~fontface).ToStd();
	if(arg.length()) SendCommand("fontName", arg.c_str());
}

void BrowseEdit::FontSize()
{
	const char* arg=NULL;
	// prepare the argument to send to document.execCommand()
	std::string s = ((String)~fontsize).ToStd();
	if(s=="x-small")    arg = "1";
	if(s=="small size") arg = "2";
	if(s=="normal size")arg = "3";
	if(s=="medium size")arg = "4";
	if(s=="large size") arg = "5";
	if(s=="x-large")    arg = "6";
	if(s=="xx-large")   arg = "7";
	if(arg) SendCommand("fontSize", arg);
}

void BrowseEdit::Undo() { SendCommand("undo" , NULL); }
void BrowseEdit::Redo() { SendCommand("redo" , NULL); }
void BrowseEdit::Cut()  { SendCommand("cut"  , NULL); }
void BrowseEdit::Copy() { SendCommand("copy" , NULL); }
void BrowseEdit::Paste(){ cef_browser_Paste(window); }

void BrowseEdit::Bold() { SendCommand("bold", NULL); }
void BrowseEdit::Italic(){ SendCommand("italic", NULL); }
void BrowseEdit::Underline(){ SendCommand("underline", NULL); }
void BrowseEdit::Strikeout(){ SendCommand("strikeThrough", NULL); }
void BrowseEdit::SetScript(int script){
	if(script==1) SendCommand("superscript", NULL);
	if(script==2) SendCommand("subscript", NULL);
}

// get the colour as a string in the form "#RRGGBB"
static const char* getColor(const Color& color)
{
	static char str[100];
	RGBA c = RGBA(color);
	if(c.a!=0xFF) return NULL;
	// NOTE: the '0' padding below is important
	sprintf(str, "#%02x%02x%02x", c.r, c.g, c.b);
	return str;
}
void BrowseEdit::SetInk(){ SendCommand("foreColor", getColor(~ink)); }
void BrowseEdit::SetPaper(){ SendCommand("hiliteColor", getColor(~paper)); }

void BrowseEdit::AlignLeft() { SendCommand("justifyLeft", NULL); }
void BrowseEdit::AlignRight() { SendCommand("justifyRight", NULL);}
void BrowseEdit::AlignCenter() { SendCommand("justifyCenter", NULL); }
void BrowseEdit::AlignJustify() { SendCommand("justifyFull", NULL); }

void BrowseEdit::IncIndent(){ SendCommand("indent", NULL); }
void BrowseEdit::DecIndent(){ SendCommand("outdent", NULL); }
void BrowseEdit::Bulleted() { SendCommand("insertUnorderedList", NULL); }
void BrowseEdit::Numbered() { SendCommand("insertOrderedList", NULL);}


void BrowseEdit::Hyperlink() // callback to process user-entered Hyperlink
{
	// prepare the JavaScript command to be executed, then do execute
	std::string s = ((String)~hyperlink).ToStd();
	std::string command = "document.execCommand";
	if(s.length()==0) command += "('unlink', false);";
	else command += "('createLink', false, '" + s + "');";
	ExecuteJavaScript(command.c_str());
}

void BrowseEdit::OnEnterUrl(){ // callback to process user-entered URL
	std::string s = ((String)~url).ToStd();
	cef_browser_Load(window, s.c_str(), NULL); // load the URL
}

// override the Key() method so to detect the
// 'Enter' key so to then fetch the entry text
bool CustomEdit::Key (dword key, int count)
{
	if((key=='\n' || key=='\r') && owner) {             // detect 'Enter' key
		if(this==&owner->url) owner->OnEnterUrl();      // process user-entered URL
		if(this==&owner->hyperlink) owner->Hyperlink(); // process user-entered Hyperlink
	}
	return EditField::Key(key,count);
}

void BrowseEdit::OnBack() { cef_browser_GoBack(window); }
void BrowseEdit::OnForward() { cef_browser_GoForward(window); }
void BrowseEdit::OnReload() { cef_browser_Reload(window); }
void BrowseEdit::OnStop() { cef_browser_StopLoad(window); }


// Load browser content. if 'html' is non-null and non-empty then 'url' is
// a dummy url used to resolve paths. It could be say the current directory.
// If 'html' is null or empty then url (could be a file path+name) is loaded.
void BrowseEdit::Load(const String& url, const String& html)
{
	info.showhtml = false;            // do not display html
	info.changes = 0;                 // set as non-modified
	std::string _url = url.ToStd();   // first store url locally
	std::string _html = html.ToStd(); // also store html locally
	cef_browser_Load(window, _url.c_str(), _html.c_str()); // load
}

// get HTML/Text content of the browser and call OnGetContent() with the result
void BrowseEdit::GetContent(bool getText, void* arg,
		void OnGetContent (void* arg, const char* content)) {
	cef_browser_GetContent(window, getText, arg, OnGetContent);
}

// print current cef window rendered content
void BrowseEdit::Print() { cef_browser_Print(window); }
void BrowseEdit::PrintPDF(const String& filename, bool putHeaderFooter)
{
	std::string name = filename.ToStd(); // first store string as an l-value
	cef_browser_PrintToPDF(window, name.c_str(), putHeaderFooter);
}


#define FINDBOX WithRichFindReplaceLayout<TopWindow>
#define LAYOUTFILE <BrowseEdit/BrowseEdit.lay>
#include <CtrlCore/lay.h>

void BrowseEdit::SetupFindBox(bool create) // create 'find' diablog box or else destroy it
{
	if(create && findBox==NULL) {
		findBox = new FINDBOX;
		FINDBOX& findbox = *(FINDBOX*)findBox;
		CtrlLayoutOKCancel(findbox, t_("Find")); // specify the title of dialog box
		findbox.cancel <<= THISBACK(FindStop);   // specify the callback method for 'Cancel'
		findbox.ok <<= THISBACK(FindDo);         // specify the callback method for 'Find' button
	}
	else if(!create && findBox!=NULL) {
		delete ((FINDBOX*)findBox);
		findBox = NULL;
	}
}
void BrowseEdit::OpenFindBox() // open the 'find' dialog box
{
	if(findBox==NULL) SetupFindBox(true);
	FINDBOX& findbox = *(FINDBOX*)findBox;
	if(findbox.IsOpen()) return;     // if dialog is already opened then quit
	findNext=false;                  // specify that next search is a starting
	findbox.Open();                  // now open the find-dialog-box
	findbox.find.SetFocus();         // set focus to the 'find' edit text field
}
void BrowseEdit::FindDo()
{
	if(findBox==NULL) return;
	FINDBOX& findbox = *(FINDBOX*)findBox;
	String txt = (~findbox.find);    // get the text to be searched
	const std::string& text = txt.ToStd();
	if(text.empty()) return;         // if text is empty then quit
	findbox.find.AddHistory();       // else add it to find-history
	cef_browser_Find(window, 0,      // now perform the cef call
					text.c_str(),    // providing the text to be searched
					!findbox.upward, // as well as extra info
					!findbox.ignorecase,
					findNext);
	findNext=true; // specify that next search is 'not' a starting
}
void BrowseEdit::FindStop() // upon 'cancel' of the find-dialog-box
{
	if(findBox==NULL) return;
	cef_browser_StopFinding(window, false); // stop finding
	((FINDBOX*)findBox)->Close(); // then close the dialog box
}


void BrowseEdit::UpdateToolBar(){ // updates the toolbar components states
	if(info.editable) info.paste=info.dele=true;
	WhenRefreshBar(); // the callback function was set by the top window
}

// parse queryInfo string obtained from OnConsoleMessage()
// and extract the necessary toolbar state information
void BrowseEdit::OnToolbarInfo (const char* msg)
{
  msg += 7; // skip the "query: " text

  info.copy       = *msg++=='1';
  info.editable   = *msg++=='1';
  if(!info.editable)
  {
    blockstyle.Set("block style");
    fontface.Set("font face");
    fontsize.Set("font size");
    UpdateToolBar(); // toolbar state may need to change
  }
  else {
    info.spellchecker=*msg++=='1';
    info.cut        = *msg++=='1';
    info.undo       = *msg++=='1';
    info.redo       = *msg++=='1';
    msg++;
    info.bold       = *msg++=='1';
    info.italic     = *msg++=='1';
    info.underline  = *msg++=='1';
    info.strikeout  = *msg++=='1';
    info.superscript= *msg++=='1';
    info.subscript  = *msg++=='1';
    msg++;
    if(*msg++=='1') info.align = ALIGN_LEFT;
    if(*msg++=='1') info.align = ALIGN_CENTER;
    if(*msg++=='1') info.align = ALIGN_RIGHT;
    if(*msg++=='1') info.align = ALIGN_JUSTIFY;
    info.listBulleted = *msg++=='1';
    info.listNumbered = *msg++=='1';

    UpdateToolBar(); // toolbar state may need to change

    msg++; // prepare for next
    char str[1000];
    int i;

	//**************************** Extract Block Style
    ASSERT(*msg++=='(');
    for(i=0; *msg!=')'; ) str[i++] = *msg++;
    str[i]=0;
    i=-1;
    if(str[0]==0) blockstyle.Set("default"); // if default
    else if(str[0]=='h' || str[0]=='H') { // if heading
      if('1'<=str[1] && str[1]<='6')
        i = str[1]-'1'+2;
    }
    else if(0==strcmp(str, "p"  )) i=0; // if paragraph
    else if(0==strcmp(str, "pre")) i=1; // if preformatted
    else blockstyle.Set(str);
    if(i>=0) {
      blockstyle.SetIndex(i);      // update toolbar component
      blockstyle.Set(~blockstyle); // set value so to reflect change
    }
    msg++; // prepare for next

	//**************************** Extract Font Face
    ASSERT(*msg++=='(');
    for(i=0; *msg!=')'; msg++) {
      if(i+1==sizeof(str)) break;
      if(*msg!='"') str[i++] = *msg;
    } str[i]=0;
    if(i>0) {
      i=-1;
           if(0==strcmp(str, "sans-serif"     )) i=0;
      else if(0==strcmp(str, "serif"          )) i=1;
      else if(0==strcmp(str, "monospace"      )) i=2;
      else if(0==strcmp(str, "Arial"          )) i=3;
      else if(0==strcmp(str, "Times New Roman")) i=4;
      else if(0==strcmp(str, "Courier New"    )) i=5;
      if(i>=0) fontface.SetIndex(i);
      fontface.Set(str); // update toolbar component
    }
    msg++; // prepare for next

	//**************************** Extract Font Size
    ASSERT(*msg++=='(');
    for(i=0; *msg!=')'; ) str[i++] = *msg++;
    str[i]=0;
    if(i>0 && '1'<=str[0] && str[0]<='7') {
      fontsize.SetIndex(str[0]-'1'); // update toolbar component
      fontsize.Set(~fontsize);       // set value so to reflect change
    }

    msg+=2; // prepare for next
    RGBA c;

	//**************************** Extract Text Colour
    ASSERT(*msg++=='(');
    c.a = c.r = c.g = c.b = 0;
    if(0==memcmp(msg, "rgb(", 4)){ // if "rgb(r,g,b)"
      msg += 4;
      for(i=0; *msg!=','; ) str[i++] = *msg++;
      str[i]=0;
      c.r = atoi(str);
      msg++; // skip ','
      for(i=0; *msg!=','; ) str[i++] = *msg++;
      str[i]=0;
      c.g = atoi(str);
      msg++; // skip ','
      for(i=0; *msg!=')'; ) str[i++] = *msg++;
      str[i]=0;
      c.b = atoi(str);
      msg++; // skip ')'
      c.a = 0xFF;
    }
    ink.SetData(Color(c)); // update toolbar component

    msg++; // prepare for next

	//**************************** Extract Background Colour
    ASSERT(*msg++=='(');
    c.a = c.r = c.g = c.b = 0;
    if(0==memcmp(msg, "rgb(", 4)){ // if "rgb(r,g,b)"
      msg += 4;
      for(i=0; *msg!=','; ) str[i++] = *msg++;
      str[i]=0;
      c.r = atoi(str);
      msg++; // skip ','
      for(i=0; *msg!=','; ) str[i++] = *msg++;
      str[i]=0;
      c.g = atoi(str);
      msg++; // skip ','
      for(i=0; *msg!=')'; ) str[i++] = *msg++;
      str[i]=0;
      c.b = atoi(str);
      msg++; // skip ')'
      c.a = 0xFF;
    }
    paper.SetData(Color(c)); // update toolbar component
  }
}

}

// cef callback function for when URL changes
void cef_window_OnAddressChange (void* owner, const char* url)
{ if(owner && url) ((Upp::BrowseEdit*)owner)->url.SetData(url); }

// cef callback function for when title changes
void cef_window_OnTitleChange (void* owner, const char* title)
{
	if(owner && title && *title
	&& 0!=strcmp(title, "about:blank")
	&& ((Upp::BrowseEdit*)owner)->topWindow)
		((Upp::BrowseEdit*)owner)->topWindow->Title(title);
}

// cef callback function to process console message
bool cef_window_OnConsoleMessage (void* owner,
		const char* message, const char* source, int line)
{
	if(!owner) return false;
	Upp::BrowseEdit* be = ((Upp::BrowseEdit*)owner);

	if(0==memcmp(message, "query: ", 7)) // if message is the queryInfo, then
		be->OnToolbarInfo(message);      // parse it to extract toolbar state info

	else if(0==strcmp(message, "oninput")) {            // if message is due to document editing
		if(be->info.editable && be->info.changes++ ==0) // then mark that BrowseEdit is modified
			be->UpdateIconSave(); // if was not previous modified then update the Save toolbar icon
	}

	// if user tries to InsertImage() without haven saved the document to file, then report error.
	else if(0==memcmp(message, "Not allowed to load local resource:", 35))
		Upp::PromptOK("Error, first save the current file locally.");

	else printf("console[%s]\r\n", message);
	return true;
}


// this is a set of JavaScript commands used so to get info about the cef browser document,
// especially info pertaining to the state of the toolbar components (e.g: bold, italic, colour)
static const char* queryInfo = "function queryInfo() { console.log('query: '+(document.queryCommandEnabled('copy')?'1':'0')+(!document.body.isContentEditable?'0':'1'+(document.body.spellcheck?'1':'0')+(document.queryCommandEnabled('cut')?'1':'0')+(document.queryCommandEnabled('undo')?'1':'0')+(document.queryCommandEnabled('redo')?'1':'0')+' '+(document.queryCommandState('bold')?'1':'0')+(document.queryCommandState('italic')?'1':'0')+(document.queryCommandState('underline')?'1':'0')+(document.queryCommandState('strikethrough')?'1':'0')+(document.queryCommandState('superscript')?'1':'0')+(document.queryCommandState('subscript')?'1':'0')+' '+(document.queryCommandState('justifyLeft')?'1':'0')+(document.queryCommandState('justifyCenter')?'1':'0')+(document.queryCommandState('justifyRight')?'1':'0')+(document.queryCommandState('justifyFull')?'1':'0')+(document.queryCommandState('insertUnorderedList')?'1':'0')+(document.queryCommandState('insertOrderedList')?'1':'0')+' ('+document.queryCommandValue('formatBlock')+')('+document.queryCommandValue('fontName')+')('+document.queryCommandValue('fontSize')+') ('+document.queryCommandValue('foreColor')+')('+document.queryCommandValue('backColor')+')')); } document.body.addEventListener('click', queryInfo, false); document.body.addEventListener('contextmenu', queryInfo, false); document.body.addEventListener('input', function(){console.log('oninput'); queryInfo();});";

// cef callback method to process changes in the document loading state
void cef_window_OnLoadingStateChange (void* owner,
		bool isLoading, bool canGoBack, bool canGoForward)
{
	if(!owner) return;
	Upp::BrowseEdit& be = *((Upp::BrowseEdit*)owner);

	// update the state of the toolbar's navigation-and-loading buttons
	be.back.Enable   (!be.info.editable && canGoBack);
	be.forward.Enable(!be.info.editable && canGoForward);
	be.reload.Enable (!be.info.editable && !isLoading);
	be.stop.Enable   (!be.info.editable && isLoading);

  if(!isLoading) // if finished loading the document
  {
    be.ExecuteJavaScript(queryInfo); // load-in the queryInfo() JavaScript function
    be.ExecuteJavaScript("setTimeout(queryInfo,50)"); // then schedule to call it

    if(!be.info.editable) {
    }
    else {
      // after loading, if the document is supposed to be editable then
      // send the appropriate JavaScript command that will effect this.
      be.ExecuteJavaScript("document.body.contentEditable=true");

      // However, focus is not automatically placed on the document's body,
      // even though the browser window does has focus. In order to fix this,
      // simulate a single click of the Left Mouse button.
      Upp::Point p(10,10);
      be.LeftDown(p,0);
      be.LeftUp(p,0);
    }
  }
}
