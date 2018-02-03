# Copyright 2014-2018  TERNISIEN d'OUVILLE Matthieu
PROG = piboat

P = @ echo
Q = @

all: $(PROG)


PIBOAT_VERSION = $(shell git describe)

CFLAGS = -Wall -Werror
CFLAGS += -DVERSION=\"$(PIBOAT_VERSION)\"

SRCS = main.c
SRCS += connect_tcp.c
SRCS += direction.c
SRCS += pwm.c
SRCS += motor.c
SRCS += receive_rc.c
SRCS += thread_manager.c

LDADD = -lm
LDADD += -lpthread
LDADD += -lwiringPi

OBJS:=$(SRCS:%.c=%.o)

$(PROG): $(OBJS)
	$P '  PROG $(notdir $@)'
	$Q $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDADD)

%.o:%.c
	$P '  CC $(notdir $@)'
	$Q mkdir -p $(@D)
	$Q $(CC) $(CFLAGS) -o $@ -c $<

clean:
	$Q rm -f $(OBJS) $(PROG)

.PHONY: all clean
