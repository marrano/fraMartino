#ifndef GPIO_G20_H_INCLUDED
#define GPIO_G20_H_INCLUDED

#define GPIO_OUT           0
#define GPIO_IN            1


int gpioexport(int gpioid)
{
	FILE *filestream;

	if ((filestream=fopen("/sys/class/gpio/export","w"))==NULL) {
		printf("Error on export GPIO\n");
		return -1;
	}
	fprintf(filestream,"%d",gpioid);
	fclose(filestream);
	return 0;
}

int gpiounexport(int gpioid)
{
	FILE *filestream;

	if ((filestream=fopen("/sys/class/gpio/unexport","w"))==NULL) {
		printf("Error on unexport GPIO\n");
		return -1;
	}
	fprintf(filestream,"%d",gpioid);
	fclose(filestream);
	return 0;
}


int gpiosetdir(int gpioid,int mode)
{
	FILE *filestream;
	char filename[50];

	sprintf(filename,"/sys/class/gpio/gpio%d/direction",gpioid);
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on direction setup\n");
		return -1 ;
	}

	if (mode == GPIO_IN) {
		fprintf(filestream,"in");
	} else {
		fprintf(filestream,"out");
	}
	fclose(filestream);
	return 0;
}

int gpiogetbits(int gpioid)
{
	FILE *filestream;
	char filename[50];
	char retchar;

	sprintf(filename,"/sys/class/gpio/gpio%d/value",gpioid);
	if ((filestream=fopen(filename,"r"))==NULL) {
		printf("Error on gpiogetbits %d\n",gpioid);
		return -1;
	}
	retchar=fgetc(filestream);
	fclose(filestream);
	if (retchar=='0') return 0;
	else return 1;
}

int gpiosetbits(int gpioid)
{
	FILE *filestream;
	char filename[50];

	sprintf(filename,"/sys/class/gpio/gpio%d/value",gpioid);
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on setbits %d\n",gpioid);
		return -1;
	}
	fprintf(filestream,"1");
	fclose(filestream);
	return 0;
}

int gpioclearbits(int gpioid)
{
	FILE *filestream;
	char filename[50];

	sprintf(filename,"/sys/class/gpio/gpio%d/value",gpioid);
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on clearbits %d\n",gpioid);
		return -1;
	}
	fprintf(filestream,"0");
	fclose(filestream);
	return 0;
}


#endif // GPIO_G20_H_INCLUDED
