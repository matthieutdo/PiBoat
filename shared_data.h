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

#ifndef _shared_data_h
#define _shared_data_h

#include <pthread.h>

typedef int pwm_t;

/*  Struct contain all parametrable values */
struct param_t{
	char ai_on;	/* The AI can manage the boat (ai can be active -shared_data_t.ai_active) */
	// ...
};

typedef struct shd_t{
	char ai_active;			/* The AI manage the boat (mortors and rudders) */
	pwm_t pwm;
	pthread_mutex_t pwm_mutex;	/* = PTHREAD_MUTEX_INITIALIZER */
	struct param_t param;
}shared_data_t;

#endif
