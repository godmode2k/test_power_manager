/* --------------------------------------------------------------
Project:	
Purpose:	Test System Lib
Author:		Ho-Jung Kim (godmode2k@hotmail.com)
Date:		November 11, 2013
Filename:	test_power_manager.c

Last modified:
License:
-----------------------------------------------------------------
Note:
 - Tested on Cygwin
-----------------------------------------------------------------
-------------------------------------------------------------- */



//! Header
// ---------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// ---------------------------------------------------------------



//! Definition
// ---------------------------------------------------------------
#define SERVERPORT		5000
#define SERVERPORT_BC	4999

#define _MAC_ID_		XXXXXX	// 6 bytes
// ---------------------------------------------------------------



//! Prototype
// ---------------------------------------------------------------
void func_query_broadcast(const char* ipaddr, const char* port, const int onoff);
void func_query_info(const char* ipaddr, const char* port);
void func_turn_onoff(const char* ipaddr, const char* port, const int onoff);
// ---------------------------------------------------------------





//! MAIN
// ---------------------------------------------------------------
int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in svr_addr;
    int numbytes;
	int optVal = 1;
	int udp = 0;



    if ( argc < 3 ) {
		printf( "\n" );
		fprintf( stderr, "[Turn on/off]\n" );
        fprintf( stderr, "usage: ip port {turn on/off: 1 if on, otherwise 0}\n" );
		printf( "\n" );
		fprintf( stderr, "[Broadcast]\n" );
        fprintf( stderr, "usage: ip port 255 {turn on/off: 1 if on, otherwise 0}\n" );
		printf( "\n" );
		fprintf( stderr, "[Get info]\n" );
        fprintf( stderr, "usage: ip port\n" );

        exit( 1 );
    }

	if ( argc == 3 ) {
		// Get info (PowerManager AP's IP address)
		func_query_info( argv[1], argv[2] );
	}
	else if ( argc == 5 ) {
		if ( atoi(argv[3]) == 255 ) {
			// Broadcast
			func_query_broadcast( argv[1], argv[2], atoi(argv[4]) );
		}
		else {
			// Turn on/off
			func_turn_onoff( argv[1], argv[2], atoi(argv[3]) );
		}
	}
	else {
		printf( "\n" );
		fprintf( stderr, "[Turn on/off]\n" );
        fprintf( stderr, "usage: ip port {turn on/off: 1 if on, otherwise 0}\n" );
		printf( "\n" );
		fprintf( stderr, "[Broadcast]\n" );
        fprintf( stderr, "usage: ip port 255 {turn on/off: 1 if on, otherwise 0}\n" );
		printf( "\n" );
		fprintf( stderr, "[Get info]\n" );
        fprintf( stderr, "usage: ip port\n" );

        exit( 1 );
	}

	return 0;
}


// ------------------------------------------------------------------------------


void func_query_broadcast(const char* ipaddr, const char* port, const int onoff) {
    int sockfd;
    int sockfd_bcr;
    struct sockaddr_in svr_addr;
    struct sockaddr_in svr_bcr_addr;
	unsigned int svr_addr_len = 0;
	unsigned int svr_bcr_addr_len = 0;
	struct hostent* host_ent;
    int numbytes;
	int optVal = 1;
	int optVal_bcr = 1;
	int optVal_broadcast = 1;	// broadcast permission

	const char* cmd = "PM Req!";
	unsigned char send_buf[24+1] = { 0, };
	char recv_buf[1024] = { 0, };

	printf( "func_query_broadcast()\n" );

	printf( "[+] gethostbyname...\n" );
	if ( (host_ent = gethostbyname(ipaddr)) == NULL ) {
		perror( "gethostbyname" );
		exit( 1 );
	}

	printf( "[+] socket...\n" );
	printf( "[+] UDP...\n" );
	if ( (sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) {
		perror( "socket" );
		exit( 1 );
	}

	printf( "[+]UDP... for Broadcast receiver\n" );
	if ( (sockfd_bcr = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) {
		perror( "socket: broadcast receiver" );
		exit( 1 );
	}

	printf( "[+] socket opt...\n" );
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal))  == -1 ) {
        perror( "setsockopt" );
		close( sockfd );
        exit( 1 );
	}
	if ( setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optVal_broadcast, sizeof(optVal_broadcast)) == -1 ) {
        perror( "setsockopt" );
		close( sockfd );
        exit( 1 );
    }

	printf( "[+] socket opt... for Broadcast receiver\n" );
    if ( setsockopt(sockfd_bcr, SOL_SOCKET, SO_REUSEADDR, &optVal_bcr, sizeof(optVal_bcr))  == -1 ) {
        perror( "setsockopt" );
		close( sockfd );
		close( sockfd_bcr );
        exit( 1 );
	}


    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons( (unsigned int)atoi(port) );
    svr_addr.sin_addr.s_addr = inet_addr( ipaddr );

	// for broadcast receiver
    svr_bcr_addr.sin_family = AF_INET;
    svr_bcr_addr.sin_port = htons( SERVERPORT_BC );
    svr_bcr_addr.sin_addr.s_addr = htonl( INADDR_ANY );


	{
		printf( "[+] send...\n" );
		snprintf( send_buf, sizeof(send_buf), "%s", cmd );
		if ( (numbytes = sendto(sockfd, send_buf, strlen(send_buf), 0,
					(struct sockaddr *)&svr_addr, sizeof(svr_addr))) == -1 ) {
			perror( "sendto" );
			close( sockfd );
			close( sockfd_bcr );
			exit( 1 );
		}

		if ( bind(sockfd_bcr, (struct sockaddr *)&svr_bcr_addr, sizeof(svr_bcr_addr)) == -1 ) {
			perror( "bind: broadcast receiver" );
			close( sockfd );
			close( sockfd_bcr );
			exit( 1 );
		}

		svr_bcr_addr_len = sizeof( svr_bcr_addr );
		if ( (numbytes = recvfrom(sockfd_bcr, recv_buf, sizeof(recv_buf), 0,
					(struct sockaddr *)&svr_bcr_addr, &svr_bcr_addr_len)) == -1 ) {
			perror( "recvfrom: broadcast receiver" );
			close( sockfd );
			close( sockfd_bcr );
			exit( 1 );
		}
	}

	if ( strlen(recv_buf) > 0 ) {
		// parse the ipaddr
		// 	syntax: MAC_ADDR/IP_ADDR/PM_"?????"_"MAC_ADDR"/, len = 52
		//	e.g.,: XXXXXX/192.168.xxx.xxx/PM_XXXXX_XXXXXX/, len = 52

		int ifs_ipaddr_start = 0;
		int ifs_ipaddr_end = 0;
		char* _res = &recv_buf;

		ifs_ipaddr_start = strcspn( (const char*)&recv_buf, "/" );
		ifs_ipaddr_start += 1;

		ifs_ipaddr_end = strcspn( ((const char*)&recv_buf + ifs_ipaddr_start), "/" );

		if ( strtok((_res + ifs_ipaddr_start), "/" ) ) {
			printf( "split last '/': ipaddr = %s\n", _res );

			if ( _res ) {
				const char* res_ipaddr =  (_res + ifs_ipaddr_start);
				char buf_port[4+1] = { 0, };

				snprintf( buf_port, sizeof(buf_port), "%d", SERVERPORT );
				printf( "ipaddr = %s:%s, on/off = %d\n", res_ipaddr, buf_port, onoff );

				func_turn_onoff( res_ipaddr, buf_port, onoff );
			}
			else
				printf( "result == NULL\n" );
		}
		else
			printf( "split == NULL\n" );
	}

	close( sockfd );
	close( sockfd_bcr );
}

void func_query_info(const char* ipaddr, const char* port) {
    int sockfd;
    struct sockaddr_in svr_addr;
	unsigned int svr_addr_len = 0;
    int numbytes;
	int optVal = 1;

	const char* cmd = "PM Req!";
	unsigned char send_buf[24+1] = { 0, };
	char recv_buf[1024] = { 0, };

	printf( "func_query_info()\n" );


	printf( "[+] socket...\n" );
	printf( "[+] UDP...\n" );
	if ( (sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) {
		perror( "socket" );
		exit( 1 );
	}

	printf( "[+] socket opt...\n" );
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal))  == -1 ) {
        perror( "setsockopt" );
		close( sockfd );
        exit( 1 );
	}

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons( SERVERPORT_BC );
    svr_addr.sin_addr.s_addr = inet_addr( ipaddr );


	printf( "[+] connect... ip = %s:%s\n", ipaddr, port );
	if ( connect(sockfd, (struct sockaddr*)&svr_addr, sizeof(svr_addr)) == -1 ) {
        perror( "connect" );
		close( sockfd );
        exit( 1 );
	}

	printf( "[+] send...\n" );
	snprintf( send_buf, sizeof(send_buf), "%s", cmd );
	if ( (numbytes = sendto(sockfd, send_buf, strlen(send_buf), 0,
				(struct sockaddr *)&svr_addr, sizeof(svr_addr))) == -1 ) {
		perror( "sendto" );
		close( sockfd );
		exit( 1 );
	}

	printf( "[+] recv...\n" );
	svr_addr_len = sizeof( svr_addr );
	if ( (numbytes = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0,
				(struct sockaddr *)&svr_addr, &svr_addr_len)) == -1 ) {
		perror( "recvfrom" );
		close( sockfd );
		exit( 1 );
	}
	printf("recv = %s, len = %d\n", recv_buf, numbytes );

	close( sockfd );
}

void func_turn_onoff(const char* ipaddr, const char* port, const int onoff) {
	// Turn on/off command packet structure
	// Example:
	/*
	STX  | MAC Id            | Cmd | SSID     | EXT CS CR
	-----+-------------------+-----+----------+-----------
	0xfa | xx xx xx xx xx xx | 50  | xx xx xx | fb f3 0d
	-----+-------------------+-----+----------+-----------
	  .  | X  X  X  X  X  X  | P   | .  .  .  | .  .  .
	*/

    int sockfd;
    struct sockaddr_in svr_addr;
    int numbytes;
	int optVal = 1;

	unsigned char send_buf[24+1] = { 0, };
	char recv_buf[1024] = { 0, };

	printf( "func_turn_onoff()\n" );
	printf( "func_turn_onoff(): ipaddr = %s, len = %d, port = %s\n", ipaddr, strlen(ipaddr), port );


	printf( "[+] socket...\n" );
	printf( "[+] TCP...\n" );
	if ( (sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ) {
		perror( "socket" );
		exit( 1 );
	}

	printf( "[+] socket opt...\n" );
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal))  == -1 ) {
        perror( "setsockopt" );
		close( sockfd );
        exit( 1 );
	}

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons( (unsigned int)atoi(port) );
    svr_addr.sin_addr.s_addr = inet_addr( ipaddr );

	printf( "[+] connect... ip = %s:%s\n", ipaddr, port );
	if ( connect(sockfd, (struct sockaddr*)&svr_addr, sizeof(svr_addr)) == -1 ) {
        perror( "connect" );
		close( sockfd );
        exit( 1 );
	}


	if ( onoff == 1 ) {
		const int status = 1;	// Relay on
		unsigned char onOff = 0x57;

		printf( "[+] Turn On...\n" );

		// On: 'W' (0x57)
		// MAC ID: XXXXXX (6 bytes)
		//snprintf( send_buf, sizeof(send_buf), "%c_MAC_ID_%c%c%c%c%c%c%c%c", 0xfa, 0x57, 0x01, 0xfb, 0xf3, 0x0d );
		snprintf( send_buf, sizeof(send_buf), "%c%s%c%c%c%c%c%c%c%c", 0xfa, _MAC_ID_, 0x57, 0x01, 0xfb, 0xf3, 0x0d );
		if ( (numbytes = send(sockfd, send_buf, strlen(send_buf), 0)) == -1 ) {
			perror( "send" );
			close( sockfd );
			exit( 1 );
		}

		printf( "[+] recv...\n" );
		if ( (numbytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) == -1 ) {
			perror( "recv" );
			close( sockfd );
			exit( 1 );
		}
	}
	else if ( !onoff ) {
		const int status = 0;	// Relay off
		unsigned char onOff = 0x57;

		onOff = status & ~0x01;

		printf( "[+] Turn Off...\n" );


		printf( "[+] # 1of3\n" );
		printf( "[+] send...\n" );

		// MAC ID: XXXXXX (6 bytes)
		//snprintf( send_buf, sizeof(send_buf), "%c_MAC_ID_%c%c%c%c%c%c%c%c", 0xfa, 0x57, 0xa5, 0xfb, 0xf1, 0x0d );
		snprintf( send_buf, sizeof(send_buf), "%c%s%c%c%c%c%c%c%c%c", 0xfa, _MAC_ID_, 0x57, 0xa5, 0xfb, 0xf1, 0x0d );
		if ( (numbytes = send(sockfd, send_buf, strlen(send_buf), 0)) == -1 ) {
			perror( "send" );
			close( sockfd );
			exit( 1 );
		}
		
		printf( "[+] recv...\n" );
		if ( (numbytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) == -1 ) {
			perror( "recv" );
			close( sockfd );
			exit( 1 );
		}


		printf( "[+] # 2of3\n" );
		printf( "[+] send...\n" );

		// MAC ID: XXXXXX (6 bytes)
		//snprintf( send_buf, sizeof(send_buf), "%c_MAC_ID_%c%c%c%c%c%c%c%c", 0xfa, 0x57, 0xa4, 0xfb, 0xf0, 0x0d );
		snprintf( send_buf, sizeof(send_buf), "%c%s%c%c%c%c%c%c%c%c", 0xfa, _MAC_ID_, 0x57, 0xa4, 0xfb, 0xf0, 0x0d );
		if ( (numbytes = send(sockfd, send_buf, strlen(send_buf), 0)) == -1 ) {
			perror( "send" );
			close( sockfd );
			exit( 1 );
		}
		
		printf( "[+] recv...\n" );
		if ( (numbytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) == -1 ) {
			perror( "recv" );
			close( sockfd );
			exit( 1 );
		}
		

		printf( "[+] # 3of3\n" );
		printf( "[+] send...\n" );

		// Off: '' ()
		// MAC ID: XXXXXX (6 bytes)
		//snprintf( send_buf, sizeof(send_buf), "%c_MAC_ID_%c%c%c%c%c%c%c%c", 0xfa, 0x57, 0x01, 0xfb, 0x54, 0x0d );
		snprintf( send_buf, sizeof(send_buf), "%c%s%c%c%c%c%c%c%c%c", 0xfa, _MAC_ID_, 0x57, 0x01, 0xfb, 0x54, 0x0d );

		if ( (numbytes = send(sockfd, send_buf, strlen(send_buf), 0)) == -1 ) {
			perror( "send" );
			close( sockfd );
			exit( 1 );
		}

		printf( "[+] recv...\n" );
		if ( (numbytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0)) == -1 ) {
			perror( "recv" );
			close( sockfd );
			exit( 1 );
		}
	}

	close( sockfd );
}
