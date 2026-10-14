#ifndef EVENT_H
#define EVENT_H

#include <sys/neutrino.h>
#include <stdio.h>

#define STEP_TIMER (_PULSE_CODE_MINIVAIL + 1)
#define CAR_NS (_PULSE_CODE_MINIVAIL + 1)
#define CAR_EW (_PULSE_CODE_MINIVAIL + 1)
#define PED_NS (_PULSE_CODE_MINIVAIL + 1)
#define PED_EW (_PULSE_CODE_MINIVAIL + 1)

typedef union {
	struct _pulse pulse;
} my_message_t;

typedef struct{
	int chid;
	int chod;
} EventParam;

int eventInit(EventParam *param);
int eventCleanUp(EventParam *param);
