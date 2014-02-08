/*************************************************************************
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
 ************************************************************************/


#include "pwm.h"


static const int PWM_ADR = 0x40;



/**************************************************************
 *	Modification de la frequence
 *
 *	@param fd	Gestionnaire PWM
 *	@param freq	Nouvelle Frequence
 **************************************************************/
void set_freq(int fd, int freq);


int init_pwm(){
	int fd, err=0;
	
	// Init I2C bus
	fd = wiringPiI2CSetup(PWM_ADR);
	if (fd == -1){
		printf("I2C setup error: %i\n", errno);
		return -1;
	}
	
	
	err = wiringPiI2CWriteReg8(fd, MODE2, 0x00);
	if (err < 0){
		printf("I2C initalisation error: %i\n", err);
		return -2;
	}
	
	err = 0;
	err = wiringPiI2CWriteReg8(fd, MODE1, 0x00);
	if (err < 0){
		printf("I2C initalisation error: %i\n", err);
		return -3;
	}
	
	set_freq(fd, 60);
	
	return fd;
}


void set_pwm(int fd, int channel, int on, int off){
	wiringPiI2CWriteReg8(fd, LED0_ON_L+4*channel, on&0xFF);
	wiringPiI2CWriteReg8(fd, LED0_ON_H+4*channel, on>>8);
	wiringPiI2CWriteReg8(fd, LED0_OFF_L+4*channel, off&0xFF);
	wiringPiI2CWriteReg8(fd, LED0_OFF_H+4*channel, off>>8);
}


void get_pwm(int fd, int channel, int *on, int *off){
	*on = wiringPiI2CReadReg8(fd, LED0_ON_H+4*channel);
	*on = *on<<8 | wiringPiI2CReadReg8(fd, LED0_ON_L+4*channel);
	*off = wiringPiI2CReadReg8(fd, LED0_OFF_H+4*channel);
	*off = *off<<8 | wiringPiI2CReadReg8(fd, LED0_OFF_L+4*channel);
}


void set_freq(int fd, int freq){
	double prescaleval, prescale;
	int oldmode, newmode;
	
	prescaleval = 25000000.0 / 4096.0; // 25MHz/12-bit
	prescaleval /= (float)freq;
	prescaleval -= 1.0;
	
	printf("Setting PWM frequency to %i Hz\n", freq);
	printf("Estimated pre-scale: %f\n", prescaleval);
	
	prescale = floor(prescaleval + 0.5);
	
	printf("Final pre-scale: %f\n", prescale);
	
	oldmode = wiringPiI2CReadReg8(fd, MODE1);
	newmode = (oldmode & 0x7F) | 0x10;		// Sleep
	wiringPiI2CWriteReg8(fd, MODE1, newmode);	// Go to sleep
	wiringPiI2CWriteReg8(fd, PRESCALE, (int)floor(prescale));
	wiringPiI2CWriteReg8(fd, MODE1, oldmode);
	delay(5);
	wiringPiI2CWriteReg8(fd, MODE1, oldmode|0x80);	// Restart
}
