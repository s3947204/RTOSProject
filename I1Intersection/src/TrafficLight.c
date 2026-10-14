#include "TrafficLight.h"

phase cycle[NUM_PHASES] = {
	{ "NSG-EWR", 10, DIR_EW, DIR_NS, TYPE_GREEN}, // hold until EW sensor goes off
	{ "NSY-EWR", 3, DIR_NONE, DIR_NS, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_NS, TYPE_ALLRED},
	{ "NSR-EWG", 10, DIR_NS, DIR_EW, TYPE_GREEN}, // hold until NS sensor goes off
	{ "NSR-EWY", 3, DIR_NONE, DIR_EW, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_EW, TYPE_ALLRED}
};

// This needs to match this enum in the .h file
// typedef enum {PED_RED, PED_GREEN, PED_FLASHING} PedState;
char *pedOutputString[] = { "RED", "GREEN", "FLASHING" };

int readSensor(){
	return (rand() % 5) == 0;
}

// Random pedestrian button
void readPedButtons(TrafficParam *param){
	param->ns.pedRequest = 1;
	param->ew.pedRequest = 1;
}

RoadState* roadOf(TrafficParam *param, Direction dir){
	if(dir == DIR_NS){
		return &param->ns;
	}else{
		return &param->ew;
	}
}

// Are we currently in the green phase for a given dir?
int isGreenFor(phase *p, Direction dir){
	return (p->kind == TYPE_GREEN) && (p->dir == dir);
}

// What is the current state of the pedestrian lights for a given direction?
PedState pedState(TrafficParam *param, Direction dir){
	phase *p = &cycle[param->phase];

	// If not in green phase, then pedestiran is RED
	if(!isGreenFor(p, dir)){
		return PED_RED;
	}

	// if light is not active, then state is red
	if(!(roadOf(param, dir)->pedActive)){
		return PED_RED;
	}

	if(param->elapsed < WALK_TIME){
		return PED_GREEN;
	}

	if(param->elapsed < (WALK_TIME + FLASH_TIME)){
		return PED_FLASHING;
	}

	return PED_RED;
}





void enterPhase (TrafficParam *param){
	phase *p = &cycle[param->phase];
	RoadState *road = roadOf(param, p->dir);

	// If phase kind is currently greed and there is a request
	// Set the peadestrian light to be active
	// Zero the request
	if(p->kind == TYPE_GREEN && road->pedRequest){
		road->pedActive = 1;
		road->pedRequest = 0;
	}
}

void leavePhase(TrafficParam *param){
	phase *p = &cycle[param->phase];
	// When leaving from a green phase, set ped active to 0
	if(p->kind == TYPE_GREEN){
		roadOf(param, p->dir) -> pedActive = 0;
	}
}

int hasDemand(TrafficParam *param, Direction dir){
	// if there is car waiting at the given direction
	// or if there is a ped request
	return readSensor(dir) || roadOf(param, dir) -> pedRequest;
}

int phaseFinished(TrafficParam *param){
	phase *p = &cycle[param->phase];

	if(param->elapsed < p->duration){
		return 0;
	}

	// If cycling and duration has finished, then
	// phase is also finished
	if(param->mode == MODE_CYCLING){
		return 1;
	}

	// If we are not in green state, duration has elapsed
	// and in sensor mode
	// then phase is finished
	if(p->holdUntil == DIR_NONE){
		return 1;
	}

	return hasDemand(param, p->holdUntil); // sensor mode
}

void printStatus(TrafficParam *param){
	PedState ns = pedState(param, DIR_NS);
	PedState ew = pedState(param, DIR_EW);

	RoadState *EWRoad = roadOf(param, DIR_EW);
	RoadState *NSRoad = roadOf(param, DIR_NS);
	// Source - https://stackoverflow.com/a/7660837
	// Posted by Avinash Katiyar, modified by community. See post 'Timeline' for change history
	// Retrieved 2026-10-14, License - CC BY-SA 3.0
//	printf("\e[1;1H\e[2J");


	printf("%s %2ds/%2d		NS ped: %-8s EW ped: %s\n",
			cycle[param->phase].lights,
			param->elapsed,
			cycle[param->phase].duration,
			pedOutputString[ns],
			pedOutputString[ew]
	);
}



void trafficInit(TrafficParam *param, int mode){
	param->mode = mode;
	param->phase = 0;
	param->elapsed = 0;

	param->ns.pedRequest = 0; 		// No request for sensor
	param->ns.pedActive = 0;  		// Currently inactive

	param->ew.pedRequest = 0; 		// No request for sensor
	param->ew.pedActive = 0;  		// Currently inactive

	enterPhase(param);
	printStatus(param);
}

void trafficTick(TrafficParam *param){
	param->elapsed++;
    readPedButtons(param);

	if(phaseFinished(param)){
		leavePhase(param);
		param->elapsed = 0;
		param->phase = (param->phase + 1) % NUM_PHASES;
		enterPhase(param);
	}
	printStatus(param);
}


