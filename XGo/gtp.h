/*
 *	Author:	hyf042
 *	Date:	12-02-2013
 *
 *  Copyright 2013 hyf042. All rights reserved.
 */
#ifndef _GTP_H_
#define _GTP_H_

#include <string>
#include <functional>
#include <map>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include "engine.h"

namespace Go
{
	class GTP
	{
		typedef std::function<int(char*)> command_handler;
		typedef std::function<void(Point, Point&)> transform_handler;

		// Constants & Codes
		static const int GTP_BUFSIZE = 1000;

		enum GtpStatus{GTP_QUIT=-1, GTP_OK=0, GTP_FATAL=1};
		enum GtpRetCode{GTP_SUCCESS=0, GTP_FAILURE=1};

		// GTP Variables
		int current_id;
		transform_handler vertex_transform_input_hook;
		transform_handler vertex_transform_output_hook;
		std::map<std::string, command_handler> commands;
		Engine *engine;

		// implement singleton
		static GTP *_instance;
		GTP():engine(NULL) {}
		~GTP() {
			if (engine != NULL) {
				delete engine;
				engine = NULL;
			}
		}

	public:
		static GTP& instance() {
			if (_instance == NULL)
				_instance = new GTP();
			return *_instance;
		}

		void init(Engine *engine) {
			if (this->engine)
				delete this->engine;
			this->engine = engine;

			commands["protocol_version"]	= std::bind(&GTP::gtp_protocol_version, this, std::placeholders::_1);
			commands["name"]				= std::bind(&GTP::gtp_name, this, std::placeholders::_1);
			commands["version"]				= std::bind(&GTP::gtp_version, this, std::placeholders::_1);
			commands["known_command"]		= std::bind(&GTP::gtp_known_command, this, std::placeholders::_1);
			commands["list_commands"]		= std::bind(&GTP::gtp_list_commands, this, std::placeholders::_1);
			commands["quit"]				= std::bind(&GTP::gtp_quit, this, std::placeholders::_1);
			commands["boardsize"]			= std::bind(&GTP::gtp_boardsize, this, std::placeholders::_1);
			commands["clear_board"]			= std::bind(&GTP::gtp_clear_board, this, std::placeholders::_1);
			commands["komi"]				= std::bind(&GTP::gtp_komi, this, std::placeholders::_1);
			commands["fixed_handicap"]		= std::bind(&GTP::gtp_fixed_handicap, this, std::placeholders::_1);
			commands["place_free_handicap"]	= std::bind(&GTP::gtp_place_free_handicap, this, std::placeholders::_1);
			commands["set_free_handicap"]	= std::bind(&GTP::gtp_set_free_handicap, this, std::placeholders::_1);
			commands["play"]				= std::bind(&GTP::gtp_play, this, std::placeholders::_1);
			commands["genmove"]				= std::bind(&GTP::gtp_genmove, this, std::placeholders::_1);
			commands["final_score"]			= std::bind(&GTP::gtp_final_score, this, std::placeholders::_1);
			commands["final_status_list"]	= std::bind(&GTP::gtp_final_status_list, this, std::placeholders::_1);
			commands["showboard"]			= std::bind(&GTP::gtp_showboard, this, std::placeholders::_1);
		}
		void run(FILE *gtp_input, FILE *gtp_dump_commands) {
			char line[GTP_BUFSIZE];
			char command[GTP_BUFSIZE];
			char *p;
			int i;
			int n;
			int status = GTP_OK;

			while (status == GTP_OK) {
				/* Read a line from gtp_input. */
				if (!fgets(line, GTP_BUFSIZE, gtp_input))
					break; /* EOF or some error */

				if (gtp_dump_commands)
					fputs(line, gtp_dump_commands);

				/* Preprocess the line. */
				for (i = 0, p = line; line[i]; i++) {
					char c = line[i];
					/* Convert HT (9) to SPACE (32). */
					if (c == 9)
						*p++ = 32;
					/* Remove CR (13) and all other control characters except LF (10). */
					else if ((c > 0 && c <= 9)
						|| (c >= 11 && c <= 31)
						|| c == 127)
						continue;
					/* Remove comments. */
					else if (c == '#')
						break;
					/* Keep ordinary text. */
					else
						*p++ = c;
				}
				/* Terminate string. */
				*p = 0;

				p = line;

				/* Look for an identification number. */
				if (sscanf(p, "%d%n", &current_id, &n) == 1)
					p += n;
				else
					current_id = -1; /* No identification number. */

				/* Look for command name. */
				if (sscanf(p, " %s %n", command, &n) < 1)
					continue; /* Whitespace only on this line, ignore. */
				p += n;

				/* Search the list of commands and call the corresponding function
				* if it's found.
				*/
				if (commands.find(command) != commands.end())
					status = commands[command](p);
				else
					gtp_failure("unknown command");

				if (status == GTP_FATAL)
					gtp_panic();
			}
		}

	public:
		/*
	     * GTP Command Handlers
		 */
		/* We are talking version 2 of the protocol. */
		int gtp_protocol_version(char *s)
		{
		  return gtp_success("2");
		}
		int gtp_name(char *s)
		{
			return gtp_success(engine->get_name().c_str());
		}
		int gtp_version(char *s)
		{
			return gtp_success(engine->get_version().c_str());
		}
		int gtp_known_command(char *s)
		{
			char command_name[GTP_BUFSIZE];

			/* If no command name supplied, return false (this command never
			* fails according to specification).
			*/
			if (sscanf(s, "%s", command_name) < 1)
			return gtp_success("false");

			if (commands.find(command_name) != commands.end())
				return gtp_success("true");
			else
				return gtp_success("false");
		}
		int gtp_list_commands(char *s)
		{
			gtp_start_response(GTP_SUCCESS);

			for (auto ptr = commands.begin(); ptr != commands.end(); ptr++)
				gtp_printf("%s\n", ptr->first.c_str());

			gtp_printf("\n");
			return GTP_OK;
		}
		int gtp_quit(char *s)
		{
			gtp_success("");
			return GTP_QUIT;
		}
		int gtp_boardsize(char *s) {
			int boardsize;

			if (sscanf(s, "%d", &boardsize) < 1)
			return gtp_failure("boardsize not an integer");
  
			if (boardsize < MIN_BOARD || boardsize > MAX_BOARD)
			return gtp_failure("unacceptable size");

			engine->set_board_size(boardsize);
			engine->init();
  
			return gtp_success("");
		}
		int gtp_clear_board(char *s)
		{
			engine->clear_board();
			return gtp_success("");
		}
		int gtp_komi(char *s)
		{
			float komi = 0;
			if (sscanf(s, "%f", &komi) < 1)
			return gtp_failure("komi not a float");
			
			engine->set_komi(komi);
			return gtp_success("");
		}
		int gtp_fixed_handicap(char *s)
		{
		  return place_handicap(s, 1);
		}
		int gtp_place_free_handicap(char *s)
		{
		  return place_handicap(s, 0);
		}
		int gtp_set_free_handicap(char *s)
		{
		  int i, j;
		  int n;
		  int handicap = 0;
  
		  if (!engine->board_empty())
			return gtp_failure("board not empty");

		  while ((n = gtp_decode_coord(s, &i, &j)) > 0) {
			s += n;
    
			if (engine->get_board(Point(i, j)) != EMPTY) {
			  engine->clear_board();
			  return gtp_failure("repeated vertex");
			}

			engine->play_move(Point(i, j), BLACK);
			handicap++;
		  }

		  if (sscanf(s, "%*s") != EOF) {
			  engine->clear_board();
			  return gtp_failure("invalid coordinate");
		  }

		  if (handicap < 2 || handicap >= engine->get_board_size() * engine->get_board_size()) {
			  engine->clear_board();
			  return gtp_failure("invalid handicap");
		  }

		  return gtp_success("");
		}
		int gtp_play(char *s)
		{
		  int i, j;
		  int color = EMPTY;

		  if (!gtp_decode_move(s, &color, &i, &j))
			return gtp_failure("invalid color or coordinate");
		  
		  if (!engine->legal_move(Point(i, j), color)) 
			return gtp_failure("illegal move");

		  engine->play_move(Point(i, j), color);
		  return gtp_success("");
		}
		int gtp_genmove(char *s)
		{
		  int color = EMPTY;

		  if (!gtp_decode_color(s, &color))
			return gtp_failure("invalid color");

		  Point p = engine->generate_move(color);
		  engine->play_move(p, color);

		  gtp_start_response(GTP_SUCCESS);
		  gtp_mprintf("%m", p.r, p.c);
		  return gtp_finish_response();
		}
		int gtp_final_score(char *s)
		{
			float score = engine->get_komi();
			int i, j;

			engine->compute_final_status();
			for (i = 0; i < engine->get_board_size(); i++)
				for (j = 0; j < engine->get_board_size(); j++) {
					int status = engine->get_final_status(Point(i, j));
					if (status == BLACK_TERRITORY)
						score--;
					else if (status == WHITE_TERRITORY)
						score++;
					else if ((status == ALIVE) ^ (engine->get_board(Point(i, j)) == WHITE))
						score--;
					else
						score++;
			}

			if (score > 0.0)
				return gtp_success("W+%3.1f", score);
			if (score < 0.0)
				return gtp_success("B+%3.1f", -score);
			return gtp_success("0");
		}
		int gtp_final_status_list(char *s)
		{
			int n;
			int i, j;
			int status = UNKNOWN;
			char status_string[GTP_BUFSIZE];
			int first_string;

			if (sscanf(s, "%s %n", status_string, &n) != 1)
				return gtp_failure("missing status");

			if (!strcmp(status_string, "alive"))
				status = ALIVE;
			else if (!strcmp(status_string, "dead"))
				status = DEAD;
			else if (!strcmp(status_string, "seki"))
				status = SEKI;
			else
				return gtp_failure("invalid status");

			engine->compute_final_status();

			gtp_start_response(GTP_SUCCESS);

			first_string = 1;
			for (i = 0; i < engine->get_board_size(); i++)
				for (j = 0; j < engine->get_board_size(); j++)
					if (engine->get_final_status(Point(i, j)) == status) 
					{
						int k;
						int stonei[MAX_BOARD * MAX_BOARD];
						int stonej[MAX_BOARD * MAX_BOARD];
						std::vector<Point> stones = engine->get_string(Point(i, j));
						int num_stones = stones.size();
						/* Clear the status so we don't find the string again. */
						for (k = 0; k < num_stones; k++) {
							stonei[k] = stones[k].r, stonej[k] = stones[k].c;
							engine->set_final_status(stones[k], UNKNOWN);
						}

						if (first_string)
							first_string = 0;
						else
							gtp_printf("\n");
	
						gtp_print_vertices(num_stones, stonei, stonej);
					}

			return gtp_finish_response();
		}
		int gtp_showboard(char *s)
		{
			int i, j;
			int symbols[3] = {'.', 'O', 'X'};
  
			gtp_start_response(GTP_SUCCESS);
			gtp_printf("\n");

			letters();
  
			for (i = 0; i < engine->get_board_size(); i++) {
				printf("\n%2d", engine->get_board_size() - i);
    
				for (j = 0; j < engine->get_board_size(); j++)
					printf(" %c", symbols[engine->get_board(Point(i, j))]);

				printf(" %d", engine->get_board_size() - i);
			}
  
			printf("\n");
			letters();
			return gtp_finish_response();
		}

		/*
		 * Utilities
		 */
		int gtp_success(const char *format, ...)
		{
			va_list ap;
			gtp_start_response(GTP_SUCCESS);
			va_start(ap, format);
			vfprintf(stdout, format, ap);
			va_end(ap);
			return gtp_finish_response();
		}
		/* Write a full failure response. The call is identical to gtp_success. */
		int gtp_failure(const char *format, ...)
		{
			va_list ap;
			gtp_start_response(GTP_FAILURE);
			va_start(ap, format);
			vfprintf(stdout, format, ap);
			va_end(ap);
			return gtp_finish_response();
		}
		/* Write success or failure indication plus identity number if one was
		* given.
		*/
		void gtp_start_response(int status)
		{
			if (status == GTP_SUCCESS)
				gtp_printf("=");
			else
				gtp_printf("?");

			if (current_id < 0)
				gtp_printf(" ");
			else
				gtp_printf("%d ", current_id);
		}
		/* Finish a GTP response by writing a double newline and returning GTP_OK. */
		int	gtp_finish_response()
		{
			gtp_printf("\n\n");
			return GTP_OK;
		}
		/* This currently works exactly like printf. */
		void gtp_printf(const char *format, ...)
		{
			va_list ap;
			va_start(ap, format);
			vfprintf(stdout, format, ap);
			va_end(ap);
		}
		/* Write a panic message. */
		void gtp_panic()
		{
			gtp_printf("! panic\n\n");
		}
		int gtp_decode_coord(char *s, int *i, int *j)
		{
			char column;
			int row;
			int n;

			assert(engine->get_board_size() > 0);

			if (sscanf(s, " %c%d%n", &column, &row, &n) != 2)
				return 0;

			if (tolower((int) column) == 'i')
				return 0;
			*j = tolower((int) column) - 'a';
			if (tolower((int) column) > 'i')
				--*j;

			*i = engine->get_board_size() - row;

			if (*i < 0 || *i >= engine->get_board_size() || *j < 0 || *j >= engine->get_board_size())
				return 0;

			if (vertex_transform_input_hook) {
				Point ret = Point();
				vertex_transform_input_hook(Point(*i, *j), ret);
				*i = ret.r, *j = ret.c;
			}

			return n;
		}
		/*
		* This function works like printf, except that it only understands
		* very few of the standard formats, to be precise %c, %d, %f, %s.
		* But it also accepts %m, which takes two integers and writes a vertex,
		* and %C, which takes a color value and writes a color string.
		*/
		void gtp_mprintf(const char *fmt, ...)
		{
			va_list ap;
			va_start(ap, fmt);

			for (; *fmt; ++fmt) {
				if (*fmt == '%') {
					switch (*++fmt) {
					case 'c':
						{
							/* rules of promotion => passed as int, not char */
							int c = va_arg(ap, int);
							putc(c, stdout);
							break;
						}
					case 'd':
						{
							int d = va_arg(ap, int);
							fprintf(stdout, "%d", d);
							break;
						}
					case 'f':
						{
							double f = va_arg(ap, double); /* passed as double, not float */
							fprintf(stdout, "%f", f);
							break;
						}
					case 's':
						{
							char *s = va_arg(ap, char *);
							fputs(s, stdout);
							break;
						}
					case 'm':
						{
							int m = va_arg(ap, int);
							int n = va_arg(ap, int);
							gtp_print_vertex(m, n);
							break;
						}
					case 'C':
						{
							int color = va_arg(ap, int);
							if (color == WHITE)
								fputs("white", stdout);
							else if (color == BLACK)
								fputs("black", stdout);
							else
								fputs("empty", stdout);
							break;
						}
					default:
						fprintf(stdout, "\n\nUnknown format character '%c'\n", *fmt);
						break;
					}
				}
				else
					putc(*fmt, stdout);
			}
			va_end(ap);
		}
		/* Convert a string describing a color, "b", "black", "w", or "white",
		* to GNU Go's integer representation of colors. Return the number of
		* characters read from the string s.
		*/
		int gtp_decode_color(char *s, int *color)
		{
			char color_string[7];
			int i;
			int n;

			assert(engine->get_board_size() > 0);

			if (sscanf(s, "%6s%n", color_string, &n) != 1)
				return 0;

			for (i = 0; i < (int) strlen(color_string); i++)
				color_string[i] = tolower((int) color_string[i]);

			if (strcmp(color_string, "b") == 0
				|| strcmp(color_string, "black") == 0)
				*color = BLACK;
			else if (strcmp(color_string, "w") == 0
				|| strcmp(color_string, "white") == 0)
				*color = WHITE;
			else
				return 0;

			return n;
		}
		/* Convert a move, i.e. "b" or "w" followed by a vertex to a color and
		* coordinates. Return the number of characters read from the string
		* s. The vertex may be "pass" and then the coordinates are set to (-1, -1).
		*/
		int gtp_decode_move(char *s, int *color, int *i, int *j)
		{
			int n1, n2;
			int k;

			assert(engine->get_board_size() > 0);

			n1 = gtp_decode_color(s, color);
			if (n1 == 0)
				return 0;

			n2 = gtp_decode_coord(s + n1, i, j);
			if (n2 == 0) {
				char buf[6];
				if (sscanf(s + n1, "%5s%n", buf, &n2) != 1)
					return 0;
				for (k = 0; k < (int) strlen(buf); k++)
					buf[k] = tolower((int) buf[k]);
				if (strcmp(buf, "pass") != 0)
					return 0;
				*i = -1;
				*j = -1;
			}

			return n1 + n2;
		}
		/* Write a number of space separated vertices. The moves are sorted
		* before being written.
		*/
		void gtp_print_vertices(int n, int movei[], int movej[])
		{
			int k;
			int ri, rj;

			assert(engine->get_board_size() > 0);

			sort_moves(n, movei, movej);
			for (k = 0; k < n; k++) {
				if (k > 0)
					gtp_printf(" ");
				if (movei[k] == -1 && movej[k] == -1)
					gtp_printf("PASS");
				else if (movei[k] < 0 || movei[k] >= engine->get_board_size()
					|| movej[k] < 0 || movej[k] >= engine->get_board_size())
					gtp_printf("??");
				else {
					if (vertex_transform_output_hook) {
						Point ret;
						vertex_transform_output_hook(Point(movei[k], movej[k]), ret);
						ri = ret.r;
						rj = ret.c;
					}
					else {
						ri = movei[k];
						rj = movej[k];
					}
					gtp_printf("%c%d", 'A' + rj + (rj >= 8), engine->get_board_size() - ri);
				}
			}
		}
		/* Write a single move. */
		void gtp_print_vertex(int i, int j)
		{
			gtp_print_vertices(1, &i, &j);
		}
		/* This a bubble sort. Given the expected size of the sets to
		* sort, it's probably not worth the overhead to set up a call to
		* qsort.
		*/
		void sort_moves(int n, int movei[], int movej[])
		{
			int b, a;
			for (b = n-1; b > 0; b--) {
				for (a = 0; a < b; a++) {
					if (movei[a] > movei[b]
					|| (movei[a] == movei[b] && movej[a] > movej[b])) {
						int tmp;
						tmp = movei[b];
						movei[b] = movei[a];
						movei[a] = tmp;
						tmp = movej[b];
						movej[b] = movej[a];
						movej[a] = tmp;
					}
				}
			}
		}
		/* Write a row of letters, skipping 'I'. */
		void letters(void)
		{
			int i;
  
			printf("  ");
			for (i = 0; i < engine->get_board_size(); i++)
			printf(" %c", 'A' + i + (i >= 8));
		}
		int place_handicap(char *s, int fixed)
		{
		  int handicap;
		  int m, n;
		  int first_stone = 1;

		  if (!engine->board_empty())
			return gtp_failure("board not empty");

		  if (sscanf(s, "%d", &handicap) < 1)
			return gtp_failure("handicap not an integer");
  
		  if (handicap < 2)
			return gtp_failure("invalid handicap");

		  if (fixed && !engine->valid_fixed_handicap(handicap))
			return gtp_failure("invalid handicap");

		  if (fixed)
			engine->place_fixed_handicap(handicap);
		  else
			engine->place_free_handicap(handicap);

		  gtp_start_response(GTP_SUCCESS);
		  for (m = 0; m < engine->get_board_size(); m++)
			for (n = 0; n < engine->get_board_size(); n++)
			  if (engine->get_board(Point(m, n)) != EMPTY) {
				if (first_stone)
				  first_stone = 0;
				else
				  gtp_printf(" ");
			  gtp_mprintf("%m", m, n);
		  }
		  return gtp_finish_response();
		}
	};
	GTP *GTP::_instance = NULL;
}

#endif