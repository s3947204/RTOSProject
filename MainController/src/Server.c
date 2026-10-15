#include "Server.h"

char *pedOutputString[] = { "RED", "GREEN", "FLASHING" };
char *modeOutputSring[] = { "Cycling", "Sensor" };

phase cycleIntersectionOne[6] = {
	{ "NSG-EWR", 10, DIR_EW, DIR_NS, TYPE_GREEN}, // hold until EW sensor goes off
	{ "NSY-EWR", 3, DIR_NONE, DIR_NS, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_NS, TYPE_ALLRED},
	{ "NSR-EWG", 10, DIR_NS, DIR_EW, TYPE_GREEN}, // hold until NS sensor goes off
	{ "NSR-EWY", 3, DIR_NONE, DIR_EW, TYPE_YELLOW},
	{ "NSR-EWR", 1, DIR_NONE, DIR_EW, TYPE_ALLRED}
};

phase cycleIntersectionTwo[12] = {
    { "NSG-EWR",   10, DIR_EW,   DIR_NS, TYPE_GREEN  },
    { "NSY-EWR",    3, DIR_NONE, DIR_NS, TYPE_YELLOW },
    { "NSR-EWR",    1, DIR_NONE, DIR_NS, TYPE_ALLRED },

    { "NSRR-EWRG",  6, DIR_NONE, DIR_NONE, TYPE_ALLRED  },
    { "NSRR-EWRY",  3, DIR_NONE, DIR_NONE, TYPE_ALLRED  },
    { "NSRR-EWRR",  1, DIR_NONE, DIR_NONE, TYPE_ALLRED  },

    { "NSR-EWG",   10, DIR_NS,   DIR_EW,   TYPE_GREEN  },
    { "NSR-EWY",    3, DIR_NONE, DIR_EW,   TYPE_YELLOW },
    { "NSR-EWR",    1, DIR_NONE, DIR_EW,   TYPE_ALLRED },

    { "NSRG-EWRR",  6, DIR_NONE, DIR_NONE, TYPE_ALLRED  },
    { "NSRY-EWRR",  3, DIR_NONE, DIR_NONE, TYPE_ALLRED  },
    { "NSRR-EWRR",  1, DIR_NONE, DIR_NONE, TYPE_ALLRED  }
};
int serverInit(ServerParam *param){
	printf("MainController: Creating the server\n");
	param->attach = name_attach(NULL, ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL);

    param->selfCoid = -1;
    param->togglePending[0] = 0;
    param->togglePending[1] = 0;

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
	union {
		node_message_t header;
		traffic_status_t traffic;
		crossing_status_t crossing;
	} msg;

	ServerParam *param = (ServerParam *)arg;
	int rcvid;

    /* Do your MsgReceive's here now with the chid */
    while (1) {
        rcvid = MsgReceive(param->attach->chid, &msg, sizeof(msg), NULL);

        if (rcvid == -1) {      /* Error condition, exit */
            break;
        }

        if (rcvid == 0) {       /* Pulse received */
            switch (msg.header.hdr.code) {
            case _PULSE_CODE_DISCONNECT:
                /*
                 * A client disconnected all its connections (called
                 * name_close() for each name_open() of our name) or
                 * terminated
                 */
                ConnectDetach(msg.header.hdr.scoid);
                break;

            case _PULSE_CODE_UNBLOCK:
                /*
                 * REPLY blocked client wants to unblock (was hit by
                 * a signal or timed out). It's up to you if you
                 * reply now or later.
                 */
                break;
            case TOGGLE_PULSE_INT1:
            	param->togglePending[0] = 1;
            	break;
            case TOGGLE_PULSE_INT2:
            	param->togglePending[1] = 1;
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
        if (msg.header.hdr.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        /* Some other I/O message was received; reject it */
        if (msg.header.hdr.type > _IO_BASE && msg.header.hdr.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        /* A message (presumably ours) received, handle */

        // Id = 1 - intersection 1
        // Id = 2 - intersection 2
        // Id = 3 - train intersection
        // Message is from intersection 1
        if(msg.header.id == 1){
            traffic_status_t *s = &msg.traffic;


            printf("I1: Mode: %s | Phase: %s | Elapsed: %ds | NS Ped: %s | EW Ped: %s | NS Car Sensor: %d | EW Car Sensor: %d | NS Ped Req: %d | EW Ped Req: %d\n",
            		modeOutputSring[s->mode],
					cycleIntersectionOne[s->phase].lights, s->elapsed, pedOutputString[s->nsPed], pedOutputString[s->ewPed],
    				s->nsCar, s->ewCar, s->nsPedReq, s->ewPedReq);

            reply_t reply = { CMD_NONE };
            if (param->togglePending[0] == 1){
            	param->togglePending[0] = 0;
            	reply.cmd = MODE_TOGGLE;
            }

            MsgReply(rcvid, EOK, &reply, sizeof(reply));
        }else if(msg.header.id == 2){
            traffic_status_t *s = &msg.traffic;
            printf("I2: Mode: %s | Phase: %s | Elapsed: %ds | NS Ped: %s | EW Ped: %s | NS Car Sensor: %d | EW Car Sensor: %d | NS Ped Req: %d | EW Ped Req: %d\n",
            		modeOutputSring[s->mode],
					cycleIntersectionTwo[s->phase].lights, s->elapsed, pedOutputString[s->nsPed], pedOutputString[s->ewPed],
    				s->nsCar, s->ewCar, s->nsPedReq, s->ewPedReq);
            reply_t reply = { CMD_NONE };
            if (param->togglePending[1] == 1){
				param->togglePending[1] = 0;
				reply.cmd = MODE_TOGGLE;
			}
            MsgReply(rcvid, EOK, &reply, sizeof(reply));
        }else if(msg.header.id == 3){
        	crossing_status_t *s = &msg.crossing;

			printf("X1: Heartbeat: %d | State: %d\n",
				   s->crossingHeartbeat,
				   s->crossingState);
	        MsgReply(rcvid, EOK, NULL, 0);
        }else{
        	printf("Message received from unknown node\n");
        	MsgReply(rcvid, EOK, NULL, 0);
        }
    }
    return NULL;
}

void *serverConsole(void *arg){
    ServerParam *param = (ServerParam *)arg;
    char line[16];

    printf("Press t + Enter to toggle the mode of intersection 1\n");
    printf("Press T + Enter to toggle the mode of intersection 2\n");
    while(fgets(line, sizeof(line), stdin)){
        if(line[0] == 't'){
            MsgSendPulse(param->selfCoid, -1, TOGGLE_PULSE_INT1, 0);
        }else if(line[0] == 'T'){
        	MsgSendPulse(param->selfCoid, -1, TOGGLE_PULSE_INT2, 0);
        }else {
            printf("Unknown command '%c' (t = toggle mode)\n", line[0]);
        }
    }
    return NULL;
}


void serverCleanUp(ServerParam *param){
    name_detach(param->attach, 0);
}


