#include "eiger.h"
#include "mpifakeeiger.h"
#include <iostream>
int main(int argc, char **argv)
{
	try {
    std::cout << "Beginning load..." << std::endl;
		eiger::FakeEigerLoader f;
		f.parse();
		eiger::Disconnect();
    std::cout << "Done!" << std::endl;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
	}
}
