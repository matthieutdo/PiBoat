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

#include <syslog.h>
#include <sys/queue.h>

#include "thread_manager.h"

struct module_entry {
	LIST_ENTRY(module_entry) next;
	module_t *module;

	pthread_t thread;
	pthread_attr_t thread_attr;
};

LIST_HEAD(module_list, module_entry);
struct module_list module_list = LIST_HEAD_INITIALIZER(module_list);

static pthread_mutex_t finish_mutex;
static pthread_cond_t cond_finish;

int register_module(module_t *mod)
{
	struct module_entry *e;

	e = malloc(sizeof(struct module_entry));
	if (e == NULL) {
		printf("Failed to allocate memory for module %s", mod->name);
		return -1;
	}

	e->module = mod;
	LIST_INSERT_HEAD(&module_list, e, next);

	return 0;
}

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

int module_start(shared_data_t *data)
{
	struct module_entry *mod_e;
	int res;

	/* Init mutex for ended programm */
	pthread_mutex_init(&finish_mutex, NULL);
	pthread_cond_init(&cond_finish, NULL);

	/* Intercept signal */
	signal(SIGINT, finish_prog);
	signal(SIGQUIT, finish_prog);
	signal(SIGTERM, finish_prog);

	LIST_FOREACH(mod_e, &module_list, next) {
		pthread_attr_init(&(mod_e->thread_attr));
		pthread_attr_setdetachstate(&(mod_e->thread_attr),
					    PTHREAD_CREATE_JOINABLE);

		res = pthread_create(&(mod_e->thread), &(mod_e->thread_attr),
				     mod_e->module->loop, (void*)data);
		if (res != 0){
			/* XXX errno */
			syslog(LOG_EMERG,
			       "Thread %s activated          [FAILED]\n",
			       mod_e->module->name);

			return -1;
		}
		syslog(LOG_INFO, "Thread %s activated           [  OK  ]\n",
		       mod_e->module->name);
	}

	return 0;
}

void piboat_wait(shared_data_t *d)
{
	struct module_entry *mod_e;

	/* Wait signal for stop programm */
	pthread_mutex_lock(&finish_mutex);
	pthread_cond_wait(&cond_finish, &finish_mutex);

	LIST_FOREACH(mod_e, &module_list, next) {
		pthread_cancel(mod_e->thread);

		syslog(LOG_INFO,
		       "Thread %s canceled                 [  OK  ]\n",
		       mod_e->module->name);
	}

	LIST_FOREACH(mod_e, &module_list, next) {
		pthread_join(mod_e->thread, NULL);

		syslog(LOG_INFO,
		       "Thread %s joined                   [  OK  ]\n",
		       mod_e->module->name);
	}

	pthread_mutex_destroy(&finish_mutex);
	pthread_mutex_destroy(&(d->pwm_mutex));

	syslog(LOG_INFO, "Memory freed                    [  OK  ]\n");
}
