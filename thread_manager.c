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

#include "thread_manager.h"

static pthread_mutex_t finish_mutex;
static pthread_cond_t cond_finish;

/**************************************************************
 *	When a signal is intercepted by the programm this function
 *	is call and the programm is ended.
 *
 *	@param sig		Intercepted signal.
 *
 *	@return void
 **************************************************************/
static void finish_prog(int sig);

void finish_prog(int sig)
{
	pthread_cond_signal(&cond_finish);
	pthread_mutex_unlock(&finish_mutex);

	return ;
}

int exec_thread(shared_data_t *data, pthread_t *threads_id)
{
	int res;
	pthread_attr_t attr[3];

	/* Init mutex for ended programm */
	pthread_mutex_init(&finish_mutex, NULL);
	pthread_cond_init(&cond_finish, NULL);

	/* MAIN thread */
	pthread_attr_init(&attr[0]);
	pthread_attr_setdetachstate(&attr[0], PTHREAD_CREATE_JOINABLE);
	/* CAM thread */
	pthread_attr_init(&attr[1]);
	pthread_attr_setdetachstate(&attr[1], PTHREAD_CREATE_JOINABLE);
	/* AI thread */
	pthread_attr_init(&attr[2]);
	pthread_attr_setdetachstate(&attr[2], PTHREAD_CREATE_JOINABLE);

	/* Intercept signal */
	signal(SIGINT, finish_prog);
	signal(SIGQUIT, finish_prog);
	signal(SIGTERM, finish_prog);

	/* Create MAIN thread here */
	res = pthread_create(&threads_id[0], &attr[0], receive_rc_thread, (void*)data);
	if (res != 0){
		printf("MAIN thread activated\t\t\t[FAILED]\n");
		perror("Thread MAIN not create...\n");

		return -1;
	}
	/* receive_rc_thread((void*)data); */
	printf("MAIN thread activated\t\t\t[OK]\n");

	/* Create CAM thread here */
	/*res = pthread_create(&threads_id[1], &attr[1], camera_thread, (void*)data);
	if (res != 0){
		printf("CAM thread activated\t\t[FAILED]\n");
		perror("Thread CAM not create...\n");
		pthread_cancel(thread_id[0]);
		pthread_join(thread_id[0], NULL);
		
		return -1;
	}

	printf("CAM thread activated\t\t[OK]\n");*/

	/* Create AI thread here */
	/*res = pthread_create(&threads_id[0], &attr[0], ai_thread, (void*)data);
	if (res != 0){
		printf("AI thread activated\t\t[FAILED]\n");
		perror("Thread AI not create...\n");
		pthread_cancel(thread_id[0]);
		pthread_join(thread_id[0], NULL);
		pthread_cancel(thread_id[1]);
		pthread_join(thread_id[1], NULL);
		
		return -1;
	}

	printf("AI thread activated\t\t[OK]\n");*/

	return 0;
}


void piboat_wait(shared_data_t *d, pthread_t *thread_id)
{
	/* Wait signal for stop programm */
	pthread_mutex_lock(&finish_mutex);
	pthread_cond_wait(&cond_finish, &finish_mutex);

	/* Stop all thread */
	pthread_cancel(thread_id[0]);
	/* pthread_cancel(thread_id[1]); */
	/* pthread_cancel(thread_id[2]); */

	printf("Thread canceled\t\t\t\t[OK]\n");

	/* Wait threads termination */
	pthread_join(thread_id[0], NULL);
	/* pthread_join(thread_id[1], NULL); */
	/* pthread_join(thread_id[2], NULL); */

	printf("Thread joins\t\t\t\t[OK]\n");

	pthread_mutex_destroy(&finish_mutex);
	pthread_mutex_destroy(&(d->pwm_mutex));

	printf("Memory free\t\t\t\t[OK]\n");
}
