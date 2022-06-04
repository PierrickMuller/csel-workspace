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


/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT		"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_LED		"/sys/class/gpio/gpio10"
#define GPIO_k1			"/sys/class/gpio/gpio0"
#define GPIO_k2			"/sys/class/gpio/gpio2"
#define GPIO_k3			"/sys/class/gpio/gpio3"
//#define GPIO_GENERIC(number,directory)	"/sys/class/gpio/gpio" #number "/" #directory
#define LED				"10"
#define K1				"0"
#define K2				"2"
#define K3				"3"

void printerror(){
	char estr[100] = {[0]=0,};
	strerror_r(errno, estr, sizeof(estr)-1);
	fprintf (stderr, "ERROR: %s", estr);
}

static int open_led()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, LED, strlen(LED));
	int ret = close (f);
	if (ret == -1)
	{
		printerror();
	}

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, LED, strlen(LED));
	ret = close (f);
	if (ret == -1)
	{
		printerror();
	}

	// config pin
	f = open (GPIO_LED "/direction", O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, "out", 3);
	ret = close (f);
	if (ret == -1)
	{
		printerror();
	}

	// open gpio value attribute
 	f = open (GPIO_LED "/value", O_RDWR);
	if (f == -1)
	{
		printerror();
	}
	return f;
}

static int open_button_k1()
{

	//unexport
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, K1, strlen(K1));
	int ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	//export pin to sysfsa
	f = open(GPIO_EXPORT, O_WRONLY);
	if(f == -1)
	{
		printerror();
	}
	write (f, K1, strlen(K1));
	ret = close (f); 
	if(ret == -1)
	{
		printerror();
	}

	//config pin 
	f = open(GPIO_k1 "/direction",O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, "in", 3);
	ret = close(f);
	if (ret == -1)
	{
		printerror();
	}

	//falling mode
	f = open(GPIO_k1 "/edge",O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f,"falling",7);
	ret = close(f);
	if (ret == -1)
	{
		printerror();
	}

	f = open(GPIO_k1 "/value", O_RDONLY);
	if(f == -1)
	{
		printerror();
	}
	return f;
}

static int open_button_k2()
{

	//unexport
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, K2, strlen(K2));
	int ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	//export pin to sysfsa
	f = open(GPIO_EXPORT, O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, K2, strlen(K2));
	ret = close (f); 
	if(ret == -1)
	{
		printerror();
	}

	//config pin 
	f = open(GPIO_k2 "/direction",O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f, "in", 3);
	ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	//falling mode
	f = open(GPIO_k2 "/edge",O_WRONLY);
	if (f == -1)
	{
		printerror();
	}
	write (f,"falling",7);
	ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	f = open(GPIO_k2 "/value", O_RDONLY);
	if(f == -1)
	{
		printerror();
	}
	return f;
}

static int open_button_k3()
{

	//unexport
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if(f == -1)
	{
		printerror();
	}
	write (f, K3, strlen(K3));
	int ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	//export pin to sysfsa
	f = open(GPIO_EXPORT, O_WRONLY);
	if(f == -1)
	{
		printerror();
	}
	write (f, K3, strlen(K3));
	ret = close (f); 
	if(ret == -1)
	{
		printerror();
	}

	//config pin 
	f = open(GPIO_k3 "/direction",O_WRONLY);
	if(f == -1)
	{
		printerror();
	}
	write (f, "in", 2);
	ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	//falling mode
	f = open(GPIO_k3 "/edge",O_WRONLY);
	if(f == -1)
	{
		printerror();
	}
	write (f,"falling",7);
	ret = close(f);
	if(ret == -1)
	{
		printerror();
	}

	f = open(GPIO_k3 "/value", O_RDONLY);
	if(f == -1)
	{
		printerror();
	}
	return f;
}


void set_timer_values(struct itimerspec *ts, struct itimerspec *ts2, long p1, long p2)
{
	ts->it_interval.tv_sec = p2 / 1000000000;
	ts->it_interval.tv_nsec = (p2 % 1000000000);
	ts->it_value.tv_sec = p2 / 1000000000; // ns to s
	ts->it_value.tv_nsec = (p2 % 1000000000); // ns

	ts2->it_interval.tv_sec = p1 / 1000000000;
	ts2->it_interval.tv_nsec = p1 % 1000000000;
	ts2->it_value.tv_sec = p1 / 1000000000;
	ts2->it_value.tv_nsec = p1 % 1000000000;	

}

int main(int argc, char* argv[])
{

	int tfd,epfd,ret;
	struct itimerspec ts, ts2;
	long duty = 2;		// %
	long period = 500; // ms (2Hz)
	uint64_t value;
	int k = 0;
	if (argc >= 2)
		period   = atoi (argv[1]);
	period *= 1000000; // in ns

	// compute duty period...
	long p1 = period / 100 * duty;
	long p2 = period - p1;

	long act_period = period;

 	int led = open_led();
	pwrite (led, "1", sizeof("1"), 0);
	int k1 = open_button_k1();
	int k2 = open_button_k2();
	int k3 = open_button_k3();


	tfd = timerfd_create(CLOCK_MONOTONIC, 0);

	// Initialise first value period timer
	set_timer_values(&ts,&ts2,p1,p2);	
	timerfd_settime(tfd, 0, &ts, NULL);

	epfd = epoll_create1(0);
	if (epfd == -1)
	{
		printerror();
	}

	struct epoll_event ev_k1 = {
		.events = EPOLLPRI,
		.data.fd = k1,

	};
	struct epoll_event ev_k2 = {
		.events = EPOLLPRI,
		.data.fd = k2,

	};
	struct epoll_event ev_k3 = {
		.events = EPOLLPRI,
		.data.fd = k3,

	};		
	struct epoll_event ev_tfd = {
		.events = EPOLLIN,
		.data.fd = tfd,

	};

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev_tfd);
	if(ret == -1)
	{
		printerror();
	}
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, k1, &ev_k1);
	if(ret == -1)
	{
		printerror();
	}
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, k2, &ev_k2);
	if(ret == -1)
	{
		printerror();
	}
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, k3, &ev_k3);
	if(ret == -1)
	{
		printerror();
	}

	openlog("silly_led_control",LOG_CONS | LOG_PID | LOG_NDELAY,LOG_USER);
	while(1) {

		struct epoll_event events[5];	
		ret = epoll_wait(epfd, events, 5, -1);
		if(ret == -1)
		{
			printerror();
		}
		
		for(int i = 0; i< ret; i++)
		{
			if(events[i].data.fd == tfd)
			{
				
				read(tfd, &value, 8);
				if(k == 0)
				{
					k = 1;
					timerfd_settime(tfd, 0, &ts2, NULL);
					pwrite (led, "1", sizeof("1"), 0);	
				}
				else
				{
					k = 0;
					timerfd_settime(tfd, 0, &ts, NULL);
					pwrite (led, "0", sizeof("0"), 0);
				}
			}
			if(events[i].data.fd == k1)
			{
				read(k1,&value,8);	
				syslog(LOG_INFO,"Bouton K1, Fréquence augmentée");
				act_period = act_period / 2;
				p1 = act_period / 100 * duty;
				p2 = act_period - p1;
				set_timer_values(&ts,&ts2,p1,p2);
				timerfd_settime(tfd, 0, &ts, NULL);
				pwrite (led, "0", sizeof("0"), 0);
			}
			if(events[i].data.fd == k2)
			{
				read(k2,&value,8);
				syslog(LOG_INFO,"Bouton K2, Fréquence restorée");
				act_period = period;
				p1 = act_period / 100 * duty;
				p2 = act_period - p1;
				set_timer_values(&ts,&ts2,p1,p2);
				timerfd_settime(tfd, 0, &ts, NULL);
				pwrite (led, "0", sizeof("0"), 0);
			}
			if(events[i].data.fd == k3)
			{

				read(k3,&value,8);
				syslog(LOG_INFO,"Bouton K3, Fréquence diminuée");
				act_period = act_period * 2;
				p1 = act_period / 100 * duty;
				p2 = act_period - p1;
				set_timer_values(&ts,&ts2,p1,p2);
				timerfd_settime(tfd, 0, &ts, NULL);
				pwrite (led, "0", sizeof("0"), 0);
			}
		}

	}

	return 0;
}