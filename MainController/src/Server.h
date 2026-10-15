#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>


#define ATTACH_POINT "MainControllerServer"

/* We specify the header as being at least a pulse */
typedef struct _pulse msg_header_t;

/* Our real data comes after the header */
//typedef struct _my_data {
//    msg_header_t hdr;
//    int          data;
//} my_data_t;

typedef struct _my_data {
    msg_header_t hdr;
    int  mode;         		// MODE_CYCLING = 0, MODE_SENSOR = 1
    int  phase;        		// index into the phase table
    int  elapsed;      		// seconds in this phase
    int  nsPed, ewPed; 		// PedState: 0 RED, 1 GREEN, 2 FLASHING
    int  nsCar, ewCar; 		// car waiting
    int  nsPedReq, ewPedReq;
} traffic_status_t;


typedef struct {
	name_attach_t *attach;
} ServerParam;

int serverInit(ServerParam *param);
void *serverRun(void *arg);
void serverCleanUp(ServerParam *param);

#endif
