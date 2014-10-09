#include <ZApplication.h>
#include "magxOffice.h"

int main(int argc,char *argv[])
{
	ZApplication app(argc,argv);
	MagxOffice *fileDialog = new MagxOffice();
	app.showMainWidget(fileDialog);
	return app.exec();
}
