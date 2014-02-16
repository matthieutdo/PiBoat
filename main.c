/*************************************************************************
 *	PiBoat project: this programm is used for control a RC boat with a 
 *	Raspberry Pi.
 *
 *	Note:	this programm use the WiringPi library (LGPLv3)
 *			see https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *	Version 0.2
 *
 *	Copyright (C) 2014  TERNISIEN d'OUVILLE Matthieu
 *	
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *	Author: TERNISEN d'OUVILLE Matthieu <matthieu.tdo@gmail.com>
 *	See also piboat.blogspot.fr
 ************************************************************************/


//	Revisions:
//	15-02-2014:
//			- main.c: no need to restart the program after a client disconnection.
//	16-02-2014:
//			- all files: for pwm descriptor -> use pwm_t type instead int.
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>

//#include <signal.h>

#include "shared_data.h"
#include "connect_tcp.h"
#include "pwm.h"
#include "motor.h"
#include "direction.h"


// Changer la vitesse des moteurs
// m <vitesse moteur 1> <vitesse moteur 2>
#define CMD_MOTOR	"m"
// Changer reglage des moteurs
// mri <pourcentage add/sub>
#define CMD_MOTOR_R1	"mr1"	
#define CMD_MOTOR_R2	"mr2"
// Changer position gouvernails
// g <degre entre 40 et 140>
#define CMD_DIRECTION	"g"
// Changer reglage gouvernails
// gr <degre add/sub>
#define CMD_DIR_REG	"gr"


/*void handler_end(){
	// TODO
	exit(-1);
}*/

// retourne le nom de la commande
void value_cmd(char *cmd, char *cmd_val){
	int i;
	
	i = 0;
	while (cmd[i] != ' ' && cmd[i] != '\0'){
		cmd_val[i] = cmd[i];
		
		++i;
	}
	
	cmd_val[i] = '\0';
}


// retourne la valeur entier d'un parametre
int value_param(char *cmd, int pnum){
	int i, j, y, cur_pnum;
	char param[10];
	
	i=0;
	cur_pnum = 0;
	for (y=0 ; y<strlen(cmd) && cur_pnum!=pnum ; ++y){
		while (cmd[i] != ' ')
			++i;
		
		++i;
		++cur_pnum;
	}
	
	j = 0;
	while (cmd[i] != ' ' && i < strlen(cmd)){
		param[j] = cmd[i];
		
		++i;
		++j;
	}
	
	param[j] = '\0';
	
	return atoi(param);
}



int main(int argc, char* argv[]){
	socket_t sock, sock_cli;
	struct sockaddr_in csin = {0};
	char cmd[21], cmd_val[5];
	int nb, err;
	pwm_t pwm_handle;
	
	// GPLv3 licence
	printf("PiBoat  Copyright (C) 2014  TERNISIEN d'OUVILLE Matthieu \n");
	printf("This program comes with ABSOLUTELY NO WARRANTY. \n");
	printf("This is free software, and you are welcome to redistribute it under certain \n");
	printf("conditions; see https://www.gnu.org/copyleft/gpl.html for details.\n");
	
	
	// Initialisation PWM board
	pwm_handle = init_pwm();
	
	// initialisation GPIO
        err = wiringPiSetup();
        if (err == -1){
                printf("GPIO setup error: %i\n", errno);
                return -1;
        }
	
	// Initialisation motor and direction
	init_motor(pwm_handle);
	init_direction(pwm_handle);
	
	
	// Server initialisation
	sock = init_socket_serv(1);
	switch (sock){
		case SOCK_CREATE:
			printf("Server initialisation\t\t[Failed]\n");
			fprintf(stderr, "Server initiation: socket not create\n");
			return -1;
		case SOCK_BIND:
			printf("Server initialisation\t\t[Failed]\n");
			fprintf(stderr, "Server initiation: bind error\n");
			return -1;
		default: printf("Server initialisation\t\t[OK]\n");
	}
	
	// Only one client can use the boat.
	while (true) {
		// Client connection
		nb = sizeof(csin);
		if ((sock_cli = accept(sock, (struct sockaddr*)&csin, (socklen_t *)&nb)) < 0)
			fprintf(stderr, "Accept connection error: %i\n", sock_cli);
		
		printf("connection accepted\n");
		
		// Commands recv
		do{
			nb = recv(sock_cli, cmd, 20, 0);
			cmd[nb] = '\0';
			
			// DEBUG
			printf("msg rcv: %s\n", cmd);
			value_cmd(cmd, cmd_val);
			
			if (strcmp(cmd_val, CMD_MOTOR) == 0){
				// DEBUG
				printf("cmd value: %i %i\n", value_param(cmd, 1), value_param(cmd, 2));
				set_motor_speed(pwm_handle, value_param(cmd, 1), value_param(cmd, 2));
			}
			else if (strcmp(cmd_val, CMD_DIRECTION) == 0){
				// DEBUG
				printf("cmd value: %i\n", value_param(cmd, 1));
				set_direction(pwm_handle, value_param(cmd, 1));
			}
			else if (strcmp(cmd_val, CMD_DIR_REG) == 0){
				// DEBUG
				printf("cmd value: %i\n", value_param(cmd, 1));
				set_dir_adjust(pwm_handle, value_param(cmd, 1));
			}
			else if (strcmp(cmd_val, CMD_MOTOR_R1) == 0){
				// DEBUG
				printf("cmd value: %i\n", value_param(cmd, 1));
				err = set_motor_adjust(pwm_handle, 1, value_param(cmd, 1));
				if (err < 0) printf("Error: set_motor_adjust param incorrect.\n");
			}
			else if (strcmp(cmd_val, CMD_MOTOR_R2) == 0){
				// DEBUG
				printf("cmd value: %i\n", value_param(cmd, 1));
				err = set_motor_adjust(pwm_handle, 2, value_param(cmd, 1));
				if (err < 0) printf("Error: set_motor_adjust param incorrect.\n");
			}
			else if (strcmp(cmd_val, "exit") != 0){
				printf("cmd unknown: %s\n", cmd);
			}
		} while (strcmp(cmd, "exit") != 0);
		
		close_sock(sock_cli);
		
		deinit_motor(pwm_handle);
		deinit_direction(pwm_handle);
	}

	//close_sock(sock);
	
	
	return 0;
}
