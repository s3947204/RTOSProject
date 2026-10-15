#ifndef CLIENT_H
#define CLIENT_H
#include <sys/neutrino.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include "Event.h"

#define CONTROLLER_ID 1
#define ATTACH_POINT "MainControllerServer"

typedef enum { CMD_NONE, MODE_TOGGLE } Cmd;

typedef struct {
	Cmd cmd;
} reply_t;

typedef struct _pulse msg_header_t;

typedef struct _my_data {
    msg_header_t hdr;
    int id;					// Aid the server to identify which node sent the status
    int mode;         		// MODE_CYCLING = 0, MODE_SENSOR = 1
    int phase;        		// index into the phase table
    int elapsed;      		// seconds in this phase
    int nsPed, ewPed; 		// PedState: 0 RED, 1 GREEN, 2 FLASHING
    int nsCar, ewCar; 		// car waiting
    int nsPedReq, ewPedReq;
} traffic_status_t;


typedef struct _some_other {
	msg_header_t hdr;
	int data;
} my_data_t;

typedef struct {
	int serverCoid; // client will use this to send a msg to central

	// client will use this to send a pulse to main
	// there is a channel already owned my main which
	// works with inputs and timer
	int eventCoid;

	// this allows the entire system to be synced to a single timer
	// client will use chid to receive pulse from main
	// and main will use coid to send that pulse
	int mainChid;	// receive pulse from main to send status to central
	int mainCoid; 	// Main will use this to send a pulse to client thread
} ClientParam;

int clientInit(ClientParam *param);
void *clientRun(void *arg);
void clientCleanUp(ClientParam *param);

#endif
