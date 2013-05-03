#include "eiger.h"
#include "mpifakeeiger.h"
int main(int argc, char **argv)
{
	try {
		eiger::FakeEigerLoader f;
		f.parse();
		eiger::Disconnect();
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
	}
}
