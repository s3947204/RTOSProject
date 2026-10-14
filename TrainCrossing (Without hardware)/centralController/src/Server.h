#ifndef SERVER_H
#define SERVER_H

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
    int			state;
} my_data_t;


typedef struct {
	name_attach_t *attach;
} ServerParam;

int serverInit(ServerParam *param);
void *serverRun(void *arg);
void serverCleanUp(ServerParam *param);

#endif
