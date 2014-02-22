/*************************************************************************
 *	PiBoat project: this programm is used for control a RC boat with a 
 *	Raspberry Pi.
 *
 *	Note:	this programm use the WiringPi library (LGPLv3)
 *			see https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *	Version 1.01
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
 *
 *	For debug mode add -DDEBUG_MODE for the compilation
 ************************************************************************/


//	Revisions:
//	16-02-2014:
//			- create receive_rc file for manage connection between rc app
//			  (recv and execute control command);
//			- update all file for use shared_data_t struct;
//			- create files debug.c/h:
//				- printf for debug replaced by print_debug.
//	18-02-2014
//			- files thread_manager.c/h -> created and ended threads;
//				- recv SIGINT -> ended all threads;
//			- mutex for pwm access.
//	22-02-2014
//			- Bug fixed:
//				- print_debug displaying;
//				- connection lost;
//
//	TODO:
//			- CAM threads:
//				- camera control;
//				- camera turret;
//			- AI threads:
//				- Dist sensors control;
//			- ...
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <wiringPi.h>

#include "shared_data.h"
#include "pwm.h"
#include "thread_manager.h"



void init_data(shared_data_t *d){
	d->ai_active = false;
	d->pwm = -1;
	pthread_mutex_init(&(d->pwm_mutex), NULL);
	d->param.ai_on = true;
}


int main(int argc, char* argv[]){
	int err;
	pthread_t threads_id[3];
	shared_data_t data;
	
	init_data(&data);
	
	// GPLv3 licence
	printf("PiBoat  Copyright (C) 2014  TERNISIEN d'OUVILLE Matthieu \n");
	printf("This program comes with ABSOLUTELY NO WARRANTY. \n");
	printf("This is free software, and you are welcome to redistribute it under certain \n");
	printf("conditions; see https://www.gnu.org/copyleft/gpl.html for details.\n");
	
	
	// Initialisation PWM board
	data.pwm = init_pwm();
	
	// initialisation GPIO
	err = wiringPiSetup();
	if (err == -1){
		fprintf(stderr, "GPIO setup error: %i\n", errno);
		return -1;
	}
	
	// Initialisation motor and direction
	init_motor(&data);
	init_direction(&data);
	
	
	// Create and execute thread
	err = exec_thread(&data, threads_id);
	if (err != 0) {
		fprintf(stderr, "ERROR: threads not created...\n");
	}
	else{
		printf("Thread initialisation\t\t\t[OK]\n");
		// Wait thread termination
		loop(&data, threads_id);
	}
	
	// Ended system
	deinit_motor(&data);
	deinit_direction(&data);
	
	return 0;
}
