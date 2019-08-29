#ifndef _DispatcherTest_DispatcherTest_h
#define _DispatcherTest_DispatcherTest_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <DispatcherTest2/DispatcherTest.lay>
#include <CtrlCore/lay.h>

#include <Dispatcher/Dispatcher.hpp>

class MyPane
	: public WithPane<ParentCtrl>
	, public Dispatchable<Value>
	, public Dispatchable<int>
{
public:
	typedef MyPane CLASSNAME;
	MyPane();

	virtual void Dispatch(const Value& o);
	virtual void Dispatch(const int& o);
};

class DispatcherTest : public WithDispatcherTestLayout<TopWindow> 
{
public:
	typedef DispatcherTest CLASSNAME;
	DispatcherTest();
	
	void sliderCB();
	void slideriCB();

	Dispatcher<Value> disp;
	Dispatcher<int> dispi;

	Splitter splith1, splith2, splitv;
	MyPane pane1, pane2, pane3, pane4;
};

#endif

