#include "myapp.h"
#include <qcopchannel_qws.h>


MyZApplication::MyZApplication(int argc, char **argv) :
	ZApplication(argc, argv) {
}

MyZApplication::~MyZApplication() {

}

void MyZApplication::slotShutdown()
{
    printf("App slot shutdown\n");
}

void MyZApplication::slotQuickQuit()
{
	printf("App slot quick quit\n");
}

void MyZApplication::slotRaise()
{
	printf("Slot raise\n");
}
