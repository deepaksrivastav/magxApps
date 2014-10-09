#include "myapp.h"
#include "icpu.h"

/**
 * The main method
 */
int main( int argc, char **argv )
{
	MyZApplication* a = new MyZApplication( argc, argv );
	MyZKbMain*zmain=new MyZKbMain();
	a->setMainWidget(zmain);
    zmain->hide();
	a->exec();

	delete zmain;
	delete a;
   	return argc;
}
