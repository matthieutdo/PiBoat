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


#ifndef __DIRECTION__
#define __DIRECTION__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pwm.h"


/**************************************************************
 *	Initialisation position des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 **************************************************************/
void init_direction(int fd);


/**************************************************************
 *	deinitialisation position des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 **************************************************************/
void deinit_direction(int fd);


/**************************************************************
 *	Change la position des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *	@param pos	nouvelle position (en degres)
 **************************************************************/
void set_direction(int fd, int pos);


/**************************************************************
 *	Donne la position des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *
 *	@return pos	Position (en degres)
 **************************************************************/
void get_direction(int fd, int *pos);


/**************************************************************
 *	change les reglages des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *	@param new_reg	Nouveau reglage
 *
 *	@return 	<0 si reglage incorrect
 **************************************************************/
int set_dir_adjust(int fd, int new_reg);


/**************************************************************
 *	donne les reglages des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *
 *	@return reg	Valeur des relglages
 **************************************************************/
void get_dir_adjust(int fd, int *reg);


#endif

