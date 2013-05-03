#include "eiger.h"
#include "fakeeiger.h"
int main(int argc, char **argv)
{
	eiger::FakeEigerLoader f;
	f.parse();
	eiger::Disconnect();
}
