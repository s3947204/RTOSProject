#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <sys/neutrino.h>
#include "Event.h"

typedef struct {
	int coid;
} InputParam;

void *inputThread(void *arg);

#endif
