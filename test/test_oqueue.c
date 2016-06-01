#include"test_oqueue.h"
#include<util.h>
#include<obliv.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

int main(int argc,char* argv[])
{ 
	ProtocolDesc pd;

	if(argc<3)
	{
		if(argc<2) fprintf(stderr,"Port number missing\n");
		else fprintf(stderr,"Party missing\n");
		fprintf(stderr,"Usage: %s <server|--> <port>\n",argv[0]);
		return 1;
	}

	int party = (strcmp(argv[1],"--")==0?1:2);

	connectOrDie(&pd,argv[1],argv[2]);

	setCurrentParty(&pd,party);

	srand(time(0));

	double lap = wallClock();
	fprintf(stderr,"Executing test: %s\n","oqueue");
	fprintf(stderr,"Role: %s\n", (party==1?"1/Server":"2/Client"));
	execYaoProtocol(&pd,testMain,0);
	fprintf(stderr,"Total time: %lf s\n",wallClock()-lap);
	cleanupProtocol(&pd);
	return 0;
}
