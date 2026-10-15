#include "Input.h"

void sendEvent(int coid, int code){
	if(MsgSendPulse(coid, -1, code, 0) == -1){
		printf("Sending Message Failed");
	}
}

void *inputThread(void *arg){
	InputParam *input = (InputParam *) arg;
	char line[16];

	while(fgets(line, sizeof(line), stdin)){
		switch(line[0]){
		case 'n':
				sendEvent(input->coid, CAR_NS);
				break;
		case 'e':
				sendEvent(input->coid, CAR_EW);
				break;
		case 'N':
				sendEvent(input->coid, PED_NS);
				break;
		case 'E':
				sendEvent(input->coid, PED_EW);
				break;
		default:
			printf("\n");
			printf("=============== SENSOR CONTROLS ===============\n");
			printf("  n + Enter   Car arrives on NS road\n");
			printf("  e + Enter   Car arrives on EW road\n");
			printf("  N + Enter   NS pedestrian button\n");
			printf("  E + Enter   EW pedestrian button\n");
			printf("================================================\n");
		}
	}

	return NULL;
}
