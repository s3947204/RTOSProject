#include "Server.h"

int serverInit(ServerParam *param){
	printf("MainController: Creating the server\n");
	param->attach = name_attach(NULL, ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL);

	if ( param->attach == NULL) {
		printf("MainController: Creating a attach point failed \n");
		return -1;
	}

	printf("MainController: Attach point created successfully\n");
	return 0;
}

void* serverRun(void *arg){
	my_data_t msg;
    ServerParam *param = (ServerParam *)arg;
	int rcvid;

    /* Do your MsgReceive's here now with the chid */
    while (1) {
        rcvid = MsgReceive(param->attach->chid, &msg, sizeof(msg), NULL);
        printf("server: rcvid=%d type=%d\n", rcvid, msg.hdr.type);
        fflush(stdout);

        if (rcvid == -1) {      /* Error condition, exit */
            break;
        }

        if (rcvid == 0) {       /* Pulse received */
            switch (msg.hdr.code) {
            case _PULSE_CODE_DISCONNECT:
                /*
                 * A client disconnected all its connections (called
                 * name_close() for each name_open() of our name) or
                 * terminated
                 */
                ConnectDetach(msg.hdr.scoid);
                break;

            case _PULSE_CODE_UNBLOCK:
                /*
                 * REPLY blocked client wants to unblock (was hit by
                 * a signal or timed out). It's up to you if you
                 * reply now or later.
                 */
                break;

            default:
                /*
                 * A pulse sent by one of your processes or a
                 * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
                 * from the kernel?
                 */
                break;
            }
            continue;
        }

        /*
         * If the Global Name Service (gns) is running, name_open()
         * sends a connect message. The server must EOK it.
         */
        if (msg.hdr.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        /* Some other I/O message was received; reject it */
        if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        /* A message (presumably ours) received, handle */
        printf("Server receive %d \n", msg.data);
        MsgReply(rcvid, EOK, 0, 0);
    }
    return NULL;
}


void serverCleanUp(ServerParam *param){
    name_detach(param->attach, 0);
}


