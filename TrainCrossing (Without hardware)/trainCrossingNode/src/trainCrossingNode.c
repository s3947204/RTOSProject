#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "Timer.h"
#include <sys/neutrino.h>
#include "Client.h"

enum states {
	State0, //Waiting for train
	State1, //Train crossing
	State2 //Crossing error
};
enum states CurState = State0; //starting state

//set state to 2 when failure is detected
#define FAIL_TO_STATE2() do { \
		printf("A failure was detected\n"); \
		printf("Waiting for failure to be dismissed by master controller\n"); \
		CurState = State2; \
		return NULL; \
}while(0)

bool gatesLowered;
bool redLight;
bool crossingAlarm;
bool trainSignalRed;
bool CrossingOpen;

pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

void *communication (void *data) { //communication thread
	ClientParam clientParam;

	my_data_t clientMsg;

	if(clientInit(&clientParam) == -1){
		printf("Failed to initialise client\n");
		return EXIT_FAILURE;
	}

	clientMsg.hdr.type    = 0x00;
	clientMsg.hdr.subtype = 0x00;

	// Client started, can start sending messages
	int rcvid;
	my_message_t msg;
	TimerParam timerParam;

	if(timerInit(&timerParam, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}

	//printf("Timer started. Waiting for pulses...\n");

	int dataToSend = 0;
	while(1){
		rcvid = MsgReceive(timerParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}
		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			// can also check the pulse code here to make sure its from the timer
			if (msg.code == STATUS_TIMER) {
			    clientMsg.data = dataToSend;
			    clientMsg.state = (int)CurState;
			    if (MsgSend(clientParam.serverCoid, &clientMsg, sizeof(clientMsg), NULL, 0) == -1) {
			    	printf("Failed to send message\n");
			    	return EXIT_FAILURE;
			    }
			    dataToSend += 1;
			}
		}
	}

	clientCleanUp(&clientParam);
	timerCleanUp(&timerParam);
}

void *railSensor (void *data) { //rail sensor read thread
	int key;

	printf("Enter t to simulate train detection, n to clear sensor\n");
	fflush(stdout);

	while ((key = getchar()) != EOF){
		pthread_mutex_lock(&stateMutex);

		if (key == 't') {
			CurState = State1;
		} else if (key == 'n') {
			CurState = State0;
		}else if (CurState == State2){
			pthread_mutex_unlock(&stateMutex);
			break;
		}
		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

void *crossingOutput (void *data){ //crossing output thread
	if (CrossingOpen){
		if (trainSignalRed){
				printf("The train signal is red\n");
			} else {
				FAIL_TO_STATE2();
			}

		if (!gatesLowered) {
				printf("The gates are up\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (!redLight) {
				printf("Red lights are off\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (!crossingAlarm) {
				printf("The alarm is off\n");
			} else {
				FAIL_TO_STATE2();
			}
			printf("Waiting for train\n");

	}else if (!CrossingOpen){
		if (gatesLowered) {
			printf("Train detected\n");
				printf("The gates are down\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (redLight) {
				printf("Red lights are flashing\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (crossingAlarm) {
				printf("The alarm is on\n");
			} else {
				FAIL_TO_STATE2();
			}
			if (!trainSignalRed){
				printf("The train signal is green\n");
			} else {
				FAIL_TO_STATE2();
			}
	}
	return NULL;
}

int main(void) {

	pthread_t th1, th2, th3;

	if (pthread_create (&th2, NULL, communication, NULL) != 0) {
				perror("pthread_create");
				return 1;
			}

	if (pthread_create (&th1, NULL, railSensor, NULL) != 0) {
			perror("pthread_create");
			return 1;
	}

	enum states lastState;

	while(1) {
		enum states state;

		pthread_mutex_lock(&stateMutex);
		state = CurState;
		pthread_mutex_unlock(&stateMutex);

		if (state != lastState) {

			if (state == State0) {
				CrossingOpen = true;
				gatesLowered = false;
				//gatesLowered = true; //Simulate a component fail
				redLight = false;
				crossingAlarm = false;
				trainSignalRed = true;

			} else if (state == State1){
				CrossingOpen = false;
				gatesLowered = true;
				redLight = true;
				crossingAlarm = true;
				trainSignalRed = false;
			}else if (state == State2){
				break;
			}
		if (pthread_create (&th3, NULL, crossingOutput, NULL) != 0) {
				perror("pthread_create");
				return 1;
		}
		pthread_join (th3, NULL);
		lastState = state;
		}
	}

	pthread_join (th2, NULL);
	pthread_join (th1, NULL);
}
