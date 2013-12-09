/*
*	Author:	hyf042
*	Date:	12-09-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _UCT_H_
#define _UCT_H_

#include "../naiveSimulator.h"

namespace Go
{
	namespace UCT {
		class UCT : public NaiveSimulator
		{
		public:
			override std::string get_name() { return "MyUCT"; }
			override Point generate_move(int color) {

			}
		};
	}
}

#endif