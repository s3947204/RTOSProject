#include "TrafficLight.h"

phase cycle[NUM_PHASES] = {
	{ "NSG-EWR", 10, EW}, // hold until EW sensor goes off
	{ "NSY-EWR", 3, -1},
	{ "NSR-EWR", 1, -1},
	{ "NSR-EWG", 10, NS}, // hold until NS sensor goes off
	{ "NSR-EWY", 3, -1},
	{ "NSR-EWR", 1, -1}
};

int readSensor(){
	return (rand() % 5) == 0;
}

void trafficInit(TrafficParam *param, int mode){
	param->mode = mode;
	param->phase = 0;
	param->elapsed = 0;

    printf("%s\n", cycle[param->phase].lights);
}

int phaseFinished(TrafficParam *param){
	phase *p = &cycle[param->phase];
	if(param->elapsed < p->duration){
		return 0;
	}

	if(param->mode == MODE_SENSOR && p->holdUntil != -1){
		return readSensor();
	}

	return 1;
}


void trafficTick(TrafficParam *param){
	param->elapsed++;

	if(phaseFinished(param)){
		param->elapsed = 0;
		param->phase = (param->phase + 1) % NUM_PHASES;
	    printf("%s\n", cycle[param->phase].lights);
	}
}


