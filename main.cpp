#include <fstream>
#include <string>
#include "stuff.h"


std::ofstream out;
int main( int argc, char** argv )
{
	try {
		Initialize( argc, argv );
		while( true )
			Update();
	} catch( std::string error ) {
		out << error << std::endl;
	} catch( exit_success e ) {
	}
	glfwTerminate();
	out.close();

	exit( EXIT_SUCCESS );
};

