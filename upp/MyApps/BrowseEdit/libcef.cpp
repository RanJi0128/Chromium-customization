/*
	libcef.cpp

	This mainly serves as an interface between
	Chromium Embedded Framework (CEF) and the
	main application aiming at using CEF in
	Off-Screen Rendering (OSR) mode.
*/
#include "libcef.h"
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>


#ifdef WIN32
bool cef_init (void* hInstance)
{	CefMainArgs main_args((HINSTANCE)hInstance);
#else
bool cef_init (int argc, char** argv)
{	CefMainArgs main_args(argc, argv);
#endif

	// Execute the secondary process, if any.
	int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
	if(exit_code>=0) return exit_code; // child process has ended, so exit

	CefSettings settings; // enable Off-Screen Rendering (OSR)
	settings.windowless_rendering_enabled = true;

	return CefInitialize(main_args, settings, nullptr, nullptr);
}
void cef_shutdown() { CefShutdown(); }

void cef_message_loop_run() { CefRunMessageLoop(); }
void cef_message_loop_quit() { CefQuitMessageLoop(); }
void cef_message_loop_dowork() { CefDoMessageLoopWork(); }


// implement the needed CefRenderHandler callback methods
// see <include/cef_render_handler.h> for documentation
class RenderHandler : public CefRenderHandler
{
public:
	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		RecT r = cef_window_GetViewRect(owner);       // Convert from one datatype to
		rect = CefRect(r.x, r.y, r.width, r.height);  // the other: RecT to CefRect.
		return true;
	}
	bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
						int viewX, int viewY,
						int& screenX, int& screenY) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		RecT r = cef_window_GetViewRect(owner);
		screenX = viewX + r.x; // just add an (x,y) offset.
		screenY = viewY + r.y; // (CEF should even have made this a default...!)
		return true;
	}
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
				const RectList &dirtyRects, const void* buffer, int width, int height) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		cef_window_OnPaint(owner, buffer, width, height);
	}
	void OnCursorChange(CefRefPtr<CefBrowser> browser,
						CefCursorHandle cursor,
						CursorType type,
						const CefCursorInfo& custom_cursor_info) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		cef_window_OnCursorChange(owner, type);
	}
	IMPLEMENT_REFCOUNTING(RenderHandler);
};


class BrowserClient :	public CefClient,
						public CefDisplayHandler,
						public CefLoadHandler
{
public:
	// implement the needed CefDisplayHandler and CefLoadHandler callback methods
	// see <include/cef_display_handler.h> for documentation
	// see <include/cef_load_handler.h> for documentation

	void OnAddressChange(CefRefPtr<CefBrowser> browser,
						CefRefPtr<CefFrame> frame, const CefString& url) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		cef_window_OnAddressChange(owner, url.ToString().c_str());
	}
	void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		cef_window_OnTitleChange(owner, title.ToString().c_str());
	}
	bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
			const CefString& message, const CefString& source, int line) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		return cef_window_OnConsoleMessage(owner,
			message.ToString().c_str(), source.ToString().c_str(), line);
	}
	void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
			bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE
	{
		void* owner = (void*)browser->GetHost()->GetWindowHandle();
		cef_window_OnLoadingStateChange(owner, isLoading, canGoBack, canGoForward);
	}

	CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
	CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
	CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return renderHandler; }

	CefRefPtr<CefRenderHandler> renderHandler;
	CefRefPtr<CefBrowser> browser;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};


void* cef_window_new (void* owner, const char* url)
{
	CefBrowserSettings browserSettings;
	// browserSettings.windowless_frame_rate = 60; // 30 is default

	CefWindowInfo window_info;
	#ifdef WIN32
	window_info.SetAsWindowless((HWND)owner); // see <internal/cef_win.h>
	#else
	window_info.SetAsWindowless((size_t)owner);
	#endif

	BrowserClient* browserClient = new BrowserClient();
	browserClient->renderHandler = new RenderHandler();
	browserClient->browser =
		CefBrowserHost::CreateBrowserSync( // create the browser
			window_info,
			browserClient,
			url, // the browser will load this upon its creation
			browserSettings,
			nullptr);
	return browserClient; // return the browser window to main application
}

void  cef_window_delete (void* window) // free allocated resources
{
	if(window==NULL) return;
	BrowserClient* browserClient = (BrowserClient*)window;
	browserClient->renderHandler = nullptr;
	browserClient->browser = nullptr;
	//delete browserClient; // uncommenting causes code to crash
}



static inline CefRefPtr<CefBrowser> getBrowser (void* window)
{ return ((BrowserClient*)window)->browser; }

static inline CefRefPtr<CefBrowserHost> getHost (void* window)
{ return getBrowser(window)->GetHost(); }

void cef_window_close (void* window)
{ if(window) getHost(window)->CloseBrowser(false); }

void cef_window_resized (void* window)
{ if(window) getHost(window)->WasResized(); }

void cef_event_OnFocus (void* window, bool setFocus)
{ if(window) getHost(window)->SendFocusEvent(setFocus); }


enum { // from <uppsrc/CtrlCore/CtrlCore.h>
	K_DELTA        = 0x010000,

	K_ALT          = 0x080000,
	K_SHIFT        = 0x040000,
	K_CTRL         = 0x020000,

	K_KEYUP        = 0x100000,

	K_MOUSEMIDDLE  = 0x200000,
	K_MOUSERIGHT   = 0x400000,
	K_MOUSELEFT    = 0x800000,
	K_MOUSEDOUBLE  = 0x1000000,
	K_MOUSETRIPLE  = 0x2000000,

	K_SHIFT_CTRL = K_SHIFT|K_CTRL,
};

// convert from what U++ understands to what CEF understands
static uint32 GetEventModifiers(int keyflags)
{
	uint32 modifiers = 0;
	if(keyflags & K_CTRL)       modifiers |= EVENTFLAG_CONTROL_DOWN;
	if(keyflags & K_SHIFT)      modifiers |= EVENTFLAG_SHIFT_DOWN;
	if(keyflags & K_ALT)        modifiers |= EVENTFLAG_ALT_DOWN;
	if(keyflags & K_MOUSELEFT)  modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
	if(keyflags & K_MOUSERIGHT) modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
	if(keyflags & K_MOUSEMIDDLE)modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	return modifiers;
}

bool cef_event_Key (void* window, int key, int count)
{
	// convert from what U++ understands to what CEF understands
	CefKeyEvent event; event.Reset();
	event.windows_key_code = key & 0xFFFF;
	event.character = key & 0xFFFF;

	if(key&K_KEYUP) event.type = KEYEVENT_KEYUP;
	else if(key&K_DELTA) event.type = KEYEVENT_RAWKEYDOWN;
	else event.type = KEYEVENT_CHAR;
	event.modifiers = GetEventModifiers(key);

	if(window) getHost(window)->SendKeyEvent(event);
	//printf("key=0x%8x [%c] count=%d\r\n", key, (char)key, count);
	return true;
}

void cef_event_MouseClick (void* window, int x, int y, int keyflags,
							int button, bool is_up, int click_count)
{
	// convert from what U++ understands to what CEF understands
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = GetEventModifiers(keyflags);

	CefBrowserHost::MouseButtonType type =
		button==0 ? MBT_LEFT : button==1 ? MBT_RIGHT : MBT_MIDDLE;

	if(window) getHost(window)->SendMouseClickEvent(event, type, is_up, click_count);
}

void cef_event_MouseMove (void* window, int x, int y, int keyflags, bool outside)
{
	// convert from what U++ understands to what CEF understands
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = GetEventModifiers(keyflags);
	if(window) getHost(window)->SendMouseMoveEvent(event, outside);
}

void cef_event_MouseWheel (void* window, int x, int y, int zdelta, int keyflags)
{
	// convert from what U++ understands to what CEF understands
	CefMouseEvent event;
	event.x = x;
	event.y = y;
	event.modifiers = GetEventModifiers(keyflags);
	if(window) getHost(window)->SendMouseWheelEvent(event,
		(keyflags & K_SHIFT) ? zdelta : 0,
		!(keyflags & K_SHIFT) ? zdelta : 0);
}


static inline CefRefPtr<CefFrame> getFrame (void* window)
{ return getBrowser(window)->GetMainFrame(); }

bool cef_browser_CanGoBack (void* window) { return (window) ? getBrowser(window)->CanGoBack() : 0; }
bool cef_browser_CanGoForward (void* window) { return (window) ? getBrowser(window)->CanGoForward() : 0; }
void cef_browser_Paste (void* window) { if(window) getFrame(window)->Paste(); }

 // print current cef window rendered content
void cef_browser_Print (void* window) { if(window) getHost(window)->Print(); }
void cef_browser_PrintToPDF (void* window, const char* filepath, bool putHeaderFooter)
{
	if(!window) return;
	CefPdfPrintSettings settings;

	// Show the URL in the footer.
	settings.header_footer_enabled = putHeaderFooter;
	CefString(&settings.header_footer_url) =
		getFrame(window)->GetURL();

	// Print to the selected PDF file.
	getHost(window)->PrintToPDF(filepath, settings, nullptr);
}

void cef_browser_GoBack   (void* window) { if(window) getBrowser(window)->GoBack(); }
void cef_browser_GoForward(void* window) { if(window) getBrowser(window)->GoForward(); }
void cef_browser_Reload   (void* window) { if(window) getBrowser(window)->Reload(); }
void cef_browser_StopLoad (void* window) { if(window) getBrowser(window)->StopLoad(); }

// Load browser content. if 'html' is non-null and non-empty then 'url' is
// a dummy url used to resolve paths. It could be say the current directory.
// If 'html' is null or empty then url (could be a file path+name) is loaded.
void cef_browser_Load (void* window, const char* url, const char* html)
{
	if(window && url && *url) {
		if(!html || !*html) getFrame(window)->LoadURL(url);
		else getFrame(window)->LoadString(html, url);
	}
}

void cef_browser_ExecuteJavaScript (void* window, const char* command)
{ if(window) getFrame(window)->ExecuteJavaScript(command, getFrame(window)->GetURL(), 0); }

void cef_browser_Find (void* window, int identifier, const char* findText,
                       bool forward, bool matchCase, bool findNext)
{   if(window && findText && *findText)
        getHost(window)->Find(identifier, findText, forward, matchCase, findNext);
}
void cef_browser_StopFinding (void* window, bool clearSelection)
{ if(window) getHost(window)->StopFinding(clearSelection); }


// get HTML/Text content of the browser and call OnGetContent() with the result
void cef_browser_GetContent (void* window, bool getText, void* arg,
		void OnGetContent (void* arg, const char* content))
{
	if(!window) return;
    class Visitor : public CefStringVisitor {
     public:
      explicit Visitor(void* arg, void OnGetContent (void* arg, const char* content))
      : arg(arg), OnGetContent(OnGetContent) {}

      virtual void Visit(const CefString& string) OVERRIDE {
        std::string content = string.ToString(); // first store string locally
        OnGetContent(arg, content.c_str()); // then call the callback function
      }
     private:
      void* arg;
      void (*OnGetContent) (void* arg, const char* content);
      IMPLEMENT_REFCOUNTING(Visitor);
    };
    CefRefPtr<CefFrame> frame = getFrame(window);
    Visitor *v = new Visitor(arg, OnGetContent);
    if(getText) // a different function is called depending on what is wanted
         frame->GetText(v);
    else frame->GetSource(v);
}
