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

#include <syslog.h>

#include "pwm.h"

static const int PWM_ADR = 0x40;

/**************************************************************
 *	Modification de la frequence
 *
 *	@param fd	Gestionnaire PWM
 *	@param freq	Nouvelle Frequence
 **************************************************************/
static void set_freq(pwm_t fd, int freq)
{
	double prescaleval, prescale;
	int oldmode, newmode;

	prescaleval = 25000000.0 / 4096.0; /* 25MHz/12-bit */
	prescaleval /= (float)freq;
	prescaleval -= 1.0;

	syslog(LOG_DEBUG, "Setting PWM frequency to %i Hz\n", freq);
	syslog(LOG_DEBUG, "Estimated pre-scale: %f\n", prescaleval);

	prescale = floor(prescaleval + 0.5);

	syslog(LOG_DEBUG, "Final pre-scale: %f\n", prescale);

	oldmode = wiringPiI2CReadReg8((int)fd, MODE1);
	newmode = (oldmode & 0x7F) | 0x10;		/* Sleep */
	wiringPiI2CWriteReg8((int)fd, MODE1, newmode);	/* Go to sleep */
	wiringPiI2CWriteReg8((int)fd, PRESCALE, (int)floor(prescale));
	wiringPiI2CWriteReg8((int)fd, MODE1, oldmode);
	delay(5);
	wiringPiI2CWriteReg8((int)fd, MODE1, oldmode|0x80);	/* Restart */
}

pwm_t init_pwm()
{
	int err=0;
	pwm_t fd;

	/* Init I2C bus */
	fd = (pwm_t) wiringPiI2CSetup(PWM_ADR);
	if (fd == -1){
		syslog(LOG_EMERG, "I2C setup error: %i\n", errno);
		return -1;
	}

	err = wiringPiI2CWriteReg8((int)fd, MODE2, 0x00);
	if (err < 0){
		syslog(LOG_EMERG, "I2C initalisation error: %i\n", err);
		return -2;
	}

	err = 0;
	err = wiringPiI2CWriteReg8((int)fd, MODE1, 0x00);
	if (err < 0){
		syslog(LOG_EMERG, "I2C initalisation error: %i\n", err);
		return -3;
	}

	set_freq(fd, 60);

	return fd;
}

void set_pwm(shared_data_t *data, int channel, int on, int off)
{
	pthread_mutex_lock(&(data->pwm_mutex));

	wiringPiI2CWriteReg8((int)data->pwm, LED0_ON_L+4*channel, on&0xFF);
	wiringPiI2CWriteReg8((int)data->pwm, LED0_ON_H+4*channel, on>>8);
	wiringPiI2CWriteReg8((int)data->pwm, LED0_OFF_L+4*channel, off&0xFF);
	wiringPiI2CWriteReg8((int)data->pwm, LED0_OFF_H+4*channel, off>>8);

	pthread_mutex_unlock(&(data->pwm_mutex));
}

void get_pwm(shared_data_t *data, int channel, int *on, int *off)
{
	pthread_mutex_lock(&(data->pwm_mutex));

	*on = wiringPiI2CReadReg8((int)data->pwm, LED0_ON_H+4*channel);
	*on = *on<<8 | wiringPiI2CReadReg8((int)data->pwm, LED0_ON_L+4*channel);
	*off = wiringPiI2CReadReg8((int)data->pwm, LED0_OFF_H+4*channel);
	*off = *off<<8 | wiringPiI2CReadReg8((int)data->pwm, LED0_OFF_L+4*channel);

	pthread_mutex_unlock(&(data->pwm_mutex));
}
