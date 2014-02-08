SER=piboat
all:$(SER)

	
motor.o:motor.c
	gcc -Wall -c motor.c
	
pwm.o:pwm.c
	gcc -Wall -c pwm.c
	
direction.o:direction.c
	gcc -Wall -c direction.c
	
connect_tcp.o:connect_tcp.c
	gcc -Wall -c connect_tcp.c

main.o:main.c
	gcc -Wall -c main.c
	
$(SER): direction.o pwm.o motor.o connect_tcp.o main.o
	gcc -Wall direction.o pwm.o motor.o connect_tcp.o main.o -o $(SER) -lwiringPi -lm


clean:
	rm -rf *.o *.stackdump
