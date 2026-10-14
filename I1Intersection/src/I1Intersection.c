#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

void* EWpedestrian(void* data);
void* NSpedestrian(void* data);
void* EWtraffic(void* data);
void* NStraffic(void* data);
void* Output(void* data);
void* Input(void* data);


#define  MESSAGESIZE 2

#define Q_FLAGS O_RDWR | O_CREAT | O_EXCL
#define Q_Mode S_IRUSR | S_IWUSR

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
char *progname = "RTSproject_I1.c";

typedef union
{
	struct _pulse   pulse;
    // your other message structures would go here too
} my_message_t;

enum trafficStates { Green, Yellow, Red};
enum pedestrianStates { pedGreen, pedFlash, pedRed};
enum trafficPhases {Init, EW, NS};

typedef struct
{
	enum trafficStates trafficState;
	enum pedestrianStates pedestrianState;
	enum trafficPhases trafficPhase;

	int EWsensor;
	int NSsensor;
	int EWpedButton;
	int NSpedButton;

	pthread_rwlock_t sensorRWL;
	pthread_rwlock_t stateRWL;
	pthread_barrier_t initBarrier;
	pthread_barrier_t   pedBarrier;

	struct itimerspec itime;
	timer_t timer_id;
	my_message_t msg;
	int chid;

} app_data;


void InitTraffic(){
	printf ("EWR-NSR(0)\n");
	return;
}


void Traffic1(){
	printf ("EWG-NSR(1)\n");
	return;
}
void Traffic2(){
	printf ("EWY-NSR(2)\n");
	return;
}
void Traffic3(){
	printf ("EWR-NSR(3)\n");
	return;
}


void Traffic4(){
	printf ("EWR-NSG(4)\n");
	return;
}
void Traffic5(){
	printf ("EWR-NSY(5)\n");
	return;
}

void Traffic6(){
	printf ("EWR-NSR(6)\n");
	return;
}

void pedestrian1(){
	printf ("pedGreen\n");
	return;
}
void pedestrian2(){
	printf ("pedFlash\n");
	return;
}
void pedestrian3(){
	printf ("pedRed\n");
	return;
}



void *Input(void *data)
{
	printf("Input thread started....\n");
	app_data *td = (app_data*) data;
	pthread_barrier_wait(&td->initBarrier);
	char buffer[3];
	while(1)
	{
		usleep(1000);
		fgets(buffer, sizeof(buffer), stdin);
		if (buffer[0] == 'e'){
			pthread_rwlock_wrlock(&td->sensorRWL);
			td->EWsensor = 1;
			pthread_rwlock_unlock(&td->sensorRWL);
			printf("traffic signal: '%s'\n", buffer);
		}
		if (buffer[0] == 'n'){
			pthread_rwlock_wrlock(&td->sensorRWL);
			td->NSsensor = 1;
			pthread_rwlock_unlock(&td->sensorRWL);
			printf("traffic signal: '%s'\n", buffer);
		}
		if (buffer[0] == 'p' && buffer[1] == 'e'){
			pthread_rwlock_wrlock(&td->sensorRWL);
			td->EWpedButton = 1;
			pthread_rwlock_unlock(&td->sensorRWL);
			printf("pedestrian signal: '%s'\n", buffer);
		}
		if (buffer[0] == 'p' && buffer[1] == 'n'){
			pthread_rwlock_wrlock(&td->sensorRWL);
			td->NSpedButton = 1;
			pthread_rwlock_unlock(&td->sensorRWL);
			printf("pedestrian signal: '%s'\n", buffer);
		}

	}
}

void *Output(void *data)
{
	printf("Output thread started....\n");
	app_data *td = (app_data*) data;
	pthread_barrier_wait(&td->initBarrier);
	//char trafficOutput[];
	//char pedestrianOutput[];
	while(1)
	{
		//read lock
		//if(EWfinnished){
			//read unlock
		//}
		//else{
			//read unlock
		//}

		usleep(1000);

	}
}


void *EWtraffic(void* data) {

	app_data *td=(app_data*)data;
	pthread_t  th1;
	while(1){
		switch (td->trafficState)
		{
			case Green:

				Traffic1();

				timer_settime(td->timer_id, 0, &td->itime, NULL);

				while(1){
					pthread_rwlock_wrlock(&td->sensorRWL);
					td->EWsensor = 0;
					if (td->EWpedButton == 1){
						td->EWpedButton = 0;
						//printf("pedestrian thread started\n");
						pthread_create(&th1,  NULL, EWpedestrian, data);
						//pthread_barrier_init (&td->pedBarrier, NULL, 2);
					}

					if (td->NSsensor == 1 || td->NSpedButton == 1){
						MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);
						pthread_rwlock_unlock(&td->sensorRWL);
						td->trafficState = Yellow;
						break;
					}
					pthread_rwlock_unlock(&td->sensorRWL);
					usleep(1000);
				}
				break;

			case Yellow:
				Traffic2(); // code or function call
				timer_settime(td->timer_id, 0, &td->itime, NULL);
				MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);
				td->trafficState = Red;
				break;
			case Red:
				Traffic3(); // code or function call
				//pthread_barrier_wait(&td->pedBarrier);

				timer_settime(td->timer_id, 0, &td->itime, NULL);
				MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				td->trafficState = Green;

				return NULL;

		}

	}
}

void *NStraffic(void* data) {

	app_data *td=(app_data*)data;
	pthread_t  th1;
	while(1){
		switch (td->trafficState)
		{
			case Green:
				Traffic4();

				timer_settime(td->timer_id, 0, &td->itime, NULL);

				while(1){
					pthread_rwlock_wrlock(&td->sensorRWL);
					td->NSsensor = 0;
					if (td->NSpedButton == 1){
						td->NSpedButton = 0;
						//printf("pedestrian thread started\n");
						pthread_create(&th1,  NULL, NSpedestrian, data);
						//pthread_barrier_init (&td->pedBarrier, NULL, 2);
					}

					if (td->EWsensor == 1 || td->EWpedButton == 1){
						MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);
						pthread_rwlock_unlock(&td->sensorRWL);
						td->trafficState = Yellow;
						break;
					}
					pthread_rwlock_unlock(&td->sensorRWL);
					usleep(1000);
				}
				break;
			case Yellow:
				Traffic5(); // code or function call
				timer_settime(td->timer_id, 0, &td->itime, NULL);
				MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);
				td->trafficState = Red;
				break;

			case Red:
				Traffic6(); // code or function call
				timer_settime(td->timer_id, 0, &td->itime, NULL);
				MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);
				td->trafficState = Green;
				return NULL;

		}

	}
}

void *EWpedestrian(void* data) {
	app_data *td=(app_data*)data;
	pthread_detach( pthread_self() );
	td->pedestrianState = pedGreen;

	while(1){
		switch (td->pedestrianState)
		{
			case pedGreen:

				pedestrian1();

				//timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				td->pedestrianState = pedFlash;

				break;

			case pedFlash:
				pedestrian2();

			//	timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				td->pedestrianState = pedRed;
				break;

			case pedRed:
				pedestrian3();

			//	timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				//pthread_barrier_wait(&td->pedBarrier);
				//printf("ped end\n");
				return NULL;
		}

	}
}

void *NSpedestrian(void* data) {
	app_data *td=(app_data*)data;
	pthread_detach( pthread_self() );
	td->pedestrianState = pedGreen;

	while(1){
		switch (td->pedestrianState)
		{
			case pedGreen:

				pedestrian1();

				//timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				td->pedestrianState = pedFlash;

				break;

			case pedFlash:
				pedestrian2();

			//	timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				td->pedestrianState = pedRed;
				break;

			case pedRed:
				pedestrian3();

			//	timer_settime(td->timer_id, 0, &td->itime, NULL);
			//	td->rcvid = MsgReceive(td->chid, &td->msg, sizeof(td->msg), NULL);

				//pthread_barrier_wait(&td->pedBarrier);
				//printf("ped end\n");
				return NULL;
		}

	}
}


int main(int argc, char *argv[])
{

	printf("main thread started....\n");
	app_data data= {Green, pedRed, Init};

	struct sigevent event;

	data.chid = ChannelCreate(0); // Create a communications channel

	event.sigev_notify = SIGEV_PULSE;

	// create a connection back to ourselves for the timer to send the pulse on
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, data.chid, _NTO_SIDE_CHANNEL, 0);

	struct sched_param th_param;
	pthread_getschedparam(pthread_self(), NULL, &th_param);
	event.sigev_priority = th_param.sched_curpriority;    // old QNX660 version getprio(0);
	event.sigev_code = MY_PULSE_CODE;


	timer_create(CLOCK_REALTIME, &event, &data.timer_id);
	// setup the timer (1.5s initial delay value, 1.5s reload interval)
	data.itime.it_value.tv_sec = 1;			  // 1 second
	data.itime.it_value.tv_nsec = 0;    // 500 million nsecs = .5 secs
	data.itime.it_interval.tv_sec = 0;          // 1 second
	data.itime.it_interval.tv_nsec = 0; // 500 million nsecs = .5 secs

	pthread_barrier_init (&data.initBarrier, NULL, 2);
	pthread_t  th1, th2, th3, th4;
	void *retval;

	pthread_rwlock_init(&data.sensorRWL, NULL);

    pthread_create(&th1,  NULL, Input, &data);
    pthread_create(&th2,  NULL, Output, &data);

	while(1){
		switch (data.trafficPhase)
		{
			case Init:
				//Init(data);
				data.trafficPhase = EW;
				break;
			case EW:
				pthread_create(&th3,  NULL, EWtraffic, &data);
				pthread_join(th3, NULL);
				data.trafficPhase = NS;
				break;
			case NS:
				pthread_create(&th4,  NULL, NStraffic, &data);
				pthread_join(th4, NULL);
				data.trafficPhase = EW;
				break;
		}

	}


    pthread_join(th1, &retval);
    pthread_join(th2, &retval);


	pthread_rwlock_destroy(&data.sensorRWL);

	printf("\nMain Terminating....");

	return EXIT_SUCCESS;
}
