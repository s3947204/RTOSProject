#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *server(void *arg);

int main(void)
{
    pthread_t server_thread;

    pthread_create(&server_thread, NULL, server, NULL);

    pthread_join(server_thread, NULL);

    return 0;
}
