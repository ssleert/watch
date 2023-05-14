CC	?= cc

CFLAGS	?= -O2
CFLAGS	+= -Wall -Wextra -std=c99 -pedantic
#CFLAGS	+= -Wall -Wextra -Werror -std=c99 -pedantic
#LDFLAGS	+= -ltermcap


PREFIX	?= /usr/local
BINDIR	?= $(PREFIX)/bin
MANDIR	?= $(PREFIX)/man/man1
INSTALL	?= install -s

PROG	= watch
MAN	= $(PROG).1
CFILES	!= ls *.c 
OBJS	= ${CFILES:.c=.o}

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

install: all
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)
	$(INSTALL) $(PROG) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 $(MAN) $(DESTDIR)$(MANDIR)/$(MAN)

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all install clean

