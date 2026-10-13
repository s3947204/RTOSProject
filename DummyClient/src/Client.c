#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define ATTACH_POINT "MainControllerServer"

typedef struct _pulse msg_header_t;

/* Our real data comes after the header */
typedef struct _my_data {
    msg_header_t hdr;
    int          data;
} my_data_t;

int client()
{
	printf("Starting client \n");
    my_data_t msg;
    int       server_coid;

    if ((server_coid = name_open(ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL)) == -1) {
    	printf("Failed to connect to server\n");
        return EXIT_FAILURE;
    }

    /* We would have pre-defined data to stuff here */
    msg.hdr.type    = 0x00;
    msg.hdr.subtype = 0x00;

    /* Do whatever work you wanted with server connection */
    for (msg.data = 0; msg.data < 5; msg.data++) {
        printf("Client sending %d \n", msg.data);

        if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0) == -1) {
            break;
        }
    }

    /* Close the connection */
    name_close(server_coid);
    return EXIT_SUCCESS;
}
