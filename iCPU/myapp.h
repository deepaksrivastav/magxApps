#ifndef __MYAPP_H__
#define __MYAPP_H__

#include <ZApplication.h>
#include <stdio.h>

class MyZApplication : public ZApplication 
{
Q_OBJECT
public:
    MyZApplication(int argc, char **argv);
    virtual ~MyZApplication();

	protected slots:
		virtual void slotShutdown();	
		virtual void slotQuickQuit();
		virtual void slotRaise();
};

#endif



