/*
 * Copyright (c) 2023 ssleert <smnrbkv@proton.me>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFSIZE 2048
#define DEFAULT_INTERVAL 2.0
#define MAX_ARGS 128

#define VERSION "0.0.1"
#define RANGE_CHECK(n) (n < DBL_MAX && n > DBL_MIN)

#define error(status, ...) do {        \
	fprintf(stderr, __VA_ARGS__);  \
	exit(status);                  \
} while (0)

static void usage(void);
static void at_sigint();
static void clear_screen(void);
static void alt_screen(void);
static void main_screen(void);
static void revprint(const char *str);
static void move_cursor(unsigned short x, unsigned short y);
static void draw_bar(double interval, const char *cmd);
static long get_mscs(double secs);
static void ms_sleep(long ms);

int
main(int argc, char *argv[]) 
{
	if (argc > MAX_ARGS)
		error(4,
		    "args: %d invalid (MAX_ARGS limit reached)\n",
		    argc);

	char *cmd = NULL;
	double interval = DEFAULT_INTERVAL;
	bool title = true, halt = false, clear = true;

	char ch;
	while ((ch = getopt(argc, argv, "txsn:c:")) != -1) {
		switch (ch) {
		case 't':
			title = false;
			break;
		case 'x':
			halt = true;
			break;
		case 's':
			clear = false;
			break;
		case 'n':
			;char *endptr;

			interval = strtod(optarg, &endptr);
			if (optarg == endptr)
				error(2,
				    "interval: %s invalid (not all digits)\n",
				    optarg);
			else if (errno == ERANGE && RANGE_CHECK(interval))
				error(2,
				    "interval: %s invalid (overflow detected)\n",
				    optarg);
			else if (errno != 0 && interval == 0.0)
				error(2,
				    "interval %s invalid (unspecified error)\n",
				    optarg);

			if (interval == 0.000)
				error(2,
				    "interval: %.3f invalid (equal to zero)\n",
				    interval);
			break;
		case 'c':
			;size_t s = strlen(optarg) + 1;
			cmd = malloc(s * sizeof(char));
			if (cmd == NULL)
				error(3,
				    "command: %s invalid (allocation failed)\n",
				    optarg);
			memcpy(cmd, optarg, s);
			break;
		default:
			usage();
		}
	}	


	if (cmd == NULL) {
		bool finded = false;
		int index = 0;
		for (int i = argc - 1; i > 0; --i) {
			if (strcmp(argv[i], "--") != 0)
				continue;
			finded = true;
			index = i + 1;
		}
		if (finded == false)
			error(4,
			    "args: -c is zero and '--' doesnt finded\n");
		if (index == argc)
			error(4,
			    "args: no command after '--'\n");

		size_t cmd_size = BUFSIZE;
		cmd = malloc(cmd_size);
		if (cmd == NULL)
			error(2,
			    "command: failed to allocate space for cmd\n");
		cmd[0] = '\0';

		for (int i = index; i < argc; ++i) {
			if (cmd_size + strlen(argv[i]) + 1 >= cmd_size) {
				cmd_size *= 2;
				cmd = realloc(cmd, cmd_size);
				if (cmd == NULL)
					error(2,
					    "command: cmd_size = %ld (reallocation failed)\n",
					    cmd_size);
			}
			strlcat(cmd, argv[i], cmd_size);
			if (i < argc - 1)
				strlcat(cmd, " ", cmd_size);
		}
	}

	if (cmd[0] == '-')
		error(4,
		    "args: cmd arg starts with '-'\n");

	signal(SIGINT, at_sigint);
	atexit(main_screen);

	alt_screen();
	while (true) {
		if (clear == true) {
			clear_screen();
			if (title == true)
				draw_bar(interval, cmd);
			fflush(stdout);
		}

		int status = system(cmd);
		if (status != 0) {
			fprintf(stderr,
			    "exit: %d\n\n",
			    status);
			if (halt == true)
				exit(status);
		}

		ms_sleep(get_mscs(interval));
	}
	return EXIT_SUCCESS;
}

static void
usage(void) 
{
	fprintf(stderr,
	    "usage: %s [-txsn:c:] -- [command ...]\n",
	    getprogname());
	exit(1);
}

static void 
at_sigint()
{
	exit(0);
}

static void
alt_screen(void) 
{
	printf("\033[?1049h");
}

static void
main_screen(void)
{
	printf("\033[?1049l");
}

static void
revprint(const char *str)
{
	printf("\033[%ldD", strlen(str) - 1);
	printf("%s", str);
}

static void
clear_screen(void)
{
	// copied from clear(1) command
	printf("\033[2J");
}

static void
move_cursor(unsigned short x, unsigned short y)
{
	printf("\033[%d;%dH", y, x);
}

static void
draw_bar(double interval, const char *str)
{
	move_cursor(1, 1);

	printf("Every %0.3f: %s", interval, str);

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	move_cursor(w.ws_col, 1);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char time_buf[BUFSIZE];
	size_t r = strftime(time_buf, BUFSIZE, "%c", &tm);
	if (r < 1)
		strlcpy(time_buf, "TIME ERROR", BUFSIZE);
	revprint(time_buf);
	move_cursor(1, 2);
}

static long
get_mscs(double secs)
{
	secs *= 1000;
	long mscs = labs((long)secs);
	return mscs;
}

static void 
ms_sleep(long ms)
{
	long sec = (ms / 1000);
	ms = ms - (sec * 1000);
	struct timespec interval = {
		.tv_sec = sec,
		.tv_nsec = ms * 1000000L,
	};
	while (nanosleep(&interval, &interval) == -1);
}
