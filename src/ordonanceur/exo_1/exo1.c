/**
 * Copyright 2018 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project:	HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose:	NanoPi silly status led control system
 *
 * Autĥor:	Daniel Gachet
 * Date:	07.11.2018
 * 
 * 
 * Modification : Pierrick Muller
 * Date :		  29.04.2022
 * sources : 
 * https://gist.github.com/ianpartridge/cb65a2bd79ba0746b9d68aa2afaed7f1
 */
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sched.h>

#define UNUSED(x) (void)(x)


const int NB_MESSAGE = 5;
static int signal_catched = 0;
static cpu_set_t set;

static void printerror(){
        char estr[100] = {[0]=0,};
        strerror_r(errno, estr, sizeof(estr)-1);
        fprintf (stderr, "ERROR: %s", estr);
}

static void parent(int socket){
	char buf[1024];
	while(strcmp(buf,"exit"))
	{
		read(socket,buf,sizeof(buf));
		printf("Parent (PID %ld) received : %s\n",(long)getpid(),buf);
	}
	
}

static void child(int socket){
	int i = 0;
	char msg[50],number[4],result[54];
	strcpy(msg,"Message number : ");
	for(i = 0; i < NB_MESSAGE;i++)
	{
		strcpy(result,msg);
		sprintf(number,"%d",i);
		strcat(result,number);
		printf("Child (PID %ld) send : %s\n",(long)getpid(),result);
		write(socket,result,sizeof(result));
		sleep(1);
	}
	printf("Child send : exit\n");
	write(socket,"exit",sizeof("exit"));

}

static void catch_signal(int signal)
{
    //syslog(LOG_INFO, "signal=%d catched\n", signal);
	printf("signal=%d catched\n",signal);
    signal_catched++;
}

int main(int argc, char* argv[])
{

	UNUSED(argc);
	UNUSED(argv);
	int i,fd[2],err,pid;
	static const int parentsocket = 0;
	static const int childsocket = 1;
	//cpu_set_t set;
	//1 catch all signals and show message when catched 
	struct sigaction act = {
        .sa_handler = catch_signal,
    };

	for (i=1; i <= 31; i++ )
	{
		sigaction(i,&act,NULL);
	}

	//2 call socket pair (https://stackoverflow.com/questions/11461106/socketpair-in-c-unix)
	err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	if (err == -1)
		printerror();


	CPU_ZERO(&set);
	//3 fork 
	pid = fork();
	if (pid == 0)
	{
		CPU_SET(1, &set);
		err = sched_setaffinity(0, sizeof(set), &set);
		close(fd[parentsocket]);	
		child(fd[childsocket]);
	}
	else if (pid > 0)
	{
		CPU_SET(0, &set);
		err = sched_setaffinity(0, sizeof(set), &set);
		close(fd[childsocket]);
		parent(fd[parentsocket]);
	}
	else
	{
		printerror();
	}
	//exit(0);

	return 0;
}