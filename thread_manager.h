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

#ifndef _thread_manager_h
#define _thread_manager_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include "shared_data.h"
#include "receive_rc.h"

/**************************************************************
 *	Create and exec threads.
 *
 *	@param data		Standard io to write the message
 *	@param thread_id	Thread array
 *
 *	@return int		<0 if an error occurred
 **************************************************************/
int exec_thread(shared_data_t *data, pthread_t *threads_id);

/**************************************************************
 *	Wait the terminaison of all thread, free the memory and
 *	finish the programm.
 *
 *	@param data		Shared data between all thread
 *	@param thread_id	Thread array
 *
 *	@return void
 **************************************************************/
void piboat_wait(shared_data_t *d, pthread_t *thread_id);
#endif
