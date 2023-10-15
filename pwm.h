/*************************************************************************
 *	Copyright (C) 2014-2024  TERNISIEN d'OUVILLE Matthieu
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

#ifndef _pwm_h
#define _pwm_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>	/* floor */
#include <pthread.h>	/* mutex */

#include <wiringPi.h>	/* delay */
#include <wiringPiI2C.h>

#include "shared_data.h"

#define MODE1           0x00
#define MODE2           0x01

#define PRESCALE	0xFE

#define LED0_ON_L       0x06
#define LED0_ON_H       0x07
#define LED0_OFF_L      0x08
#define LED0_OFF_H      0x09

/**************************************************************
 *	Initialisation du gestionnaire PWM
 *
 *	@return int	Gestionnaire PWM
 **************************************************************/
pwm_t init_pwm();

/**************************************************************
 *	Modification d'un canal PWM
 *
 *	@param fd	Gestionnaire PWM
 *	@param channel	Canal
 *	@param on	Nouvelle valeur du registre ON
 *	@param off	Nouvelle valeur du registre OFF
 **************************************************************/
void set_pwm(shared_data_t *data, int channel, int on, int off);

/**************************************************************
 *	Modification d'un canal PWM
 *
 *	@param fd	Gestionnaire PWM
 *	@param channel	Canal
 *
 *	@return on	Valeur du registre ON
 *	@return off	Valeur du registre OFF
 **************************************************************/
void get_pwm(shared_data_t *data, int channel, int *on, int *off);

#endif
