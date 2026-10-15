#ifndef EVENT_H
#define EVENT_H

#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/netmgr.h>

#define STEP_TIMER (_PULSE_CODE_MINAVAIL + 1) 		// Timer pulse
#define CAR_NS (_PULSE_CODE_MINAVAIL + 2)			// Car at NS sense pulse
#define CAR_EW (_PULSE_CODE_MINAVAIL + 3)			// ...
#define PED_NS (_PULSE_CODE_MINAVAIL + 4)			// ...
#define PED_EW (_PULSE_CODE_MINAVAIL + 5)			// ...
#define STATUS_DUE (_PULSE_CODE_MINAVAIL + 6)		// Pulse to send message to central

typedef struct _pulse my_message_t;

typedef struct{
	int chid;
	int coid;
} EventParam;

int eventInit(EventParam *param);
void eventCleanUp(EventParam *param);

#endif
