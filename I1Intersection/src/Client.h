#ifndef CLIENT_H
#define CLIENT_H
#include <sys/neutrino.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define ATTACH_POINT "MainControllerServer"

typedef struct _pulse msg_header_t;

typedef struct _my_data {
    msg_header_t hdr;
    int          data;
} my_data_t;

typedef struct {
	int serverCoid; // client will use this to send a msg to central
	int mainChid;	// receive pulse from main to send status to central
	int mainCoid; 	// Main will use this to send a pulse to client thread
} ClientParam;

int clientInit(ClientParam *param);
void *clientRun(void *arg);
void clientCleanUp(ClientParam *param);

#endif
