/*
 *	Author:	hyf042
 *	Date:	12-02-2013
 *
 *  Copyright 2013 hyf042. All rights reserved.
 */
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <vector>
#include <fstream>
#include "basic.h"

namespace Go
{
	class Engine
	{
	protected:
		std::ofstream log;
		float komi;
		int board_size;
	public:
		Engine(int board_size = 6, float komi = -3.14):board_size(board_size), komi(komi) {
		}
		virtual ~Engine() {
			if (log) {
				log.flush();
				log.close();
			}
		}
		
		int get_board_size() const { return board_size; }
		void set_board_size(int val) { board_size = val; }
		float get_komi() const { return komi; }
		void set_komi(float val) { komi = val; }

		virtual std::string get_version() { return "1.0"; }
		virtual std::string get_name() { return "test"; }
		virtual void init(std::string log_filename = "log.txt") {
			// init the log file
			if (log_filename != "") {
				if (log)
					log.close();
				log.open(log_filename);
			}
		}
		virtual void log_board() {
		}
		virtual void log_info(std::string info) {
		}

		// basic funcs
		virtual void clear_board() = 0;
		virtual bool board_empty() = 0;
		virtual int get_board(Point p) = 0;
		virtual bool valid_fixed_handicap(int handicap) = 0;
		virtual void place_fixed_handicap(int handicap) = 0;
		virtual void place_free_handicap(int handicap) = 0;

		// play go
		virtual void play_move(Point p, int color) = 0;
		virtual bool legal_move(Point p, int color) = 0;
		virtual Point generate_move(int color) = 0;

		// final evaluate
		virtual void compute_final_status() = 0;
		virtual int get_final_status(Point p) = 0;
		virtual void set_final_status(Point p, int color) = 0;
		virtual std::vector<Point> get_string(Point p) = 0;
	};
}

#endif