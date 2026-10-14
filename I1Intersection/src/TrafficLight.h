#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_N

#include <stdio.h>
#include <stdlib.h>

#define NUM_PHASES 6
#define WALK_TIME 6 // ped green in seconds
#define FLASH_TIME 4

typedef enum { DIR_NS, DIR_EW, DIR_NONE } Direction;
typedef enum { MODE_CYCLING, MODE_SENSOR } Mode;
typedef enum {TYPE_GREEN, TYPE_YELLOW, TYPE_ALLRED} PhaseKind;
typedef enum {PED_RED, PED_GREEN, PED_FLASHING} PedState;

typedef struct {
	const char *lights;
	int duration;  		// in seconds
	Direction holdUntil; 		// sensor that ends the green phase
	Direction dir; 				// Road this phase belongs to
	PhaseKind kind;
} phase;

typedef struct {
	int pedRequest; 	// Button pressed for a particular road
	int pedActive;
} RoadState;

typedef struct {
	Mode mode;
	int phase;
	int elapsed;

	RoadState ns;
	RoadState ew;
} TrafficParam;

void trafficInit(TrafficParam *param, int mode);
void trafficTick(TrafficParam *param);

#endif





