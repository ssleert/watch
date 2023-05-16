# BSD Make makefile

PROG	= watch
SRCS	= watch.c
MAN	= 
CFLAGS	+= -Wall -Wextra -std=c99 -pedantic



.include <bsd.prog.mk>
