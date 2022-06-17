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
 * Modification : Pierrick Muller
 * Date :		  29.04.2022
 * sources : 
 * https://gist.github.com/ianpartridge/cb65a2bd79ba0746b9d68aa2afaed7f1
 */
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
#include <syslog.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

void print_help(){
	printf("Bad options, should be either : \n");
	printf("--set-freq VALUE or\n");
	printf("--set-mode VALUE or\n");
	printf("--stop\n");
}

int main(int argc, char* argv[])
{
	int sock = 0, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };

	if(argc > 1)
	{

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        	printf("\n Socket creation error \n");
        	return -1;
    	}
		
		serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(PORT);
 
    	// Convert IPv4 and IPv6 addresses from text to binary
    	// form
    	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        	printf("\nInvalid address/ Address not supported \n");
        	return -1;
    	}
 
    	if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr,sizeof(serv_addr)))< 0) {
        	printf("\nConnection Failed \n");
        	return -1;
    	}

		if(strcmp(argv[1],"--set-freq") == 0){
			if(argc > 2)
			{
				sprintf(buffer,"set-freq");
				send(sock,buffer,sizeof(buffer),0);
				send(sock,argv[2],sizeof(argv[2]),0);
			}
			else{
				print_help();
			}
		}
		else if(strcmp(argv[1],"--set-mode") == 0){
			if(argc > 2)
			{
				sprintf(buffer,"set-mode");
				send(sock,buffer,sizeof(buffer),0);
				send(sock,argv[2],sizeof(argv[2]),0);
			}
			else
			{
				print_help();
			}
		}
		else if(strcmp(argv[1],"--stop") == 0){
			sprintf(buffer,"stop");
			send(sock,buffer,sizeof(buffer),0);
		}
		else{
			print_help();
		}
	}
	else
	{
		print_help();
	}

	close(client_fd);
	return 0;
}