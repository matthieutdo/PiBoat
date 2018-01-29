SER=piboat
all:$(SER)

PIBOAT_VERSION := $(shell git describe)

DEBUG.o:DEBUG.c
	gcc -DDEBUG_MODE -Wall -c DEBUG.c

thread_manager.o:thread_manager.c
	gcc -Wall -c thread_manager.c

receive_rc.o:receive_rc.c
	gcc -Wall -c receive_rc.c
	
motor.o:motor.c
	gcc -Wall -c motor.c
	
pwm.o:pwm.c
	gcc -Wall -c pwm.c
	
direction.o:direction.c
	gcc -Wall -c direction.c
	
connect_tcp.o:connect_tcp.c
	gcc -Wall -c connect_tcp.c

main.o:main.c
	gcc -DVERSION=\"$(PIBOAT_VERSION)\" -Wall -c main.c
	
$(SER): DEBUG.o thread_manager.o receive_rc.o direction.o pwm.o motor.o connect_tcp.o main.o
	gcc -Wall DEBUG.o thread_manager.o receive_rc.o direction.o pwm.o motor.o connect_tcp.o main.o -o $(SER) -lwiringPi -lm -lpthread


clean:
	rm -rf *.o *.stackdump
