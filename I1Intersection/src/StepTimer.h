#ifndef TIMER_H
#define TIMER_H

#include <sys/neutrino.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/netmgr.h>

#define STEP_TIMER _PULSE_CODE_MINAVAIL

typedef struct _pulse my_message_t;

typedef struct {
	int chid;
	int coid;
	timer_t timerId;
	int timerCreated;
} TimerParam;


int timerInit(TimerParam *param, int intervalSec);
void timerCleanUp(TimerParam *param);
#endif
