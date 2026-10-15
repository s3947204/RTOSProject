#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>



#define ATTACH_POINT "MainControllerServer"
#define TOGGLE_PULSE  (_PULSE_CODE_MINAVAIL + 1)

/* We specify the header as being at least a pulse */
typedef struct _pulse msg_header_t;

typedef enum { DIR_NS, DIR_EW, DIR_NONE } Direction;
typedef enum {TYPE_GREEN, TYPE_YELLOW, TYPE_ALLRED} PhaseKind;

typedef struct {
	const char *lights;
	int duration;  		// in seconds
	Direction holdUntil; 		// sensor that ends the green phase
	Direction dir; 				// Road this phase belongs to
	PhaseKind kind;
} phase;


//typedef struct _my_data {
//    msg_header_t hdr;
//    int id;
//    int mode;         		// MODE_CYCLING = 0, MODE_SENSOR = 1
//    int phase;        		// index into the phase table
//    int elapsed;      		// seconds in this phase
//    int nsPed, ewPed; 		// PedState: 0 RED, 1 GREEN, 2 FLASHING
//    int nsCar, ewCar; 		// car waiting
//    int nsPedReq, ewPedReq;
//} traffic_status_t;

/* Common header for all nodes */
typedef struct {
    msg_header_t hdr;
    int id;
    int type;
} node_message_t;

/* Intersection status */
typedef struct {
    msg_header_t hdr;
    int id;
    int mode;
    int phase;
    int elapsed;
    int nsPed, ewPed;
    int nsCar, ewCar;
    int nsPedReq, ewPedReq;
} traffic_status_t;

/* Train crossing status */
typedef struct {
    msg_header_t hdr;
    int id;
    int crossingHeartbeat;
    int crossingState;
} crossing_status_t;


typedef struct {
	name_attach_t *attach;
	int selfCoid;
	int togglePending;
} ServerParam;

typedef enum { CMD_NONE, MODE_TOGGLE } Cmd;
typedef struct {
	Cmd cmd;
} reply_t;

int serverInit(ServerParam *param);
void *serverRun(void *arg);
void *serverConsole(void *arg);
void serverCleanUp(ServerParam *param);

#endif
