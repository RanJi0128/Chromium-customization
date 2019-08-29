#include "BrowseEdit.h"
#include "libcef.h"
#include <stdio.h>
#include <malloc.h>

using namespace Upp;


// This cef callback function requests the position and size of the
// target window, in coordinates relative to computer screen.
RecT cef_window_GetViewRect (void* owner)
{
	Rect r = ((BrowseEdit*)owner)->GetScreenView();
	RecT rect = {r.left, r.top, r.Width(), r.Height()};
	return rect;
}

// This cef callback function provides the cef-window's image invalidate region.
// Store this information so that BrowseEdit::Paint() will later use it.
void cef_window_OnPaint (void* owner, const void* buffer, int width, int height)
{
	if(width==0 || height==0) return;
	BrowseEdit& be = *((BrowseEdit*)owner);
	int size = height*width*4;
	if(size != be.height*be.width*4)
		be.buffer = realloc(be.buffer, size);
	memcpy(be.buffer, buffer, size);
	be.height = height;
	be.width = width;
	be.Refresh(); // now schedule a call to BrowseEdit::Paint()
}

void BrowseEdit::Paint(Draw& w) // callback method to paint 'this->buffer' to widget
{
	if(width==0 || height==0) return;
	Size size = GetSize();
	w.DrawRect(size, White()); // ensure a White() background

	// copy the pixel colour data found in this->buffer
	// into a new ImageBuffer, then use w.DrawImage()
	ImageBuffer image(size);
	const int* in = (const int*)buffer;
	RGBA color;
	int x, y;

	for(y=0; y<height && y<size.cy; y++) {
		RGBA* out = image[y];
		for(x=0; x<width; x++, in++) {
			if(x>=size.cx) continue;
			register int c = *in;
			color.b = (byte)(c&0xFF); c>>=8; // extract blue
			color.g = (byte)(c&0xFF); c>>=8; // extract green
			color.r = (byte)(c&0xFF); c>>=8; // extract red
			color.a = (byte)(c); // finally extract alpha
			out[x] = color; // set the color to ImageBuffer
		}
		// do color-padding if target size.cx is too large
		for( ; x<size.cx; x++) out[x] = color;
	}
	for( ; y<size.cy; y++) { // do color-padding if target size.cy is larger
		RGBA* out = image[y];
		for(x=0; x<size.cx; x++) out[x] = color;
	}
	w.DrawImage(GetSize(), image);
}

void BrowseEdit::Layout() // notify cef window of layout changes
{ if(GetSize().cx && GetSize().cy) cef_window_resized(window); }


// the below is a function pointer declared
// inside: upp\uppsrc\CtrlCore\Win32Wnd.cpp
extern void (*cefMessageLoopDowork)();

bool BrowseEdit::CefInit() // initialise CEF
{
	// the file upp\uppsrc\CtrlCore\Win32Wnd.cpp is changed so that
	// it will call the function below inside the Ctrl::EventLoop()
	cefMessageLoopDowork = cef_message_loop_dowork;
	return cef_init(NULL); // this must come last
}
void BrowseEdit::Shutdown() { cef_shutdown(); } // shutdown CEF

// notify cef window of the OnFocus event
void BrowseEdit::OnFocus (bool setFocus) { cef_event_OnFocus(window, setFocus); }

// notify cef window of Key events
bool BrowseEdit::Key (dword key, int count) {
	cef_event_Key (window, key, count);
	switch(key&0xFFFF){
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_LEFT:
	case VK_PRIOR:  // page up key
	case VK_NEXT:   // page down key
	case VK_HOME:
	case VK_END:
	case VK_TAB: ExecuteJavaScript("setTimeout(queryInfo, 20)");
	// Upon the press of each of these keys above, the toolbar
	// components may have to change 'after' the key has been
	// processed. So schedule for a queryInfo, the later which
	// will call OnConsoleMessage to pass-over the toolbar info.
	}
	return true;
}

// notify cef window of mouse-click events
void BrowseEdit::MouseClick (Point p, dword keyflags, int button, bool is_up, int click_count)
{
	if(!is_up) { SetFocus(); OnFocus(true); } // ensure a key-down implies a gain of focus
	cef_event_MouseClick (window, p.x, p.y, keyflags, button, is_up, click_count);
}

// notify cef window of mouse-click events
void BrowseEdit::MouseLeave () { Point p=GetMouseViewPos(); cef_event_MouseMove (window, p.x, p.y, 0, true); }
void BrowseEdit::MouseMove  (Point p, dword keyflags) { cef_event_MouseMove (window, p.x, p.y, keyflags, false); }
void BrowseEdit::MouseWheel (Point p, int zdelta, dword keyflags) { cef_event_MouseWheel(window, p.x, p.y, zdelta, keyflags); }


typedef enum { // from <include/internal/cef_types.h>
  CT_POINTER = 0,
  CT_CROSS,
  CT_HAND,
  CT_IBEAM,
  CT_WAIT,
  CT_HELP,
  CT_EASTRESIZE,
  CT_NORTHRESIZE,
  CT_NORTHEASTRESIZE,
  CT_NORTHWESTRESIZE,
  CT_SOUTHRESIZE,
  CT_SOUTHEASTRESIZE,
  CT_SOUTHWESTRESIZE,
  CT_WESTRESIZE,
  CT_NORTHSOUTHRESIZE,
  CT_EASTWESTRESIZE,
  CT_NORTHEASTSOUTHWESTRESIZE,
  CT_NORTHWESTSOUTHEASTRESIZE,
  CT_COLUMNRESIZE,
  CT_ROWRESIZE,
  CT_MIDDLEPANNING,
  CT_EASTPANNING,
  CT_NORTHPANNING,
  CT_NORTHEASTPANNING,
  CT_NORTHWESTPANNING,
  CT_SOUTHPANNING,
  CT_SOUTHEASTPANNING,
  CT_SOUTHWESTPANNING,
  CT_WESTPANNING,
  CT_MOVE,
  CT_VERTICALTEXT,
  CT_CELL,
  CT_CONTEXTMENU,
  CT_ALIAS,
  CT_PROGRESS,
  CT_NODROP,
  CT_COPY,
  CT_NONE,
  CT_NOTALLOWED,
  CT_ZOOMIN,
  CT_ZOOMOUT,
  CT_GRAB,
  CT_GRABBING,
  CT_CUSTOM,
} cef_cursor_type_t;

// The cef callback function below is so to udpate the mouse cursor
// on the computer screen. Each cef_cursor_type_t is converted into
// a corresponding Upp::Image then Ctrl::OverrideCursor() is called.

void cef_window_OnCursorChange (void* owner, int cursor_type)
{
	Image image;
	switch(cursor_type) {
		case CT_VERTICALTEXT:
		case CT_IBEAM: image = Image::IBeam(); break;
		case CT_WAIT:  image = Image::Wait(); break;
		case CT_NONE:  image = Image::No(); break;

		case CT_NORTHEASTSOUTHWESTRESIZE:
		case CT_NORTHWESTSOUTHEASTRESIZE:
		case CT_MOVE:            image = Image::SizeAll(); break;
		case CT_COLUMNRESIZE:    image = Image::SizeHorz(); break;
		case CT_ROWRESIZE:       image = Image::SizeVert(); break;

		case CT_NORTHRESIZE:     image = Image::SizeTop(); break;
		case CT_WESTRESIZE:      image = Image::SizeLeft(); break;
		case CT_EASTRESIZE:      image = Image::SizeRight(); break;
		case CT_SOUTHRESIZE:     image = Image::SizeBottom(); break;

		case CT_NORTHWESTRESIZE: image = Image::SizeTopLeft(); break;
		case CT_NORTHEASTRESIZE: image = Image::SizeTopRight(); break;
		case CT_SOUTHEASTRESIZE: image = Image::SizeBottomLeft(); break;
		case CT_SOUTHWESTRESIZE: image = Image::SizeBottomRight(); break;

		case CT_CROSS: image = Image::Cross(); break;
		case CT_HAND:
		case CT_GRAB:
		case CT_GRABBING: image = Image::Hand(); break;

		default: image = Image::Arrow();
	}
	((BrowseEdit*)owner)->OverrideCursor(image);
}

