#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define ATTACH_POINT "MainControllerServer"

/* We specify the header as being at least a pulse */
typedef struct _pulse msg_header_t;

/* Our real data comes after the header */
typedef struct _my_data {
    msg_header_t hdr;
    int          data;
} my_data_t;

/*** Server Side of the code ***/
int server()
{
	printf("MainController: Creating the server\n");
    name_attach_t *attach;
    my_data_t      msg;
    int            rcvid;

    /* Create a local name (/dev/name/local/...) */
    if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
    	printf("MainController: Creating a attach point failed \n");
        return EXIT_FAILURE;
    }

    /* Do your MsgReceive's here now with the chid */
    while (1) {
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

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

    /* Remove the name from the space */
    name_detach(attach, 0);
    return EXIT_SUCCESS;
}
