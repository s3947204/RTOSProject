#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

enum states {
	State0,
	State1,
	State2
};
enum states CurState = State0;

bool gatesLowered;
bool redLight;
bool crossingAlarm;
bool trainSignalRed;
bool CrossingOpen;

pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;


void *communication (void *data) {

	return NULL;
}

void *railSensor (void *data) {
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
			break;
		}
		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

void *crossingOutput (void *data){
	pthread_mutex_lock(&stateMutex);
	if (CrossingOpen){
		if (trainSignalRed){
					printf("The train signal is red\n");
			} else {
					printf("fail\n");
					CurState = State2;
			}

		if (!gatesLowered) {
				printf("The gates are up\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}

			if (!redLight) {
				printf("Red lights are off\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}

			if (!crossingAlarm) {
				printf("The alarm is off\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}
			printf("Waiting for train\n");

	}else if (!CrossingOpen){
		if (gatesLowered) {
			printf("Train detected\n");
				printf("The gates are down\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}

			if (redLight) {
				printf("Red lights are flashing\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}

			if (crossingAlarm) {
				printf("The alarm is on\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}
			if (!trainSignalRed){
				printf("The train signal is green\n");
			} else {
				printf("fail\n");
				CurState = State2;
			}
	}
	pthread_mutex_unlock(&stateMutex);
	return NULL;
}

int main(void) {

	pthread_t th1, th2, th3;

	if (pthread_create (&th1, NULL, railSensor, NULL) != 0) {
		perror("pthread_create");
		return 1;
	}

	if (pthread_create (&th2, NULL, communication, NULL) != 0) {
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
			printf("failure has occurred setting signal lights to red\n");
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
	return 0;
}
