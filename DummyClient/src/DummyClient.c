#include <stdio.h>
#include <stdlib.h>
#include "Timer.h"
#include <sys/neutrino.h>


int main(void)
{
//	int ret;
//	ret = client();
//  return ret;
	int rcvid;
	my_message_t msg;
	TimerParam param;

	if(timerInit(&param, 1) == 1){
		printf("Failed do initialise timer\n");
		return EXIT_FAILURE;
	}

	printf("Timer started. Waiting for pulses...\n");

	while(1){
		rcvid = MsgReceive(param.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}

		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			// can also check the pulse code here to make sure its from the timer
		    if (msg.code == STATUS_TIMER) {
				printf("Hello world!\n");
		    }
		}
	}

}
