#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "gtp.h"
#include "brown.h"
#include "eater.h"
#include "uct/MonteCarlo.h"
#include "uct/uct.h"
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
	
	Go::Engine *engine = new Go::UCT::UCT<Go::UCT::Policy>();
	engine->init("log.txt");
	/*engine->set_board_size(13);
	engine->set_board("\
.............\
.............\
X............\
.............\
.............\
.............\
.............\
.............\
.............\
.............\
.............\
.............\
.............");
	Go::Point move;
	move = engine->generate_move(Go::WHITE);
	engine->play_move(move, Go::WHITE);
	move = engine->generate_move(Go::BLACK);
	engine->play_move(move, Go::BLACK);
	move = engine->generate_move(Go::WHITE);
	engine->play_move(move, Go::WHITE);
	printf("(%d, %d)\n", move.r, move.c);
	return 0;*/
	Go::GTP::instance().init(engine);
	Go::GTP::instance().run(stdin, f);
	delete engine;

	fclose(f);

	return 0;
}