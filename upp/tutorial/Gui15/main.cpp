#include <CtrlLib/CtrlLib.h>

using namespace Upp;

struct MyAppWindow : TopWindow {
	Button button;

	MyAppWindow() {
		Title("My application with font-zoomed button").Sizeable();
		*this << button.SetLabel("Button").LeftPosZ(10, 64).TopPosZ(10, 24);
	}
};

GUI_APP_MAIN
{
	MyAppWindow app;
	app.Run();
}
