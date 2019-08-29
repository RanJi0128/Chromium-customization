/*
	libcef.h

	This mainly serves as an interface between
	Chromium Embedded Framework (CEF) and the
	main application aiming at using CEF in
	Off-Screen Rendering (OSR) mode.
*/
#ifndef CEF_CLIENT_H
#define CEF_CLIENT_H

#ifdef WIN32
bool cef_init (void* hInstance);
#else
bool cef_init (int argc, char** argv);
#endif
void cef_shutdown();

void cef_message_loop_run();
void cef_message_loop_quit();
void cef_message_loop_dowork();

void* cef_window_new (void* owner, const char* url);
void  cef_window_close (void* window);
void  cef_window_delete (void* window);
void  cef_window_resized (void* window);

// see <include/cef_render_handler.h> for documentation
struct RecT { int x, y, width, height; };
RecT cef_window_GetViewRect (void* owner); // note: output in 'screen' coordinates
void cef_window_OnPaint (void* owner, const void* buffer, int width, int height);
void cef_window_OnCursorChange (void* owner, int cursor_type);

// see <include/cef_display_handler.h> for documentation
void cef_window_OnAddressChange (void* owner, const char* url);
void cef_window_OnTitleChange (void* owner, const char* title);
bool cef_window_OnConsoleMessage (void* owner,
        const char* message, const char* source, int line);

// see <include/cef_load_handler.h> for documentation
void cef_window_OnLoadingStateChange (void* owner,
        bool isLoading, bool canGoBack, bool canGoForward);

// see <include/cef_browser.h> for documentation
void cef_event_OnFocus    (void* window, bool setFocus);
bool cef_event_Key        (void* window, int key, int count);
void cef_event_MouseMove  (void* window, int x, int y, int keyflags, bool outside);
void cef_event_MouseWheel (void* window, int x, int y, int zdelta, int keyflags);
void cef_event_MouseClick (void* window, int x, int y, int keyflags,
                            int button, bool is_up, int click_count);

//***********************************************
// see <include/cef_browser.h> for documentation

bool cef_browser_CanGoBack (void* window);
bool cef_browser_CanGoForward (void* window);
void cef_browser_Paste (void* window);
void cef_browser_Print (void* window); // print current cef window rendered content
void cef_browser_PrintToPDF (void* window, const char* filepath, bool putHeaderFooter);

void cef_browser_GoBack   (void* window);
void cef_browser_GoForward(void* window);
void cef_browser_Reload   (void* window);
void cef_browser_StopLoad (void* window);
void cef_browser_ExecuteJavaScript (void* window, const char* command);

// Load browser content. if 'html' is non-null and non-empty then 'url' is
// a dummy url used to resolve paths. It could be say the current directory.
// If 'html' is null or empty then url (could be a file path+name) is loaded.
void cef_browser_Load (void* window, const char* url, const char* html);

void cef_browser_Find (void* window, int identifier, const char* findText,
                       bool forward, bool matchCase, bool findNext);
void cef_browser_StopFinding (void* window, bool clearSelection);

// get HTML/Text content of the browser and call OnGetContent() with the result
void cef_browser_GetContent (void* window, bool getText, void* arg,
        void OnGetContent (void* arg, const char* content));

#endif
