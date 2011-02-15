/*
8/02/2001 
Giovanni Burchietti
Stefano Burchietti
*/

/*
To compile:	gcc -s -Wall -Wstrict-prototypes fraMartino.c -o fraMartino
To run:		./fraMartino
To test daemon:	ps -ef|grep fraMartino (or ps -aux on BSD systems)
To test log:	tail -f /tmp/fraMartino.log
To test signal:	kill -HUP `cat /tmp/fraMartino.lock`
To terminate:	kill `cat /tmp/fraMartino.lock`
*/

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "gpio-utils.h"

#define RUNNING_DIR	"/tmp"
#define LOCK_FILE	"framartino.lock"
#define LOG_FILE	"framartino.log"

void playBell(int*,int*,unsigned char,int*);
void buzz(int*);
int wait(int,int*);
struct rtc_time setNextHalfHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextHourAlarm(struct rtc_time rtc_tm);

void daemonize();
void log_message(char*,char*);
void signal_handler(int);

/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */
static const char default_rtc[] = "/dev/rtc0";


int main(int argc, char **argv)	{
	/*variabili timer rtc */
	int fd, retval, irqcount = 0;
	unsigned long data;
	struct rtc_time rtc_tm;
	struct rtc_time ring_tm;
	const char *rtc = default_rtc;
	int i=0;
	int IDgreen=-1,IDbuzzer=-1,IDyellow=-1;	
	
	fprintf(stderr, "\n...fraMartino is started and run in background.");
	
	/* port J7.3 e J7.7 impostati come out */ 
	IDgreen=bindOutputChannel(7,3);
	IDyellow=bindOutputChannel(7,7);
	IDbuzzer=bindOutputChannel(7,4);

	
	if((IDgreen==-1)||(IDyellow==-1))	{
	  fprintf(stderr, "\nError: unable to access gpio resource. Exit.\n");
	  exit(-1);
	}
	
	daemonize();
	log_message(LOG_FILE,"*Started*");
	
	/* apre la comunicazione con il device RTC */
	fd = open(rtc, O_RDONLY);
	if (fd ==  -1) {
		perror(rtc);
		exit(errno);
	}
	
	/* Read the RTC time/date */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}
	fprintf(stderr, "\n\nCurrent RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	/*initialize alarm*/	
	/* allarme ore in punto  o 30 minuti*/
	if (rtc_tm.tm_min<30)   {
		ring_tm=setNextHalfHourAlarm(rtc_tm);
	}
	else    {
		ring_tm=setNextHourAlarm(rtc_tm);
	}

	while(1){
	    /*Set the alarm*/
	    retval = ioctl(fd, RTC_ALM_SET, &ring_tm);
	    if (retval == -1) {
		if (errno == ENOTTY) {
		    fprintf(stderr,"\n...Alarm IRQs not supported.\n");
		}
		perror("RTC_ALM_SET ioctl");
		exit(errno);
	    }

	    /* Read the current alarm settings */
	    retval = ioctl(fd, RTC_ALM_READ, &ring_tm);
	    if (retval == -1) {
		perror("RTC_ALM_READ ioctl");
		exit(errno);
	    }
	    fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n",ring_tm.tm_hour, ring_tm.tm_min, ring_tm.tm_sec);

	    fprintf(stderr, "Waiting for alarm...");
	    fflush(stderr);
	    
	    /* Enable alarm interrupts */
	    retval = ioctl(fd, RTC_AIE_ON, 0);
	    if (retval == -1) {
		perror("RTC_AIE_ON ioctl");
		exit(errno);
	    }

	    /* This blocks until the alarm ring causes an interrupt */
	    retval = read(fd, &data, sizeof(unsigned long));
	    if (retval == -1) {
		perror("read");
		exit(errno);
	    }
	    irqcount++;
	    fprintf(stderr, " okay! Alarm rang.\n");

	    /* Read the RTC time/date */
	    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	    if (retval == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	    }
	    if ((rtc_tm.tm_min == 00))  {
		/*play bells*/
		for (i=0;i<(ring_tm.tm_hour>12?ring_tm.tm_hour-12:ring_tm.tm_hour);i++) {
			playBell(&IDgreen,&IDbuzzer,2,&fd);	
			printf("\n %d\n",i+1);			
		}
		log_message(LOG_FILE,"Ora piena.");
		/*set next alarm*/
		ring_tm=setNextHalfHourAlarm(rtc_tm);
	    }
	    if ((rtc_tm.tm_min == 30))  {
		/*play bells*/
		playBell(&IDyellow,&IDbuzzer,2,&fd);	
		log_message(LOG_FILE,"Ora mezza.");
		/*set next alarm*/
		ring_tm=setNextHourAlarm(rtc_tm);
	    }
	    /* Disable alarm interrupts */
	    retval = ioctl(fd, RTC_AIE_OFF, 0);
	    if (retval == -1) {
		perror("RTC_AIE_OFF ioctl");
		exit(errno);
	    }
	}
	return 0;
}


int wait(int nSec,int* fd){
	int i,retval=0;
	unsigned long data;

	/* Turn on update interrupts (one per second) */
	retval = ioctl(*fd, RTC_UIE_ON, 0);
	if (retval == -1) {
		if (errno == ENOTTY) {
			fprintf(stderr,"\n...Update IRQs not supported.\n");
		}
		perror("RTC_UIE_ON ioctl");
		exit(errno);
	}

	for (i=1; i<nSec+1; i++) {
		/* This read will block */
		retval = read(*fd, &data, sizeof(unsigned long));
		if (retval == -1) {
			perror("read");
			exit(errno);
		}
	}

	/* Turn off update interrupts */
	retval = ioctl(*fd, RTC_UIE_OFF, 0);
	if (retval == -1) {
		perror("RTC_UIE_OFF ioctl");
		exit(errno);
	}
	return 0;
}

struct rtc_time setNextHalfHourAlarm(struct rtc_time rtc_tm){
	struct rtc_time ring_tm;
	ring_tm.tm_hour = rtc_tm.tm_hour;
	ring_tm.tm_min = 30;
	ring_tm.tm_sec = 0;
	return ring_tm;
}

struct rtc_time setNextHourAlarm(struct rtc_time rtc_tm){
	struct rtc_time ring_tm;
	if(rtc_tm.tm_hour<23)
	    ring_tm.tm_hour = rtc_tm.tm_hour+1;
	else
	    ring_tm.tm_hour = 0;
	ring_tm.tm_min = 00;
	ring_tm.tm_sec = 0;
	return ring_tm;
}

void buzz(int* IDbuzzer){
	int j,k,z=0; 
	for (j=0;j<20;j++){
		setChannelState(*IDbuzzer,1);
		setChannelState(*IDbuzzer,0);
		for (k=0;k<6400;k++){
		      z=1;
		      z=0;
		      }
	}
}
void playBell(int* IDbell,int* IDbuzzer,unsigned char delay,int *fd){
	printf ("on \n");
	setChannelState(*IDbell,1);
	/*wait(1,&fd);*/

	/*buzzo*/
	buzz(IDbuzzer);

	printf ("off \n");
	setChannelState(*IDbell,0);
	wait(delay,fd);
}

void daemonize()	{
	int i,lfp;
	char str[10];
	if(getppid()==1) return; /* already a daemon */
	i=fork();
	if (i<0) exit(1); /* fork error */
	if (i>0) exit(0); /* parent exits */
	/* child (daemon) continues */
	setsid(); /* obtain a new process group */
	for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
	i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
	umask(027); /* set newly created file permissions */
	chdir(RUNNING_DIR); /* change running directory */
	lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
	if (lfp<0) exit(1); /* can not open */
	if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
	/* first instance continues */
	sprintf(str,"%d\n",getpid());
	write(lfp,str,strlen(str)); /* record pid to lockfile */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */
}


void log_message(char* filename,char* message)	{
  	time_t rawtime;
	struct tm * timeinfo;
	char buffer [20];
	FILE *logfile;
	logfile=fopen(filename,"a");
	if(!logfile) return;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime (buffer,20,"%d/%m/%Y %H:%M",timeinfo);		
	
	fprintf(logfile,"%s \t%s\n",buffer,message);
	fclose(logfile);
}

void signal_handler(int sig)	{
	switch(sig) {
	case SIGHUP:
		log_message(LOG_FILE,"*Hangup signal catched*");
		break;
	case SIGTERM:
		log_message(LOG_FILE,"*Terminate signal catched*");
		exit(0);
		break;
	}
}