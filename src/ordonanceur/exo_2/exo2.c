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

const int NB_BYTES = 1048576;

int main(int argc, char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	int i,err;
	char * temp_val[50];
	//err = execl("/bin/echo","echo","$$",">","/sys/fs/cgroup/memory/mem/tasks");
	for (i = 0; i < 50; i++)
	{
		temp_val[i] = (char *) malloc(NB_BYTES);
		memset(temp_val[i],'0',NB_BYTES);
		//if(calloc(NB_BYTES,1) != NULL)
		if(temp_val[i] != NULL)
		{
			printf("Allocated %d Mébibytes\n",i + 1);
			sleep(1);
		}
		else 
		{
			printf("Aie ! ");
		}
	}
	for(i = 0; i < 50; i++)
	{
		free(temp_val[i]);
	}

	return 0;
}