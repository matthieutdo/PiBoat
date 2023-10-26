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

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <syslog.h>

#include "thruster.h"
#include "pwm.h"

/**************************************************************
 *	Modification de la vitesse d'un moteur
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param *t		Pointeur sur les donnees du moteur
 *	@param speed		Nouvelle vitesse du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int thruster_speed(thruster_t *t, shared_data_t *data, int speed)
{
	int pwm_value; /* , on, off; */

	/* Low speed */
	if (speed == 0) {
		set_pwm(data, t->pwm_channel, 0, SPEED_LIM);
		goto end;
	}
	/* High speed */
	if (speed == 1000 || speed == -1000) {
		set_pwm(data, t->pwm_channel, SPEED_LIM, 0);
		goto end;
	}

	/* Other speed */
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/1000.0);
	pwm_value += SPEED_LOW;

	syslog(LOG_DEBUG, "Speed value: %i\n", pwm_value);

	set_pwm(data, t->pwm_channel, 0, pwm_value);

end:
	t->cur_speed = speed;

	return 0;
}

/**************************************************************
 *	Modification du sens d'un moteur
 *
 *	@param t		Donnees du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static void thruster_switch_direction(thruster_t *t)
{
	if (t->cur_speed > 0) {
		digitalWrite(t->gpio_enable, LOW);
		digitalWrite(t->gpio_dir, HIGH);
	} else {
		digitalWrite(t->gpio_enable, HIGH);
		digitalWrite(t->gpio_dir, LOW);
	}
}

void set_thruster_speed(thruster_t *t, shared_data_t *data, int speed)
{
	if (speed < 0)
		speed += t->adjust;
	else
		speed -= t->adjust;

	syslog(LOG_DEBUG, "Real speed: %i\n", speed);

	/* Switch direction */
	if ((t->cur_speed < 0 && speed > 0) || (t->cur_speed > 0 && speed < 0))
		thruster_switch_direction(t);

	/* Update speed */
	thruster_speed(t, data, speed);
}

void init_thruster(thruster_t *t, shared_data_t *data)
{
	pinMode(t->gpio_enable, OUTPUT);
	pinMode(t->gpio_dir, OUTPUT);

	digitalWrite(t->gpio_enable, LOW);
	digitalWrite(t->gpio_dir, HIGH);

	set_thruster_speed(t, data, 0);
	thruster_switch_direction(t);
}

void deinit_thruster(thruster_t *t, shared_data_t *data)
{
	set_thruster_speed(t, data, 0);
	digitalWrite(t->gpio_enable, HIGH);
	digitalWrite(t->gpio_dir, LOW);
}
