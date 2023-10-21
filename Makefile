# Copyright 2014-2024  TERNISIEN d'OUVILLE Matthieu

PROG = piboat
START_SCRIPT = piboat.sh
SYSTEMD_SERVICE = piboat.service

P = @ echo
Q = @

all: $(PROG)

bindir = /usr/bin
sysdir = /etc/systemd/system

PIBOAT_VERSION = $(shell git describe)

CFLAGS = -Wall -Werror
CFLAGS += -DVERSION=\"$(PIBOAT_VERSION)\"
CFLAGS += -DRC_SERVER
CFLAGS += -I.
CFLAGS += -Ilib/

SRCS = main.c
SRCS += receive_rc.c
SRCS += thread_manager.c

SRCS += lib/connect_tcp.c
SRCS += lib/pwm.c
SRCS += lib/servo.c
SRCS += lib/rpc.c

SRCS += pb-module/steering/direction.c
SRCS += pb-module/thruster/motor.c

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

install:					\
	$(bindir)/$(PROG)			\
	$(bindir)/$(START_SCRIPT)		\
	$(sysdir)/$(SYSTEMD_SERVICE)

$(bindir)/$(PROG): $(PROG)
	$P '  INSTALL $(notdir $@)'
	$Q install -m 755 -D $< $@

$(bindir)/$(START_SCRIPT): scripts/$(START_SCRIPT)
	$P '  INSTALL $(notdir $@)'
	$Q mkdir -p $(@D)
	$Q install -m 755 -t $(@D) $(<)

$(sysdir)/$(SYSTEMD_SERVICE): scripts/$(SYSTEMD_SERVICE)
	$P '  INSTALL $(notdir $@)'
	$Q mkdir -p $(@D)
	$Q install -m 644 -t $(@D) $(<)

.PHONY: all clean install
