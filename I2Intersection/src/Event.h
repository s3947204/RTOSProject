#ifndef EVENT_H
#define EVENT_H

#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>

#define STEP_TIMER (_PULSE_CODE_MINAVAIL + 1) 		// Timer pulse
#define MODE_TOGGLE_PULSE (_PULSE_CODE_MINAVAIL + 2)      // Mode change pulse to main from client
#define CAR_NS (_PULSE_CODE_MINAVAIL + 3)			// Car at NS sense pulse
#define CAR_EW (_PULSE_CODE_MINAVAIL + 4)			// ...
#define PED_NS (_PULSE_CODE_MINAVAIL + 5)			// ...
#define PED_EW (_PULSE_CODE_MINAVAIL + 6)			// ...

// This is a pulse that main sends to client to tell it to
// send a status to central
// This allows us to use a single timer
#define STATUS_DUE (_PULSE_CODE_MINAVAIL + 6)

typedef struct _pulse my_message_t;

typedef struct{
	int chid;
	int coid;
} EventParam;

int eventInit(EventParam *param);
void eventCleanUp(EventParam *param);

#endif
