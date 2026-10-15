#include "StepTimer.h"
#include "Event.h"
char *progNameTimer = "StepTimer.c";

int timerInit(TimerParam *param, int coid, int intervalSec){
	param->timerCreated = 0;

	struct sigevent event;
	struct itimerspec itime;
	struct sched_param th_param;
	pthread_getschedparam(pthread_self(), NULL, &th_param);
	event.sigev_priority = th_param.sched_curpriority;
	event.sigev_code = STEP_TIMER;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = coid;

    itime.it_value.tv_sec = intervalSec;
    itime.it_value.tv_nsec = 0;
    itime.it_interval.tv_sec = intervalSec;
    itime.it_interval.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &event, &param->timerId) == -1)
	{
	   printf (stderr, "%s:  couldn't create a timer, errno %d\n", progNameTimer, errno);
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
}
