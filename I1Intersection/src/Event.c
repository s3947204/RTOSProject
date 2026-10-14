int eventInit(EventParam *param){
	param->chid = -1;
	param->coid = -1;
	param->timerCreated = 0;

	param->chid = ChannelCreate(0);
	if(param->chid == -1){
		perror("ChannelCreate");
		return -1;
	}

	// To attach to a channel, a node id, PID and channel id are needed
	// 0 for PID tells QNX to use the current process itself
	// Successful connection returns a Coid greater than or equal to 0
	// Connection ids(are used to send messages and pulses to the channel)
	// So in this case, the coid inside the event struct is set so the timer
	// can send a pulse or send some other message when the timer expires

	param->coid =  ConnectAttach(ND_LOCAL_NODE, 0, (param->chid), _NTO_SIDE_CHANNEL, 0);
	if (param->coid == -1)
	{
	   printf(stderr, "%s:  couldn't ConnectAttach to self!\n", progName);
	   perror(NULL);
	   return -1;
	}
}


void eventCleanUp(EventParam *param){
	if(param->coid != -1){
		ConnectDetach(param->coid);
		param->coid = -1;
	}

	if(param->chid != -1){
		ChannelDestroy(param->chid);
		param->chid = -1;
	}
}
