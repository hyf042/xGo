#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "gtp.h"
#include "brown.h"
#include "eater.h"
#include "uct/MonteCarlo.h"
using namespace std;

int main(int argc, char **argv)
{
	// Optional custom random seed
	unsigned int random_seed = (unsigned int)time(0);
	if (argc > 1)
	sscanf(argv[1], "%u", &random_seed);
	srand(random_seed);
  
	/* Make sure that stdout is not block buffered. */
	setbuf(stdout, NULL);
	
	FILE *f = fopen("dump.txt", "w");
	
	Go::Engine *engine = new Go::UCT::MonteCarlo();
	engine->init("log.txt");
	Go::GTP::instance().init(engine);
	Go::GTP::instance().run(stdin, f);
	delete engine;

	fclose(f);

	return 0;
}