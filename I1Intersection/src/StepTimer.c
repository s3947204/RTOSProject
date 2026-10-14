#include "StepTimer.h"
char *progName = "StepTimer.c";

int timerInit(TimerParam *param, int intervalSec){
	struct sigevent event;
	struct itimerspec itime;

	param->chid = -1;
	param->coid = -1;
	param->timerCreated = 0;

	param->chid = ChannelCreate(0);
	if(param->chid == -1){
		perror("ChannelCreate");
		return -1;
	}

	// To attach to a channel, a node id, PID and channel id are needed
	// 0 for PID tells QNX to use the current process itself
	// Successful connection returns a Coid greater than or equal to 0
	// Connection ids(are used to send messages and pulses to the channel)
	// So in this case, the coid inside the event struct is set so the timer
	// can send a pulse or send some other message when the timer expires

	param->coid =  ConnectAttach(ND_LOCAL_NODE, 0, (param->chid), _NTO_SIDE_CHANNEL, 0);
	if (param->coid == -1)
	{
	   printf(stderr, "%s:  couldn't ConnectAttach to self!\n", progName);
	   perror(NULL);
	   return -1;
	}

	struct sched_param th_param;
	pthread_getschedparam(pthread_self(), NULL, &th_param);
	event.sigev_priority = th_param.sched_curpriority;
	event.sigev_code = STEP_TIMER;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = param->coid;

    itime.it_value.tv_sec = intervalSec;
    itime.it_value.tv_nsec = 0;
    itime.it_interval.tv_sec = intervalSec;
    itime.it_interval.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &event, &param->timerId) == -1)
	{
	   printf (stderr, "%s:  couldn't create a timer, errno %d\n", progName, errno);
	   perror (NULL);
	   return -1;
	}

    param->timerCreated = 1;

	timer_settime(param->timerId, 0, &itime, NULL);
	return 0;
}

void timerCleanUp(TimerParam *param){
	if(param->timerCreated){
		timer_delete(param->timerId);
		param->timerCreated = 0;
	}

	if(param->coid != -1){
		ConnectDetach(param->coid);
		param->coid = -1;
	}

	if(param->chid != -1){
		ChannelDestroy(param->chid);
		param->chid = -1;
	}
}
