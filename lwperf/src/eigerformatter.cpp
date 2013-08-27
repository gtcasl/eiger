#include <mpi.h>
#include "eigerformatter.h"

#ifdef TEST
void do2() {
	eigerformatter log("formattest.log", false, true);
	log.addCol("apples",D);
	log.addCol("oranges",I);
	log.put(2.0);
	log.put(3);
	log.nextrow();
}
void do1() {
	eigerformatter log("formattest.log", true);
	log.addCol("apples",D);
	log.addCol("oranges",I);
	log.writeheaders();
	log.put(2.0);
	log.put(3);
	log.nextrow();
	fprintf(log.file(),"oneliner\n");

}
int main() {
	do1();
	do2();
	return 0;
}
#endif // test
