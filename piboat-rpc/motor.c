/*************************************************************************
 *	Copyright (C) 2014-2018  TERNISIEN d'OUVILLE Matthieu
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

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "pwm.h"
#include "shared_data.h"
#include "receive_rc.h"

static const int SPEED_LOW = 1500;	/* Low speed (pwm value) */
static const int SPEED_HIGH = 4095;	/* High speed (pwm value) */
static const int SPEED_LIM = 1<<12|0;	/* For full on or full off(pwm value) */

struct motor {
	int in_1;		/* Input 1 */
	int in_2;		/* Input 2 */
	int enable_pwm;		/* PWM pins */
	int cur_speed;		/* Current speed */
	int adjust;		/* Adjustement percent add to speed */
};

static struct motor motor_1 = {0, 3, 8, 0, 0};
static struct motor motor_2 = {4, 5, 9, 0, 0};

/* TODO test système de reglage... */

/**************************************************************
 *	Modification de la vitesse d'un moteur
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param *m			Pointeur sur les donnees du moteur
 *	@param speed_m2		Nouvelle vitesse du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int motor_speed(shared_data_t *data, struct motor *m, int speed)
{
	int pwm_value; /* , on, off; */

	/* Low speed */
	if (speed == 0) {
		set_pwm(data, m->enable_pwm, 0, SPEED_LIM);
		return 0;
	}
	/* High speed */
	if (speed == 100 || speed == -100) {
		set_pwm(data, m->enable_pwm, SPEED_LIM, 0);
		return 0;
	}

	/* Other speed */
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/100.0);
	pwm_value += SPEED_LOW;

	syslog(LOG_DEBUG, "Speed value: %i\n", pwm_value);

	set_pwm(data, m->enable_pwm, 0, pwm_value);

	m->cur_speed = speed;

	return 0;
}

/**************************************************************
 *	Modification du sens d'un moteur
 *
 *	@param m		Donnees du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int motor_switch_direction(struct motor m)
{
	if (m.cur_speed > 0){
		digitalWrite(m.in_1, LOW);
		digitalWrite(m.in_2, HIGH);
	}
	else {
		digitalWrite(m.in_1, HIGH);
		digitalWrite(m.in_2, LOW);
	}

	return 0;
}

static int set_motor_speed(shared_data_t *data, int speed_m1, int speed_m2)
{
	/* The adjustment is used if the speed is the same for both motors. */
	if (speed_m1 == speed_m2){
		if (speed_m1 < 0)
			speed_m1 += motor_1.adjust;
		else
			speed_m1 -= motor_1.adjust;

		if (speed_m2 < 0)
			speed_m2 += motor_2.adjust;
		else
			speed_m2 -= motor_2.adjust;
	}

	syslog(LOG_DEBUG, "Real speed: %i %i\n", speed_m1, speed_m2);

	/* Switch direction */
	if ((motor_1.cur_speed < 0 && speed_m1 > 0) ||
	    (motor_1.cur_speed > 0 && speed_m1 < 0))
		motor_switch_direction(motor_1);
	if ((motor_2.cur_speed < 0 && speed_m2 > 0) ||
	    (motor_2.cur_speed > 0 && speed_m2 < 0))
		motor_switch_direction(motor_2);

	/* Update speed */
	motor_speed(data, &motor_1, speed_m1);
	motor_speed(data, &motor_2, speed_m2);

	return 0;
}

static int init_motor(shared_data_t *data)
{
	pinMode(motor_1.in_1, OUTPUT);
	pinMode(motor_1.in_2, OUTPUT);
	pinMode(motor_2.in_1, OUTPUT);
	pinMode(motor_2.in_2, OUTPUT);

	set_motor_speed(data, 0, 0);
	motor_switch_direction(motor_1);
	motor_switch_direction(motor_2);

	return 0;
}

static void deinit_motor(shared_data_t *data)
{
	set_motor_speed(data, 0, 0);
}

static int set_motor_adjust_arg(int argc, char *argv[], shared_data_t *data)
{
	long int motor, adjust;
	struct motor *ma, *mb;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Motor adjust RPC: too few arguments *ma 1|2 <-100-100>*\n");
		return -1;
	}

	motor = strtol(argv[1], &end, 10);
	if (motor < 1 || motor > 2 || *end != '\0') {
		syslog(LOG_ERR, "Motor adjust RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	adjust = strtol(argv[2], &end, 10);
	if (adjust < -30 || adjust > 30 || *end != '\0') {
		syslog(LOG_ERR, "Motor adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	switch (motor){
		case 1 :
			ma = &motor_1;
			mb = &motor_2;
		break;
		case 2 :
			ma = &motor_2;
			mb = &motor_1;
		break;
		default : return -1;
	}

	/* On enregistre la valeur optimal pour chaque moteur. */
	/* On a donc au moins un moteur toujours à 0. */
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

static int set_motor_speed_arg(int argc, char *argv[], shared_data_t *data)
{
	long int speed_m1, speed_m2;
	char *end;

	if (argc != 3) {
		syslog(LOG_ERR, "Motor speed RPC: too few arguments *ms <-100-100> <-100-100>*\n");
		return -1;
	}

	speed_m1 = strtol(argv[1], &end, 10);
	if (speed_m1 < -100 || speed_m1 > 100 || *end != '\0') {
		syslog(LOG_ERR, "Motor speed RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	speed_m2 = strtol(argv[2], &end, 10);
	if (speed_m1 < -100 || speed_m1 > 100 || *end != '\0') {
		syslog(LOG_ERR, "Motor speed RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	return set_motor_speed(data, (int)speed_m1, (int)speed_m2);
}

static piboat_rpc_t motor_speed_rpc = {
	.cmd_name = "ms",
	.init = init_motor,
	.cmd_set = set_motor_speed_arg,
	.deinit = deinit_motor,
};

static piboat_rpc_t motor_adjust_rpc = {
	.cmd_name = "ma",
	.init = NULL,
	.cmd_set = set_motor_adjust_arg,
	.deinit = NULL,
};

static void init_piboat_motors(void) __attribute__((constructor));
void init_piboat_motors(void)
{
	register_piboat_rpc(&motor_speed_rpc);
	register_piboat_rpc(&motor_adjust_rpc);
}
