#include "icpu.h"
#include "myapp.h"

/**
 * The Constructor
 * Initializes all the variables
 *
 */
MyZKbMain::MyZKbMain():ZKbMainWidget((ZHeader::HEADER_TYPE)3,NULL , "ZMainWidget" , 0)
{
	// initialize the QCopChannel to monitory the EZX/System channel
	// all events are handled by slotReveivePhoneMsg() function
	QCopChannel *pSystemChannel = new QCopChannel("EZX/System", this);
	connect(pSystemChannel, SIGNAL(received(const QCString &,const QByteArray &)),
			this, SLOT(slotReveivePhoneMsg(const QCString &,const QByteArray &)));
	
}

void MyZKbMain::slotReveivePhoneMsg(const QCString& msg, const QByteArray& data)
{
    if (msg == "DPL_BKLGT_DIMMED")
	{
	    // change the frequency to minimum
	    system("pmtool -d 0");
        system("echo 133 > /sys/mpm/op");
        printf("Backlight off, frequency changed to 133");
	}
	
	if(msg == "LCDOn" || msg == "DPL_BKLGT_RESTORED")
	{
	    // change cpu to maximum
	    system("pmtool -d 0");
        system("echo 532 > /sys/mpm/op");
        printf("Backlight ON, frequency changed to 532");
	}
}

/**
 * The destructor
 */
MyZKbMain::~MyZKbMain()
{
     system("pmtool -d 1");
     printf("iCPU Stopped and CPU Frequency restored");
}
