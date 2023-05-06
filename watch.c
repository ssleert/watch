#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

#define DEFAULT_INTERVAL 1.0
#define MAX_ARGS 128

#define NAME "watch"
#define VERSION "0.0.1"
#define RANGE_CHECK(n) (n > DBL_MAX - 10.0 || n < DBL_MIN + 10.0)


static void usage(void);
static void clear_screen(void);
static long get_mscs(double secs);

int
main(int argc, char *argv[]) 
{
	char *cmd = NULL;
	double interval = DEFAULT_INTERVAL;
	bool notitle = false, halt = false;

	char ch;
	while ((ch = getopt(argc, argv, "txn:c:")) != -1) {
		switch (ch) {
		case 't':
			notitle = true;
			break;
		case 'x':
			halt = true;
			break;
		case 'n':
			;char *endptr = NULL;
			interval = strtod(optarg, &endptr);
			if (optarg == endptr) {
				fprintf(stderr,
				    "interval: %s invalid (not all digits)\n",
				    optarg);
				return 2;
			} else if (errno == ERANGE && RANGE_CHECK(interval)) {
				fprintf(stderr,
				    "interval: %s invalid (overflow detected)\n",
				    optarg);
				return 2;
			} else if (errno != 0 && interval == 0) {
				fprintf(stderr,
				    "interval %s invalid (unspecified error)\n",
				    optarg);
				return 2;
			}
			break;
		case 'c':
			;size_t s = strlen(optarg) + 1;
			cmd = malloc(s * sizeof(char));
			if (cmd == NULL) {
				fprintf(stderr,
				    "command: %s invalid (allocation failed)",
				    optarg);
				return 3;
			}
			memcpy(cmd, optarg, s);
			break;
		default:
			usage();
		}
	}	
		
	

	return 0;
}

static void
usage(void) 
{
	fprintf(stderr, "usage: %s %s\n", getprogname(),
	        "[-txn:c:] [command ...]");
	exit(1);
}

static void
clear_screen(void)
{
	printf("\033c");
}

static long
get_mscs(double secs)
{
	secs *= 1000;
	long mscs = (long)secs;
	return mscs;
}

