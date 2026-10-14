#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

typedef struct {
	timer_t timerId;
	int timerCreated;
} TimerParam;


int timerInit(TimerParam *param, int coid, int intervalSec);
void timerCleanUp(TimerParam *param);
#endif
