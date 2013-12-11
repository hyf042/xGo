/*
*	Author:	hyf042
*	Date:	12-03-2013
*
*  Copyright 2013 hyf042. All rights reserved.
*/
#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "engine.h"

namespace Go
{
	/*
	 * It's the naive simulator of go extracted from brown sample.
	 * If you want to use this simulate model (relatively slow),
	 * You only need to inherit from this class and override the genmove funciton
	 */
	class NaiveSimulator : public Engine
	{
	protected:
		Point ko;
		Point delta[4];

		short board[MAX_BOARD][MAX_BOARD];
		Point next_stone[MAX_BOARD][MAX_BOARD];
		short final_status[MAX_BOARD][MAX_BOARD];
	public:
		NaiveSimulator() {
			delta[0] = Point(-1, 0);
			delta[1] = Point(1, 0);
			delta[2] = Point(0, -1);
			delta[3] = Point(0, 1);
		}

		void log_board(short board[MAX_BOARD][MAX_BOARD], bool detail = false) {
			log << "-------------------------------" << std::endl;
			for (int i = 0; i < board_size; i++) {
				for (int j = 0; j < board_size; j++) {
					char c = '.';
					if (board[i][j] == WHITE)
						c = 'O';
					else if (board[i][j] == BLACK)
						c = 'X';
					log << c;
					if (detail)
						log << "(" << next_stone[i][j].r << "," << next_stone[i][j].c << ")";
				}
				log << std::endl;
			}
			log << "-------------------------------" << std::endl;
			log.flush();
		}
		override void set_board(char *s) {
			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++) {
					char c = '.';
					while (*s) {
						if (*s == 'O' || *s == 'X' || *s == '.') {
							c = *(s++);
							break;
						}
						s++;
					}
					if (c == 'O')
						board[i][j] = WHITE;
					else if (c == 'X')
						board[i][j] = BLACK;
					else
						board[i][j] = EMPTY;
				}
			reconstruct_next_stone();
		}

		override void init(std::string log_filename = "") {
			Engine::init(log_filename);

			clear_board();
			/*for (int i = 0; i < 20; i++) {
				int color = rand() % 2 ? BLACK : WHITE;
				Point p = generate_move(color);
				play_move(p, color);
			}*/
		}

		override void clear_board() {
			memset(board, 0, sizeof(board));
		}
		override bool board_empty() {
			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++)
					if (board[i][j] != EMPTY)
						return 0;
			return 1;
		}
		override int get_board(Point p) {
			return board[p.r][p.c];
		}
		override bool valid_fixed_handicap(int handicap) {
			if (handicap < 2 || handicap > 9)
				return 0;
			if (board_size % 2 == 0 && handicap > 4)
				return 0;
			if (board_size == 7 && handicap > 4)
				return 0;
			if (board_size < 7 && handicap > 0)
				return 0;

			return 1;
		}
		override void place_fixed_handicap(int handicap) {
			int low = board_size >= 13 ? 3 : 2;
			int mid = board_size / 2;
			int high = board_size - 1 - low;

			if (handicap >= 2) {
				play_move(Point(high, low), BLACK);   /* bottom left corner */
				play_move(Point(low, high), BLACK);   /* top right corner */
			}

			if (handicap >= 3)
				play_move(Point(low, low), BLACK);    /* top left corner */

			if (handicap >= 4)
				play_move(Point(high, high), BLACK);  /* bottom right corner */

			if (handicap >= 5 && handicap % 2 == 1)
				play_move(Point(mid, mid), BLACK);    /* tengen */

			if (handicap >= 6) {
				play_move(Point(mid, low), BLACK);    /* left edge */
				play_move(Point(mid, high), BLACK);   /* right edge */
			}

			if (handicap >= 8) {
				play_move(Point(low, mid), BLACK);    /* top edge */
				play_move(Point(high, mid), BLACK);   /* bottom edge */
			}
		}
		override void place_free_handicap(int handicap) {
			for (int k = 0; k < handicap; k++) {
				Point p = generate_move(BLACK);
				play_move(p, BLACK);
			}
		}

		override void play_move(Point p, int color) {
			int captured_stones = 0;

			/* Reset the ko point. */
			ko = Point(-1, -1);

			// log the entire board info into log file
			log_board(board);
			log << get_color_str(color) << ": (" << p.r << " " << p.c << ")" << std::endl;
			/* Nothing more happens if the move was a pass. */
			if (pass_move(p))
				return;

			/* If the move is a suicide we only need to remove the adjacent
			* friendly stones.
			*/
			if (suicide(p, color)) {
				for (int k = 0; k < 4; k++) {
					Point a = p+delta[k];
					if (on_board(a) && get_board(a) == color)
						remove_string(a);
				}
				return;
			}

			/* Not suicide. Remove captured opponent strings. */
			for (int k = 0; k < 4; k++) {
				Point a = p+delta[k];
				if (on_board(a)
					&& get_board(a) == other_color(color)
					&& !has_additional_liberty(a, p))
					captured_stones += remove_string(a);
			}

			/* Put down the new stone. Initially build a single stone string by
			* setting next_stone[pos] pointing to itself.
			*/
			board[p.r][p.c] = color;
			next_stone[p.r][p.c] = p;

			/* If we have friendly neighbor strings we need to link the strings
			* together.
			*/
			for (int k = 0; k < 4; k++) {
				Point a = p+delta[k];
				/* Make sure that the stones are not already linked together. This
				* may happen if the same string neighbors the new stone in more
				* than one direction.
				*/
				if (on_board(a) && board[a.r][a.c] == color && !same_string(p, a)) {
					/* The strings are linked together simply by swapping the the
					* next_stone pointers.
					*/
					std::swap(next_stone[a.r][a.c], next_stone[p.r][p.c]);
				}
			}

			/* If we have captured exactly one stone and the new string is a
			* single stone it may have been a ko capture.
			*/
			if (captured_stones == 1 && next_stone[p.r][p.c] == p) {
				Point a;
				/* Check whether the new string has exactly one liberty. If so it
				* would be an illegal ko capture to play there immediately. We
				* know that there must be a liberty immediately adjacent to the
				* new stone since we captured one stone.
				*/
				for (int k = 0; k < 4; k++) {
					a = p + delta[k];
					if (on_board(a) && get_board(a) == EMPTY)
						break;
				}

				if (!has_additional_liberty(p, a)) {
					ko = a;
				}
			}
		}
		override bool legal_move(Point p, int color) {
			int other = other_color(color);

			/* Pass is always legal. */
			if (pass_move(p))
				return 1;

			/* Already occupied. */
			if (get_board(p) != EMPTY)
				return 0;

			/* Illegal ko recapture. It is not illegal to fill the ko so we must
			* check the color of at least one neighbor.
			*/
			if (p == ko
				&& ((on_board(p+delta[0]) && get_board(p+delta[0]) == other)
				|| (on_board(p+delta[1]) && get_board(p+delta[1]) == other)))
				return 0;

			return 1;
		}
		
		override void compute_final_status() {
			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++)
					final_status[i][j] = UNKNOWN;

			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++) {
					Point p(i, j);
					if (get_board(p) == EMPTY)
						for (int k = 0; k < 4; k++) {
							Point a = p + delta[k];
							if (!on_board(a))
								continue;
							/* When the game is finished, we know for sure that (ai, aj)
							* contains a stone. The move generation algorithm would
							* never leave two adjacent empty vertices. Check the number
							* of liberties to decide its status, unless it's known
							* already.
							*
							* If we should be called in a non-final position, just make
							* sure we don't call set_final_status_string() on an empty
							* vertex.
							*/
							if (final_status[a.r][a.c] == UNKNOWN) {
								if (get_board(a) != EMPTY) {
									if (has_additional_liberty(a, p))
										set_final_status_string(a, ALIVE);
									else
										set_final_status_string(a, DEAD);
								}
							}
							/* Set the final status of the (i, j) vertex to either black
							* or white territory.
							*/
							if (final_status[p.r][p.c] == UNKNOWN) {
								if ((final_status[a.r][a.c] == ALIVE) ^ (get_board(a) == WHITE))
									final_status[p.r][p.c] = BLACK_TERRITORY;
								else
									final_status[p.r][p.c] = WHITE_TERRITORY;
							}
						}
				}
		}
		override int get_final_status(Point p) {
			return final_status[p.r][p.c];
		}
		override void set_final_status(Point p, int status) {
			final_status[p.r][p.c] = status;
		}
		override std::vector<Point> get_string(Point p) {
			std::vector<Point> ret;

			Point p2 = p;
			do {
				ret.push_back(p2);
				p2 = next_stone[p2.r][p2.c];
			} while (p2 != p);

			return ret;
		}

	protected:
		/* Basic Utility */
		int other_color(int color) const {
			return (WHITE + BLACK - (color));
		}
		std::string get_color_str(int color) const {
			if (color == 0)
				return "EMPTY";
			else if (color == 1)
				return "WHITE";
			else if (color == 2)
				return "BLACK";
			else
				return "UNKNOWN";
		}
		bool pass_move(Point p)
		{
			return p.r == -1 && p.c == -1;
		}
		bool on_board(Point p)
		{
			return p.r >= 0 && p.r < board_size && p.c >= 0 && p.c < board_size;
		}
		int get_color(Point p) const {
			return board[p.r][p.c];
		}

		/* Validator */
		bool suicide(Point p, int color)
		{
			for (int k = 0; k < 4; k++)
				if (provides_liberty(p+delta[k], p, color))
					return false;

			return true;
		}
		// check if a point p is provided liberty from p, 
		// a special case is that a is unfriendly but it's captured by p
		bool provides_liberty(Point a, Point p, int color)
		{
			/* A vertex off the board does not provide a liberty. */
			if (!on_board(a))
				return false;

			/* An empty vertex IS a liberty. */
			if (get_board(a) == EMPTY)
				return true;

			/* A friendly string provides a liberty to (i, j) if it currently
			* has more liberties than the one at (i, j).
			*/
			if (get_board(a) == color)
				return has_additional_liberty(a, p);

			/* An unfriendly string provides a liberty if and only if it is
			* captured, i.e. if it currently only has the liberty at (i, j).
			*/
			return !has_additional_liberty(a, p);
		}
		// check whether p has additional liberty except lib
		bool has_additional_liberty(Point p, Point lib)
		{
			Point pos = p;
			do {
				for (int k = 0; k < 4; k++) {
					Point b = pos+delta[k];
					if (on_board(b) && get_board(b) == EMPTY && (b.r != lib.r || b.c != lib.c))
						return true;
				}

				pos = next_stone[pos.r][pos.c];
			} while (pos != p);

			return false;
		}
		// count the liberty of p
		int count_liberty(Point p)
		{
			int cnt = 0;
			Point pos = p;
			point_set hash;
			do {
				for (int k = 0; k < 4; k++) {
					Point b = pos+delta[k];
					if (on_board(b) && get_board(b) == EMPTY && hash.find(b)==hash.end()) {
						cnt++;
						hash.insert(b);
					}
				}

				pos = next_stone[pos.r][pos.c];
			} while (pos != p);

			return cnt;
		}
		int count_additional_liberty(Point p, Point lib) {
			Point pos = p;
			point_set hash;
			do {
				for (int k = 0; k < 4; k++) {
					Point b = pos+delta[k];
					if (on_board(b) && get_board(b) == EMPTY && hash.find(b)==hash.end()) {
						hash.insert(b);
					}
				}

				pos = next_stone[pos.r][pos.c];
			} while (pos != p);

			int cnt = -1;
			for (int k = 0; k < 4; k++) {
				Point b = lib+delta[k];
				if (on_board(b) && get_board(b) == EMPTY && hash.find(b)==hash.end()) {
					cnt++;
					hash.insert(b);
				}
			}
			return cnt;
		}

		/* Strings */
		int remove_string(Point p)
		{
			Point pos = p;
			int removed = 0;
			do {
				board[pos.r][pos.c] = EMPTY;
				removed++;
				pos = next_stone[pos.r][pos.c];
			} while (pos != p);

			return removed;
		}
		int count_string(Point p)
		{
			Point pos = p;
			int count = 0;
			do {
				count++;
				pos = next_stone[pos.r][pos.c];
			} while (pos != p);

			return count;
		}
		int same_string(Point p1, Point p2)
		{
			Point p = p1;
			do {
				if (p == p2)
					return 1;
				p = next_stone[p.r][p.c];
			} while (p != p1);

			return 0;
		}
		void set_final_status_string(Point p, int status)
		{
			Point p2 = p;
			do {
				final_status[p2.r][p2.c] = status;
				p2 = next_stone[p2.r][p2.c];
			} while (p2 != p);
		}

	public:
		/* Evaluator Utility */
		// generate all the legal moves
		std::vector<Point> generate_legal_moves(int color) {
			std::vector<Point> moves;

			for (int ai = 0; ai < board_size; ai++)
				for (int aj = 0; aj < board_size; aj++) {
					Point a(ai, aj);
					/* Consider moving at (ai, aj) if it is legal and not suicide. */
					if (legal_move(a, color)
						&& !suicide(a, color)) {
							/* Further require the move not to be suicide for the opponent... */
							if (!suicide(a, other_color(color)))
								moves.push_back(a);
							else {
								/* ...however, if the move captures at least one stone,
								* consider it anyway.
								*/
								for (int k = 0; k < 4; k++) {
									Point b = a+delta[k];
									if (on_board(b) && get_board(b) == other_color(color)) {
										moves.push_back(a);
										break;
									}
								}
							}
					}
				}

			return moves;
		}
		// generate good and legal moves, here is a fake way just to generate all legal moves
		virtual std::vector<Point> generate_good_moves(int color) {
			return generate_legal_moves(color);
		}
		// evaluate the position value of a point
		int pos_evaluate(Point p) {
			if (!on_board(p))
				return -1;
			else
				return (board_size-std::min(p.r, board_size-p.r-1)) + (board_size-std::min(p.c, board_size-p.c-1));
		}
		// reconstruct the next_stone array
		void reconstruct_next_stone() {
			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++)
					next_stone[i][j] = Point(i,j);
			for (int i = 0; i < board_size; i++)
				for (int j = 0; j < board_size; j++) {
					Point p = Point(i, j);
					Point a = p + delta[1], b = p + delta[3];
					if (on_board(a) && get_color(p) == get_color(a) && !same_string(a, p))
						std::swap(next_stone[a.r][a.c], next_stone[p.r][p.c]);
					if (on_board(b) && get_color(p) == get_color(b) && !same_string(b, p))
						std::swap(next_stone[b.r][b.c], next_stone[p.r][p.c]);
				}
		}
		bool is_final_territory(Point p, int color) {
			if (!on_board(p))
				return false;

			if (color == BLACK) {
				if (final_status[p.r][p.c] == BLACK_TERRITORY)
					return true;
				if (board[p.r][p.c] != EMPTY && (board[p.r][p.c] == WHITE ^ final_status[p.r][p.c] == ALIVE))
					return true;
			}
			else if (color == WHITE)  {
				if (final_status[p.r][p.c] == WHITE_TERRITORY)
					return true;
				if (board[p.r][p.c] != EMPTY && (board[p.r][p.c] == BLACK ^ final_status[p.r][p.c] == ALIVE))
					return true;
			}
			return false;
		}
	};
}

#endif