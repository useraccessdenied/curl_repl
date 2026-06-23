CC       = cc
CFLAGS   = -std=c11 -Wall -Wextra -g $(shell curl-config --cflags)
LIBS     = $(shell curl-config --libs)
TARGET   = curl_repl
SRCDIR   = src
BUILDDIR = build
SRCS     = $(SRCDIR)/main.c \
           $(SRCDIR)/repl.c \
           $(SRCDIR)/protocol.c \
           $(SRCDIR)/command.c \
           $(SRCDIR)/ws.c
OBJS     = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

checksrc:
	perl scripts/checksrc.pl $(SRCDIR)/*.c $(SRCDIR)/*.h

clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: all checksrc clean
