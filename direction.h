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

#ifndef _direction_h
#define _direction_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pwm.h"
#include "shared_data.h"

/**************************************************************
 *	Donne la position des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *
 *	@return pos	Position (en degres)
 **************************************************************/
void get_direction(shared_data_t *data, int *pos);

/**************************************************************
 *	donne les reglages des gouvernails
 *
 *	@param fd	Gestionnaire PWM
 *
 *	@return reg	Valeur des relglages
 **************************************************************/
void get_dir_adjust(shared_data_t *data, int *reg);

#endif
