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

#ifndef _motor_h
#define _motor_h

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> /* valeur absolue */

#include "pwm.h"
#include "shared_data.h"
#include "DEBUG.h"

#define MOTOR_UNKNOWN	-1
#define MOTOR_VAL_ERROR	-2

/**************************************************************
 *	Initialisation des moteurs
 *
 *	@param pwm_handle	Gestionnaire PWM
 *
 *	@return int		<0 si erreur
 **************************************************************/
int init_motor(shared_data_t *data);

/**************************************************************
 *	Deinitialisation des moteurs
 *
 *	@param pwm_handle	Gestionnaire PWM
 *
 *	@return int		<0 si erreur
 **************************************************************/
int deinit_motor(shared_data_t *data);

/**************************************************************
 *	Gestion des moteurs (changer les réglages)
 *	Les réglages permetent de modifier la vitesse d'un moteur par
 *	rapport à l'autre. A utiliser lorsque le bateau ne se deplace
 *	pas en ligne droite.
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param motor		Identitfiant du moteur
 *	@param adjust		Nouveau reglage
 *
 *	@return int		MOTOR_UNKNOWN : motor id inconnu
 *				MOTOR_VAL_ERROR : adjust incorect
 *				0 si tout c'est bien passé
 **************************************************************/
int set_motor_adjust(shared_data_t *data, int motor, int adjust);

/**************************************************************
 *	Gestion des moteurs (connaitre les réglages)
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param motor		Identitfiant du moteur
 *
 *	@return adjust		valeur du reglage
 *	@return int		<0 si erreur
 **************************************************************/
int get_motor_adjust(shared_data_t *data, int motor, int *adjust);

/**************************************************************
 *	Gestion des moteurs (modifier la vitesse des moteur)
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param speed_m1		Nouvelle vitesse du moteur 1
 *	@param speed_m2		Nouvelle vitesse du moteur 2
 *
 *	@return int		<0 si erreur
 **************************************************************/
int set_motor_speed(shared_data_t *data, int speed_m1, int speed_m2);

#endif
