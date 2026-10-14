#include <stdio.h>
#include <stdlib.h>
#include "StepTimer.h"
#include "TrafficLight.h"

int main(int argc, char *argv[])
{
	printf("main thread started....\n");
	int rcvid;
	my_message_t msg;
	TimerParam timerParam;
	TrafficParam traffic;

	srand(time(NULL));

	// intialise the timer for a step of 1s
	if(timerInit(&timerParam, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}


	printf("Timer started. Waiting for pulses...\n");
	trafficInit(&traffic, MODE_SENSOR);


	while(1){
		rcvid = MsgReceive(timerParam.chid, &msg, sizeof(msg), NULL);
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
