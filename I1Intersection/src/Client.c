#include "Client.h"

int clientInit(ClientParam *param){
	printf("Starting client \n");
	param->mainChid = -1;
	param->mainCoid = -1;
	param->serverCoid = -1;

	// we will create another channel to get a message from main thread
	// that will then connect to the global channel and send status and receive command
	param->mainChid = ChannelCreate(0);
	if(param->mainChid == -1){
		perror("ChannelCreate");
		return -1;
	}

	param->mainCoid =  ConnectAttach(ND_LOCAL_NODE, 0, (param->mainChid), _NTO_SIDE_CHANNEL, 0);
	if (param->mainCoid == -1)
	{
	   perror(NULL);
	   return -1;
	}

	printf("Connecting to server\n");
	param->serverCoid = name_open(ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL);
	if(param->serverCoid == -1){
	  	printf("Failed to connect to server\n");
		return -1;
	}
	return 0;
}

void clientCleanUp(ClientParam *param){
	if(param->serverCoid != -1){
	    name_close(param->serverCoid);
	    param->serverCoid = -1;
	}

	if(param->mainCoid != -1){
		ConnectDetach(param->mainCoid);
		param->mainCoid = -1;
	}

	if(param->mainChid != -1){
		ChannelDestroy(param->mainChid);
		param->mainChid = -1;
	}
}

void *clientRun(void *arg){
	ClientParam *param = (ClientParam *)arg;
	my_message_t pulseMsg;
	my_data_t msg;
	int count = 0;

	while(1){
		int rcvid = MsgReceive(param->mainChid, &pulseMsg, sizeof(pulseMsg), NULL);
		if(rcvid == 0 && pulseMsg.code == STATUS_DUE){
			memset(&msg, 0, sizeof(msg));
			msg.hdr.type = 0x00;
			msg.hdr.subtype = 0x00;
			msg.data = count++;
	        printf("Client sending %d \n", msg.data);
	        if (MsgSend(param->serverCoid, &msg, sizeof(msg), NULL, 0) == -1) {
	        	printf("Error sending message to server \n");
	        }
		}
	}

	return NULL;
}

