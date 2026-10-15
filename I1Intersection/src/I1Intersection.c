#include <stdio.h>
#include <stdlib.h>
#include "StepTimer.h"
#include "TrafficLight.h"
#include "Event.h"
#include "Input.h"

int main(int argc, char *argv[])
{
	printf("main thread started....\n");
	int rcvid;
	my_message_t msg;
	TimerParam timerParam;
	InputParam inputParam;
	TrafficParam traffic;
	EventParam eventParam;
	pthread_t input;

	srand(time(NULL));

	if(eventInit(&eventParam) == -1){
		printf("Failed to intialise connection");
		return EXIT_FAILURE;
	}

	inputParam.coid = eventParam.coid;
    pthread_create(&input, NULL, inputThread, &inputParam);

	// intialise the timer for a step of 1s
	if(timerInit(&timerParam, eventParam.coid, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}


	printf("Timer started. Waiting for pulses...\n");
	trafficInit(&traffic, MODE_SENSOR);

	while(1){
		rcvid = MsgReceive(eventParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}
		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			switch(msg.code){
			case STEP_TIMER:
				trafficTick(&traffic);
				break;
			case CAR_NS:
				trafficSetSensor(&traffic, DIR_NS);
				printf("Car present at NS\n");
				break;
			case CAR_EW:
				trafficSetSensor(&traffic, DIR_EW);
				printf("Car present at EW\n");
				break;
			case PED_NS:
				trafficSetPed(&traffic, DIR_NS);
				printf("Ped present at NS\n");
				break;
			case PED_EW:
				trafficSetPed(&traffic, DIR_EW);
				printf("Ped present at EW\n");
				break;
			default:
				break;
			}
		}
	}

	printf("\nMain Terminating....");
	return EXIT_SUCCESS;
}
