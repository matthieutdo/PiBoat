/*************************************************************************
 *	Copyright (C) 2014-2025  TERNISIEN d'OUVILLE Matthieu
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

#ifndef _connect_tcp_h
#define _connect_tcp_h

#include <netinet/in.h>
#include <netdb.h> /* gethostbyname */
#include <unistd.h> /* close */

#include <errno.h>
#include <stdlib.h>
#include <string.h> /* memcpy */

/*#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>*/
//#include <unistd.h> /* close */

#define SOCK_NO_HOST	-1
#define SOCK_CREATE	-2
#define SOCK_CONNECT	-3
#define SOCK_BIND	-4

typedef int socket_t;

#ifdef RC_SERVER
/**************************************************************
 *	Constructeur socket serveur
 *
 *	@param max_wait		nombre max de client
 *				en attente
 *
 *	@return socket_t	Socket communication 
 *				avec le client
 **************************************************************/
socket_t init_socket_serv(int port, int max_wait);
#endif

#ifdef RC_CLIENT
/**************************************************************
 *	Initialisation socket client
 *
 *	@return socket_t	Socket communication 
 *				avec le serveur
 **************************************************************/
socket_t init_socket_client(int port, char *hostname);
#endif

/**************************************************************
 *	close_sock : ferme la socket client
 *	
 *	@param sock	Socket à fermer
 **************************************************************/
void close_sock(socket_t sock);

/**************************************************************
 *	recvMsg : Reception d'un message
 *	
 *	@param msg	Message a recevoir
 *	@param length	Taille du message (nombre d'octet
 *			à lire)
 *
 *	@return long	nombre d'octets lu. -1 si erreur
 **************************************************************/
/* long recv_msg(void* msg, size_t length); */

/**************************************************************
 *	sendMsg : Envoi un message
 *	
 *	@param msg	Message a envoyer
 *	@param length	Taille du message (nombre d'octet
 *			envoye)
 *
 *	@return long	Longueur du message envoye. -1 si
 *			erreur
 **************************************************************/
/* long send_msg(void* msg, size_t length); */

#endif
