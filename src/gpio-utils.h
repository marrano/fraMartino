/*
13/02/2001 
Stefano Burchietti
*/

/*On ARM based machines there is a bug in GCC, the type "char" is equivalent to "unsigned char". So I can't use char, I use int insted. (13/02/2011)*/

#include "gpio_g20.h"

const int _channels[28][3] = {
	{7,3,82},
	{7,4,83},
	{7,5,80},
	{7,6,81},
	{7,7,66},
	{7,8,67},
	{7,9,64},
	{7,10,65},
	{7,11,110},
	{7,12,111},
	{7,13,108},
	{7,14,109},
	{7,15,105},
	{7,19,101}, 
	{7,35,60},
	{7,36,59},
	{7,37,58},
	{7,38,57},

	{6,17,85},
	{6,18,84},
	{6,19,95},
	{6,20,94},
	{6,24,38},
	{6,25,39},
	{6,26,41},
	{6,36,42},
	{6,37,54},
	{6,38,43},
};

int detectKernelID(int port,int pin);

int bindInputChannel(int port,int pin);
int bindOutputChannel(int port,int pin);
int freeChannel(int port,int pin);

int setChannelState(int kernelID,int state);
int getChannelState(int kernelID);



int bindInputChannel(int port,int pin){
	int kernelID=detectKernelID(port,pin);
	if(kernelID!=-1)	{
		/*free channel before bind*/
		if (gpiounexport(kernelID)) return -1;
		if (gpioexport(kernelID)) return -1;
		if (gpiosetdir(kernelID,GPIO_IN)) return -1;
	}
	else
		fprintf(stderr,"\nError: channel J%d.%d unavailable.\n",port,pin);
	return kernelID;
}

int bindOutputChannel(int port,int pin){
	int kernelID=detectKernelID(port,pin);   
    	if(kernelID!=-1)	{
		/*free channel before bind*/
		if (gpiounexport(kernelID)) return -1;
		if (gpioexport(kernelID)) return -1;
		if (gpiosetdir(kernelID,GPIO_OUT)) return -1;
	}
	else
		fprintf(stderr,"\nError: channel J%d.%d unavailable.\n",port,pin);
	return kernelID;
}

int freeChannel(int port,int pin){
	int kernelID=detectKernelID(port,pin);   
	if(kernelID!=-1)	{
		if (gpiounexport(kernelID)) return -1;
	}
	else
		fprintf(stderr,"\nError: channel J%d.%d unavailable.\n",port,pin);
	return kernelID;
}

int setChannelState(int kernelID,int state)	{
	if(state)	{
		if (gpiosetbits(kernelID)) return -1;
	}
	else	{
		if (gpioclearbits(kernelID)) return -1;
	}
	return 0;
}

int getChannelState(int kernelID)	{	
	return gpiogetbits(kernelID);
}

int detectKernelID(int port,int pin) {
	int kernelID=-1;
	int i=0;
	while((kernelID==-1)&&(i<28))	{
		if((_channels[i][1]==pin)&&(_channels[i][0]==port))	{
			kernelID=_channels[i][2];
			/*printf("port: %d , pin: %d  => kernelID: %d \n",_channels[i][0],_channels[i][1],kernelID);*/
		}
		i++;
	}
	return kernelID;
}

