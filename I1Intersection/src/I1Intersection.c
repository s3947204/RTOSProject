#include <stdio.h>
#include <stdlib.h>
#include "StepTimer.h"
#include "TrafficLight.h"
#include "Event.h"

int main(int argc, char *argv[])
{
	printf("main thread started....\n");
	int rcvid;
	my_message_t msg;
	TimerParam timerParam;
	TrafficParam traffic;
	EventParam eventParam;
	srand(time(NULL));

	if(eventInit(&eventParam) == -1){
		printf("Failed ot intialise connection");
		return EXIT_FAILURE;
	}

	// intialise the timer for a step of 1s
	if(timerInit(&timerParam, eventParam.coid, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}


	printf("Timer started. Waiting for pulses...\n");
	trafficInit(&traffic, MODE_CYCLING);

	while(1){
		rcvid = MsgReceive(eventParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}
		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			if (msg.code == STEP_TIMER) {
				trafficTick(&traffic);
			}
		}
	}

	printf("\nMain Terminating....");
	return EXIT_SUCCESS;
}
