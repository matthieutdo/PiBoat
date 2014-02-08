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


#include "motor.h"


static const int SPEED_LOW = 1500;	// Low speed (pwm value)
static const int SPEED_HIGH = 4095;	// High speed (pwm value)
static const int SPEED_LIM = 1<<12|0;	// For full on or full off(pwm value)

struct motor{
	int in_1;		// Input 1
	int in_2;		// Input 2
	int enable_pwm;		// PWM pins
	int cur_speed;		// Current speed
	int adjust;		// Adjustement percent add to speed
};

static struct motor motor_1 = {0, 3, 8, 0, 0};
static struct motor motor_2 = {4, 5, 9, 0, 0};

// TODO adjust ne marche pas correctement...

/**************************************************************
 *	Modification de la vitesse d'un moteur
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param *m		Pointeur sur les donnees du
 *				moteur
 *	@param speed_m2		Nouvelle vitesse du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int motor_speed(int pwm_handle, struct motor *m, int speed);


/**************************************************************
 *	Modification du sens d'un moteur
 *
 *	@param m		Donnees du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int motor_switch_direction(struct motor m);



int init_motor(int pwm_handle){
	pinMode(motor_1.in_1, OUTPUT);
	pinMode(motor_1.in_2, OUTPUT);
	pinMode(motor_2.in_1, OUTPUT);
	pinMode(motor_2.in_2, OUTPUT);
	
	set_motor_speed(pwm_handle, 0, 0);
	motor_switch_direction(motor_1);
	motor_switch_direction(motor_2);
	
	return 0;
}


int deinit_motor(int pwm_handle){
	set_motor_speed(pwm_handle, 0, 0);
	
	return 0;
}


int set_motor_adjust(int pwm_handle, int motor, int adjust){
	struct motor *ma, *mb;
	
	if (fabs(adjust) > 30) return MOTOR_VAL_ERROR;
	
	switch (motor){
		case 1 : 
			ma = &motor_1;
			mb = &motor_2;
		break;
		case 2 : 
			ma = &motor_2;
			mb = &motor_1;
		break;
		default : return MOTOR_UNKNOWN;
	}
	
	// On enregistre la valeur optimal pour chaque moteur.
	// On a donc au moins un moteur toujours à 0.
	if (fabs(mb->adjust) >= fabs(adjust)){
		ma->adjust = 0;
		mb->adjust -= adjust;
	}
	else{
		ma->adjust = adjust - mb->adjust;
		mb->adjust = 0;
	}
	
	return 0;
}


int get_motor_adjust(int pwm_handle, int motor, int *adjust){
	struct motor *m;
	
	if (adjust == NULL) return MOTOR_VAL_ERROR;
	
	switch (motor){
		case 1 : m = &motor_1;
		break;
		case 2 : m = &motor_2;
		break;
		default : return MOTOR_UNKNOWN;
	}
	
	*adjust = m->adjust;
	
	return 0;
}


int set_motor_speed(int pwm_handle, int speed_m1, int speed_m2){
	int diff;
	
	diff = (motor_1.adjust != 0) ? motor_1.adjust : motor_2.adjust;
	
	if (diff != 0){
		speed_m1 = (speed_m1 == 0) ? 0 :speed_m1 + motor_1.adjust;
		speed_m2 = (speed_m2 == 0) ? 0 :speed_m2 + motor_2.adjust;
		
		// Les deux moteurs tournent en sens inverse l'un de l'autre
		if ((speed_m1>0 && speed_m2<0) || (speed_m1<0 && speed_m2>0)){
			speed_m1 = (speed_m1 > 100) ? 100 : ((speed_m1 < -100) ? -100 : speed_m1);
			speed_m2 = (speed_m2 > 100) ? 100 : ((speed_m2 < -100) ? -100 : speed_m2);
		}
		else{
			// les deux moteurs tournent dans le meme sens
			if (fabs(speed_m1) > 100){
				diff = (speed_m1 > 100) ? speed_m1-100 : speed_m1+100;
				speed_m2 = (speed_m2 > 0) ? speed_m2-diff : speed_m2+diff;
				speed_m1 = (speed_m1 > 100) ? 100 : -100;
			}
			else if (fabs(speed_m2) > 100){
				diff = (speed_m2 > 100) ? speed_m2-100 : speed_m2+100;
				speed_m1 = (speed_m1 > 0) ? speed_m1-diff : speed_m1+diff;
				speed_m2 = (speed_m2 > 100) ? 100 : -100;
			}
		}
	}
	
	printf("Real speed: %i %i\n", speed_m1, speed_m2);
	
	
	// Switch direction
	if ((motor_1.cur_speed < 0 && speed_m1 > 0) || (motor_1.cur_speed > 0 && speed_m1 < 0))
		motor_switch_direction(motor_1);
	if ((motor_2.cur_speed < 0 && speed_m2 > 0) || (motor_2.cur_speed > 0 && speed_m2 < 0))
		motor_switch_direction(motor_2);
	
	
	// Update speed
	motor_speed(pwm_handle, &motor_1, speed_m1);
	motor_speed(pwm_handle, &motor_2, speed_m2);
	
	return 0;
}


int motor_speed(int pwm_handle, struct motor *m, int speed){
	int pwm_value; //, on, off;
	
	// Low speed
	if (speed == 0) {
		set_pwm(pwm_handle, m->enable_pwm, 0, SPEED_LIM);
		return 0;
	}
	// High speed
	if (speed == 100 || speed == -100) {
		set_pwm(pwm_handle, m->enable_pwm, SPEED_LIM, 0);
		return 0;
	}
	
	// Other speed
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/100.0);
	pwm_value += SPEED_LOW;
	  
	// DEBUG
	//printf("Speed value: %i\n", pwm_value);
	
	set_pwm(pwm_handle, m->enable_pwm, 0, pwm_value);
	
	m->cur_speed = speed;
	
	return 0;
}

int motor_switch_direction(struct motor m){
	if (m.cur_speed > 0){
		digitalWrite (m.in_1, LOW);
		digitalWrite (m.in_2, HIGH);
	}
	else {
		digitalWrite (m.in_1, HIGH);
		digitalWrite (m.in_2, LOW);
	}
	
	return 0;
}


