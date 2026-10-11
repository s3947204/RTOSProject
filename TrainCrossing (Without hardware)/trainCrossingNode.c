#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

enum states {
	State0,
	State1,
	State2
};
enum states CurState = State0;

bool Done = false;
bool gatesLowered;
bool redLight;
bool crossingAlarm;
bool trainSignalRed;

pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

void crossingOpen();
void crossingClosed();

void *communication (void *data){

	return NULL;
}

void *railSensor (void *data){
	int key;

	printf("Enter t to simulate train detection, n to clear sensor\n");
	fflush(stdout);

	while ((key = getchar()) != EOF){
		pthread_mutex_lock(&stateMutex);

		if (key == 't') {
			CurState = State1;
		} else if (key == 'n'){
			CurState = State0;
		}
		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

void *crossingOutput (void *data){
	pthread_mutex_lock(&stateMutex);
	if (gatesLowered) {
		printf("The gates are down\n");
	}else{
		printf("The gates are up\n");
	}

	if (redLight) {
		printf("Red lights are flashing\n");
	}else{
		printf("Red lights are off\n");
	}

	if (crossingAlarm){
		printf("The alarm is on\n");
	}else{
		printf("The alarm is off\n");
	}
	pthread_mutex_unlock(&stateMutex);
	return NULL;
}

void *trainSignal (void *data){
	pthread_mutex_lock(&stateMutex);
	if (trainSignalRed){
			printf("The train signal is red\n");
	}else{
			printf("The train signal is green\n");
	}
	pthread_mutex_unlock(&stateMutex);
	return NULL;
}

int main(void){

	pthread_t th1, th2, th3, th4;

	if (pthread_create (&th1, NULL, railSensor, NULL) != 0) {
		perror("pthread_create");
		return 1;
	}

	if (pthread_create (&th4, NULL, communication, NULL) != 0) {
			perror("pthread_create");
			return 1;
		}

	enum states lastState = State2;

	while(1) {
		enum states state;
		bool done;

	pthread_mutex_lock(&stateMutex);
	state = CurState;
	done = Done;
	pthread_mutex_unlock(&stateMutex);

	if (done) {
		break;
	}

	if (state != lastState){
		if (state == State0){
			crossingOpen();
			if (pthread_create (&th3, NULL, trainSignal, NULL) != 0) {
						perror("pthread_create");
						return 1;
			}

			pthread_join (th3, NULL);

			if (pthread_create (&th2, NULL, crossingOutput, NULL) != 0) {
						perror("pthread_create");
						return 1;
			}

			pthread_join (th2, NULL);
			printf("Waiting for train\n");
		} else if (state == State1){
			crossingClosed();
			printf("Train detected\n");

			if (pthread_create (&th2, NULL, crossingOutput, NULL) != 0) {
						perror("pthread_create");
						return 1;
			}

			pthread_join (th2, NULL);

			if (pthread_create (&th3, NULL, trainSignal, NULL) != 0) {
						perror("pthread_create");
						return 1;
			}

			pthread_join (th3, NULL);
		}
		fflush(stdout);
		lastState = state;
		}
	}

	pthread_join (th1, NULL);
	pthread_join (th4, NULL);
	return 0;
}

void crossingOpen(void) {
	gatesLowered = false;
	redLight = false;
	crossingAlarm = false;
	trainSignalRed = true;
}

void crossingClosed(void) {
	gatesLowered = true;
	redLight = true;
	crossingAlarm = true;
	trainSignalRed = false;
}
