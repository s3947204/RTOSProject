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
	union {
	    my_message_t pulse;
	    int request;
	} msg;
    ClientParam  client;
	TimerParam timerParam;
	InputParam inputParam;
	TrafficParam traffic;
	// separates the connection from the Timer
	EventParam inputAndTimerEventParam;
	int clientInitSuccess;



    pthread_t input, clientThread;

	if(eventInit(&inputAndTimerEventParam) == -1){
		printf("Failed to intialise input and timer connections");

	}

	clientInitSuccess = clientInit(&client);
	if (clientInitSuccess == -1) {
		printf("Failed to initialise client connections\n");
	}

	// the input thread needs to send the message to this connection
	inputParam.coid = inputAndTimerEventParam.coid;

    pthread_create(&input, NULL, inputThread, &inputParam);

    if(clientInitSuccess == 0){
    	printf("Creating client thread\n");
    	client.eventCoid = inputAndTimerEventParam.coid;
    	pthread_create(&clientThread, NULL, clientRun, &client);
    }

	// initialise the timer for a step of 1s
    // timer needs a connection id to send a message to this channel
	if(timerInit(&timerParam, inputAndTimerEventParam.coid, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}


	printf("Timer started. Waiting for pulses...\n");

	trafficInit(&traffic, MODE_CYCLING);
	while(1){
		rcvid = MsgReceive(inputAndTimerEventParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}

		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			switch(msg.pulse.code){
			case STEP_TIMER:
				trafficTick(&traffic);
				if(clientInitSuccess == 0){
					MsgSendPulse(client.mainCoid, -1, STATUS_DUE, 0);
				}
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
			case MODE_TOGGLE_PULSE:
				if(traffic.mode == MODE_CYCLING){
					traffic.mode = MODE_SENSOR;
				}else{
					traffic.mode = MODE_CYCLING;
				}
				printf("Toggling mode\n");
			default:
				break;
			}
		}

		if(rcvid > 0){
			traffic_status_t status = {
				 .id = CONTROLLER_ID,
				 .mode = traffic.mode,
				 .phase = traffic.phase,
				 .elapsed = traffic.elapsed,
				 .nsPed = pedState(&traffic, DIR_NS),
				 .ewPed = pedState(&traffic, DIR_EW),
				 .nsCar = traffic.ns.carSensor,
				 .ewCar = traffic.ew.carSensor,
				 .nsPedReq = traffic.ns.pedRequest,
				 .ewPedReq = traffic.ew.pedRequest,
			};

			MsgReply(rcvid, EOK, &status, sizeof(status));
		}
	}

	printf("\nMain Terminating....");
	return EXIT_SUCCESS;
}
