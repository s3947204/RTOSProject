#include <stdio.h>
#include <stdlib.h>
#include "Timer.h"
#include <sys/neutrino.h>
#include "Client.h"


int main(void)
{

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
		printf("Failed do initialise timer\n");
		return EXIT_FAILURE;
	}

	printf("Timer started. Waiting for pulses...\n");

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
