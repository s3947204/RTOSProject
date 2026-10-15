#include <stdio.h>
#include <stdlib.h>
#include "StepTimer.h"
#include "TrafficLight.h"
#include "Event.h"
#include "Input.h"
#include "Client.h"

int main(int argc, char *argv[])
{
	printf("main thread started....\n");
	int rcvid;
	my_message_t msg;
    ClientParam  client;
	TimerParam timerParam;
	InputParam inputParam;
	TrafficParam traffic;
	// separates the connection from the Timer
	EventParam inputAndTimerEventParam;


    pthread_t input, clientThread;

	if(eventInit(&inputAndTimerEventParam) == -1){
		printf("Failed to intialise input and timer connections");

	}

	if (clientInit(&client) == -1) {
		printf("Failed to initialise client connections\n");
	}

	// the input thread needs to send the message to this connection
	inputParam.coid = inputAndTimerEventParam.coid;

    pthread_create(&input, NULL, inputThread, &inputParam);
    pthread_create(&clientThread, NULL, clientRun, &client);

	// initialise the timer for a step of 1s
    // timer needs a connection id to send a message to this channel
	if(timerInit(&timerParam, inputAndTimerEventParam.coid, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}


	printf("Timer started. Waiting for pulses...\n");
	trafficInit(&traffic, MODE_SENSOR);

	while(1){
		rcvid = MsgReceive(inputAndTimerEventParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}
		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			switch(msg.code){
			case STEP_TIMER:
				trafficTick(&traffic);
	            MsgSendPulse(client.mainCoid, -1, STATUS_DUE, 0);
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
