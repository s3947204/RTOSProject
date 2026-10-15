#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "Server.h"

int main(void){

    ServerParam param;
    pthread_t   serverThread;
    pthread_t   serverInputThread;

    if(serverInit(&param) == -1){
    	return EXIT_FAILURE;
    }
    pthread_create(&serverThread, NULL, serverRun, &param);
    pthread_create(&serverInputThread, NULL, serverConsole, &param);
    pthread_join(serverThread, NULL);
    serverCleanUp(&param);
    return EXIT_SUCCESS;

}

