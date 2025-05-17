#include <library_main.h>

int main()
{
	library_main* lib_main = new library_main;
	lib_main->run();
	delete lib_main;
	return 0;
}