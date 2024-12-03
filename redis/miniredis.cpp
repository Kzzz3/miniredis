#include "test.h"
#include "server.h"

int main()
{
	//test();
	//std::cout << "<-----------------------------------test passed----------------------------------->" << std::endl;
	server s;
	s.start();
	return 0;
}