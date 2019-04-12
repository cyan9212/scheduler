#include "ku_cfs.h"

int endTime = 0;
float pow(float p,int n)
{
	if(n>0){
		float result = 1.0;
		while(n--)
			result *=p;
		return result;
	}
	else{
		float result = 1.0;
		while(n++)
			result*=1/p;
		return result;
	}
}
typedef struct process{
	pid_t pid;
	int nice;
	char work;
	float vruntime;
	struct process *nextProcess;
}Process;

Process *HeadProcess;
Process *CurrentProcess;

void addProcess(){
	Process *newProcess = (Process*)malloc(sizeof(Process));
	CurrentProcess->nextProcess = newProcess;
	newProcess->nextProcess = NULL;
	CurrentProcess = newProcess;
}

void freeProcess(){
	Process *head = HeadProcess;
	Process *tmp;
	while(CurrentProcess!=NULL){
		tmp = CurrentProcess->nextProcess;
		free(CurrentProcess);
		CurrentProcess = tmp;
	}
}
void handler(){
	kill(CurrentProcess->pid,SIGSTOP);
	CurrentProcess = CurrentProcess->nextProcess;
	if(CurrentProcess == NULL)
		CurrentProcess = HeadProcess->nextProcess;
	endTime++;
}

int main(int argc,char* argv[])
{
	int i,j;
	int parentPid = getpid();
	int timeslice = atoi(argv[6]);
	ttt = timeslice;
	int processSum = 0;
	int processCnt = 0;
	int niceArr[26]; // A to Z
	int nicePointer = 0;
	int nice[5] = {-2,-1,0,1,2};
	FILE *file = fopen("output.txt","a");
	if(argc==7)
		fclose(file);
	char works[26];
	works[0] = 'A';
	for(i=1;i<26;i++){
		works[i] = works[i-1] + 1;
	}
	for(i=1;i<=5;i++){
		processSum +=atoi(argv[i]);
	}
	for(i=1;i<=5;i++){
		int tmp = atoi(argv[i]);
		for(j=0;j<tmp;j++){
			niceArr[nicePointer] = nice[i-1];
			nicePointer++;
		}
	}
	HeadProcess = (Process*)malloc(sizeof(Process));
	HeadProcess->nextProcess = NULL;
	CurrentProcess = HeadProcess;
	for(i=0;i<processSum;i++){
		addProcess();
		CurrentProcess->pid = fork();
		if(CurrentProcess->pid <0){
			printf("fork failed\n");
			exit(0);
		}
		if(CurrentProcess->pid == 0){ //child process

			CurrentProcess->pid = getpid();
			CurrentProcess->work = works[processCnt];
			CurrentProcess->nice = niceArr[processCnt];
			CurrentProcess->vruntime += pow(1.25,CurrentProcess->nice);
			break;
		}
		CurrentProcess->work = works[processCnt];
		CurrentProcess->nice = niceArr[processCnt];
		CurrentProcess->vruntime += pow(1.25,CurrentProcess->nice);
		processCnt++;
	}

	if(getpid() != parentPid){
		char *args[] = {"./ku_app",&CurrentProcess->work,NULL};
		execvp("./ku_app",args);
	}
	sleep(5);
	struct itimerval set_timer;
	CurrentProcess = HeadProcess->nextProcess;
	sigset(SIGALRM,handler);
	set_timer.it_value.tv_sec = 1;
	set_timer.it_value.tv_usec = 0;
	set_timer.it_interval.tv_sec = 1;
	set_timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL,&set_timer,NULL);
	while(1){
		kill(CurrentProcess->pid,SIGCONT);
		pause();
		if(endTime == timeslice)
			break;
	}
	if(argc!=7)
		fclose(file);
	freeProcess();
	exit(0);
}
