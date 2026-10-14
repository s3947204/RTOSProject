#include "Client.h"

int clientInit(ClientParam *param){
	printf("Starting client \n");
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
}

int client()
{
	my_data_t msg;
    int       server_coid;

    if ((server_coid = name_open(ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL)) == -1) {
    }

    /* We would have pre-defined data to stuff here */

    /* Do whatever work you wanted with server connection */
    for (msg.crossingHeartbeat = 0; msg.crossingHeartbeat < 5; msg.crossingHeartbeat++) {
        printf("Client sending %d \n", msg.crossingHeartbeat);

        if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0) == -1) {
            break;
        }
    }

    /* Close the connection */
    name_close(server_coid);
    return EXIT_SUCCESS;
}
