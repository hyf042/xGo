#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "gtp.h"
#include "brown.h"
#include "eater.h"
#include "uct/uct.h"
#include "minmax/minmax.h"
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
	
	//Go::Engine *engine = new Go::UCT::UCT<Go::UCT::Policy>();
	Go::NaiveSimulator *engine = new Go::UCT::UCT<Go::UCT::Policy>();
	engine->init("log.txt");
	// uncomment the following part if you want to debug a specific board
	/*engine->set_board_size(13);
	engine->set_board("\
.............\
.............\
XXXXX........\
XOOOX........\
XXXOX........\
..XOX........\
.............\
.............\
.............\
.............\
.............\
.............\
.............");
	Go::Point move;
	move = engine->generate_move(Go::WHITE);
	engine->play_move(move, Go::BLACK);
	printf("(%d, %d)\n", move.r, move.c);
	return 0;*/
	Go::GTP::instance().init(engine);
	Go::GTP::instance().run(stdin, f);
	delete engine;

	fclose(f);

	return 0;
}