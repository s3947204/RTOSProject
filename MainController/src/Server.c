#include "Server.h"

#define NUM_PHASES 6
char *pedOutputString[] = { "RED", "GREEN", "FLASHING" };
char *modeOutputSring[] = { "Cycling", "Sensor" };

phase cycle[NUM_PHASES] = {
	{ "NSG-EWR", 10, DIR_EW, DIR_NS, TYPE_GREEN}, // hold until EW sensor goes off
	{ "NSY-EWR", 3, DIR_NONE, DIR_NS, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_NS, TYPE_ALLRED},
	{ "NSR-EWG", 10, DIR_NS, DIR_EW, TYPE_GREEN}, // hold until NS sensor goes off
	{ "NSR-EWY", 3, DIR_NONE, DIR_EW, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_EW, TYPE_ALLRED}
};

int serverInit(ServerParam *param){
	printf("MainController: Creating the server\n");
	param->attach = name_attach(NULL, ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL);

    param->selfCoid = -1;
    param->togglePending = 0;

	if ( param->attach == NULL) {
		printf("MainController: Creating a attach point failed \n");
		return -1;
	}

	param->selfCoid = ConnectAttach(ND_LOCAL_NODE, 0, param->attach->chid, _NTO_SIDE_CHANNEL, 0);
	if(param->selfCoid == -1){
		perror("ConnectAttach");
		name_detach(param->attach, 0);
		return -1;
	}

	printf("MainController: Attach point created successfully\n");
	return 0;
}

void* serverRun(void *arg){
	traffic_status_t msg;
    ServerParam *param = (ServerParam *)arg;
	int rcvid;

    /* Do your MsgReceive's here now with the chid */
    while (1) {
        rcvid = MsgReceive(param->attach->chid, &msg, sizeof(msg), NULL);

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
            case TOGGLE_PULSE:
            	param->togglePending = 1;
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

        // Id = 1 - intersection 1
        // Id = 2 - intersection 2
        // Id = 3 - train intersection
        // Message is from intersection 1
        if(msg.id == 1){
            printf("I1: Mode: %s | Phase: %s | Elapsed: %ds | NS Ped: %s | EW Ped: %s | NS Car Sensor: %d | EW Car Sensor: %d | NS Ped Req: %d | EW Ped Req: %d\n",
            		modeOutputSring[msg.mode],
    				cycle[msg.phase].lights, msg.elapsed, pedOutputString[msg.nsPed], pedOutputString[msg.ewPed],
    				msg.nsCar, msg.ewCar, msg.nsPedReq, msg.ewPedReq);

            reply_t reply = { CMD_NONE };
            if (param->togglePending == 1){
            	param->togglePending = 0;
            	reply.cmd = MODE_TOGGLE;
            }
            MsgReply(rcvid, EOK, &reply, sizeof(reply));
        }
    }
    return NULL;
}

void *serverConsole(void *arg){
    ServerParam *param = (ServerParam *)arg;
    char line[16];

    printf("Press t + Enter to toggle the mode of intersection 1\n");
    while(fgets(line, sizeof(line), stdin)){
        if(line[0] == 't'){
            MsgSendPulse(param->selfCoid, -1, TOGGLE_PULSE, 0);
        } else {
            printf("Unknown command '%c' (t = toggle mode)\n", line[0]);
        }
    }
    return NULL;
}


void serverCleanUp(ServerParam *param){
    name_detach(param->attach, 0);
}


