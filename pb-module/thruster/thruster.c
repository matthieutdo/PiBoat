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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "pwm.h"
#include "shared_data.h"
#include "rpc.h"
#include "thread_manager.h"

static pthread_mutex_t rpc_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rpc_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t rpc_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct rpc_cmd_list rpc_cmd_list = TAILQ_HEAD_INITIALIZER(rpc_cmd_list);

static const int SPEED_LOW = 0;
static const int SPEED_HIGH = 4095;
static const int SPEED_LIM = 1<<12|0;	/* For full on or full off(pwm value) */

struct thruster {
	int gpio_enable;
	int gpio_dir;
	int pwm_channel;
	int cur_speed;
	int adjust;
};

static struct thruster thruster = {22, 24, 8, 0, 0};

/* TODO test système de reglage... */

/**************************************************************
 *	Modification de la vitesse d'un moteur
 *
 *	@param pwm_handle	Gestionnaire PWM
 *	@param *m		Pointeur sur les donnees du moteur
 *	@param speed		Nouvelle vitesse du moteur
 *
 *	@return int		<0 si erreur
 **************************************************************/
static int thruster_speed(shared_data_t *data, struct thruster *m, int speed)
{
	int pwm_value; /* , on, off; */

	/* Low speed */
	if (speed == 0) {
		set_pwm(data, m->pwm_channel, 0, SPEED_LIM);
		return 0;
	}
	/* High speed */
	if (speed == 1000 || speed == -1000) {
		set_pwm(data, m->pwm_channel, SPEED_LIM, 0);
		return 0;
	}

	/* Other speed */
	pwm_value = (float)((SPEED_HIGH) - (SPEED_LOW)) * ((float)fabs(speed)/1000.0);
	pwm_value += SPEED_LOW;

	syslog(LOG_DEBUG, "Speed value: %i\n", pwm_value);

	set_pwm(data, m->pwm_channel, 0, pwm_value);

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
static int thruster_switch_direction(struct thruster m)
{
	if (m.cur_speed > 0) {
		digitalWrite(m.gpio_enable, LOW);
		digitalWrite(m.gpio_dir, HIGH);
	} else {
		digitalWrite(m.gpio_enable, HIGH);
		digitalWrite(m.gpio_dir, LOW);
	}

	return 0;
}

static int set_thruster_speed(shared_data_t *data, int speed)
{
	if (speed < 0)
		speed += thruster.adjust;
	else
		speed -= thruster.adjust;

	syslog(LOG_DEBUG, "Real speed: %i\n", speed);

	/* Switch direction */
	if ((thruster.cur_speed < 0 && speed > 0) ||
	    (thruster.cur_speed > 0 && speed < 0))
		thruster_switch_direction(thruster);

	/* Update speed */
	thruster_speed(data, &thruster, speed);

	return 0;
}

#define SET_THRUSTER_ADJ_CMD "ms"
static int set_thruster_adjust_arg(int argc,
			char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN],
			shared_data_t *data)
{
	long int adjust;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster adjust RPC: too few arguments *ma <-100-100>*\n");
		return -1;
	}

	adjust = strtol(argv[2], &end, 10);
	if (adjust < -30 || adjust > 30 || *end != '\0') {
		syslog(LOG_ERR, "Thruster adjust RPC: invalid argument 2 %s",
		       argv[2]);
		return -1;
	}

	thruster.adjust = adjust;

	return 0;
}

#define SET_THRUSTER_SPEED_CMD "ms"
static int set_thruster_speed_arg(int argc,
			char argv[PIBOAT_CMD_MAXARG + 1][PIBOAT_CMD_MAXLEN],
			shared_data_t *data)
{
	long int speed;
	char *end;

	if (argc != 2) {
		syslog(LOG_ERR, "Thruster speed RPC: too few arguments *ms <-1000-1000>*\n");
		return -1;
	}

	speed = strtol(argv[1], &end, 10);
	if (speed < -1000 || speed > 1000 || *end != '\0') {
		syslog(LOG_ERR, "Thruster speed RPC: invalid argument 1 %s",
		       argv[1]);
		return -1;
	}

	return set_thruster_speed(data, (int)speed);
}

static rpc_t thruster_speed_rpc = {
	.cmd_name = SET_THRUSTER_SPEED_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static rpc_t thruster_adjust_rpc = {
	.cmd_name = SET_THRUSTER_ADJ_CMD,
	.cmd_list = &rpc_cmd_list,
	.wait_cond = &rpc_wait_cond,
	.queue_mutex = &rpc_queue_mutex,
};

static void init_thruster_rpc(void) __attribute__((constructor));
void init_thruster_rpc(void)
{
	register_rpc(&thruster_speed_rpc);
	register_rpc(&thruster_adjust_rpc);
}

static void* thruster_loop(void *p)
{
	struct rpc_cmd_entry *rpc_cmd_e;
	shared_data_t *data;
	int ret;

	data = (shared_data_t *)p;

	pinMode(thruster.gpio_enable, OUTPUT);
	pinMode(thruster.gpio_dir, OUTPUT);

	digitalWrite(thruster.gpio_enable, LOW);
	digitalWrite(thruster.gpio_dir, HIGH);

	set_thruster_speed(data, 0);
	thruster_switch_direction(thruster);

	while (true) {
		rpc_cmd_e = read_rpc(&rpc_cmd_list, &rpc_queue_mutex,
				     &rpc_wait_mutex, &rpc_wait_cond);
		if (rpc_cmd_e == NULL)
			continue;

		if (strcmp(rpc_cmd_e->cmd.argv[0],
			   SET_THRUSTER_SPEED_CMD) == 0) {
			ret = set_thruster_speed_arg(rpc_cmd_e->cmd.argc,
						     rpc_cmd_e->cmd.argv,
						     data);
		} else if (strcmp(rpc_cmd_e->cmd.argv[0],
				SET_THRUSTER_ADJ_CMD) == 0) {
			ret = set_thruster_adjust_arg(rpc_cmd_e->cmd.argc,
						      rpc_cmd_e->cmd.argv,
						      data);
		}

		if (ret != 0)
			syslog(LOG_ERR, "Error occured during %s rpc exec",
			       rpc_cmd_e->cmd.argv[0]);

		free(rpc_cmd_e);
	}

	set_thruster_speed(data, 0);
	digitalWrite(thruster.gpio_enable, HIGH);
	digitalWrite(thruster.gpio_dir, LOW);

	return NULL;
}

static module_t thruster_module = {
	.name = "thruster",
	.loop = thruster_loop,
};

static void init_thruster_module(void) __attribute__((constructor));
void init_thruster_module(void)
{
	register_module(&thruster_module);
}
