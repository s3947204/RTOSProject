#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_N

#include <stdio.h>
#include <stdlib.h>

#define NUM_PHASES 6

enum { NS = 0, EW = 1 };
enum { MODE_CYCLING, MODE_SENSOR };

typedef struct {
	const char *lights;
	int duration;  		// in seconds
	int holdUntil; 		// sensor that ends the green phase
} phase;


typedef struct {
	int mode;
	int phase;
	int elapsed;
} TrafficParam;

void trafficInit(TrafficParam *param, int mode);
void trafficTick(TrafficParam *param);

#endif





