/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project:	HEIA-FR / Embedded Systems Laboratory
 *
 * Abstract: Process and daemon samples
 *
 * Purpose: This module implements a simple daemon to be launched
 *          from a /etc/init.d/S??_* script
 *          -> this application requires /opt/daemon as root directory
 *
 * Autĥor:  Daniel Gachet
 * Date:    17.11.2015
 */
#define _XOPEN_SOURCE 600
#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "ssd1306.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define UNUSED(x) (void)(x)

#define VENTILATEUR_FREQ_FILE "/sys/class/Ventilateur_class/Ventilateur_device/freq"
#define VENTILATEUR_MODE_FILE "/sys/class/Ventilateur_class/Ventilateur_device/mode"
#define VENTILATEUR_CPU_TEMP_FILE "/sys/class/Ventilateur_class/Ventilateur_device/temp"

#define PORT 8080

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT		"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_LED		"/sys/class/gpio/gpio362"
#define GPIO_k1			"/sys/class/gpio/gpio0"
#define GPIO_k2			"/sys/class/gpio/gpio2"
#define GPIO_k3			"/sys/class/gpio/gpio3"

#define LED				"362"
#define K1				"0"
#define K2				"2"
#define K3				"3"

static int signal_catched = 0;
static int first_catched_k3 = 0;
static pthread_t thread_physic_interface;
static bool is_running = true;
//static struct itimerspec ts;
struct ptr_event_epoll {
    int fd;
    void* ptr;
};

void printerror(){
	char estr[100] = {[0]=0,};
	strerror_r(errno, estr, sizeof(estr)-1);
    syslog(LOG_INFO,"ERROR : %s",estr);
	fprintf (stderr, "ERROR: %s", estr);
}

static void catch_signal(int signal)
{
    syslog(LOG_INFO, "signal=%d catched\n", signal);
    signal_catched++;
}

static void fork_process()
{
    pid_t pid = fork();
    switch (pid) {
        case 0:
            break;  // child process has been created
        case -1:
            syslog(LOG_ERR, "ERROR while forking");
            exit(1);
            break;
        default:
            exit(0);  // exit parent process with success
    }
}

static int get_freq(int fd){
    int ret;
    char buf[10];
    pread(fd,buf,sizeof(buf),0);
    ret = atoi(buf); 
    return ret;
}
static void set_freq(int fd, int val){
    char result[50];
    sprintf(result,"%d",val);
    write(fd,result,sizeof(result));
}
static int get_mode(int fd){
    int ret;
    char buf[1024];
    pread(fd,buf,sizeof(buf),0);
    ret = atoi(buf);
    return ret;
}
static void set_mode(int fd, int val){
    char result[1];
    int real_val = val > 0 ? 1 : 0;
    sprintf(result,"%d",real_val);
    write(fd,result,sizeof(result));
}
static int get_cpu_temp(int fd){
	int ret;
	char buf[1024];
	pread(fd,buf,sizeof(buf),0);
	ret = atoi(buf);
	return ret;
}

static void launch_timer(int fd){
    struct itimerspec ts;
    ts.it_interval.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_value.tv_nsec = 125000000;
    ts.it_value.tv_sec = 0;
    timerfd_settime(fd,0,&ts,NULL);
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
static int open_freq(){
    int f = open(VENTILATEUR_FREQ_FILE,O_RDWR);
    if(f == -1)
    {
        printerror();
    }
    return f;
}
static int open_mode(){
    int f = open(VENTILATEUR_MODE_FILE,O_RDWR);
    if(f == -1)
    {
        printerror();
    }
    return f;
}
static int open_cpu_temp(){
	int f = open(VENTILATEUR_CPU_TEMP_FILE,O_RDONLY);
	if(f == -1)
	{
		printerror();
	}
	return f;
}

void event_func_tfd(int cur_fd, int led_fd, int freq_fd, int mode_fd,int cpu_temp_fd, int timer_fd){
	UNUSED(cur_fd);
	UNUSED(freq_fd);
	UNUSED(mode_fd);
	UNUSED(cpu_temp_fd);
	UNUSED(timer_fd);
	pwrite (led_fd, "0", sizeof("0"), 0);
}
void event_func_tfd_oled(int cur_fd, int led_fd, int freq_fd, int mode_fd,int cpu_temp_fd, int timer_fd){
	UNUSED(led_fd);
	UNUSED(timer_fd);
	
	int cpu_temp,freq,mode;
	char buf[1024];
	cpu_temp = get_cpu_temp(cpu_temp_fd);
	freq = get_freq(freq_fd);
	mode = get_mode(mode_fd);
	//syslog(LOG_INFO,"Cpu : %d, freq : %d, mode : %d",cpu_temp,freq,mode);
	ssd1306_set_position (0,0);
    ssd1306_puts("CSEL1a - SP.07");
    ssd1306_set_position (0,1);
    ssd1306_puts("  Demo - SW");
    ssd1306_set_position (0,2);
    ssd1306_puts("--------------");

    ssd1306_set_position (0,3);
	sprintf(buf,"Temp: %d'C",cpu_temp);
    ssd1306_puts(buf);
    ssd1306_set_position (0,4);
	sprintf(buf,"Freq: %dHz",freq);
    ssd1306_puts(buf);
    ssd1306_set_position (0,5);
	sprintf(buf,"Mode: %d",mode);
    ssd1306_puts(buf);
	launch_timer(cur_fd);
}
void event_func_k1(int cur_fd, int led_fd, int freq_fd, int mode_fd,int cpu_temp_fd, int timer_fd){
    UNUSED(mode_fd);
	UNUSED(cpu_temp_fd);
    uint64_t value;
    int ret;
    pwrite (led_fd,"1", sizeof("1"),0);
    launch_timer(timer_fd);
    if(read(cur_fd,&value,8) == -1){
        syslog(LOG_INFO,"Error on k1");
    }
    ret = get_freq(freq_fd);
    set_freq(freq_fd,ret + 1);
}
void event_func_k2(int cur_fd, int led_fd, int freq_fd, int mode_fd,int cpu_temp_fd, int timer_fd){
    UNUSED(mode_fd);
	UNUSED(cpu_temp_fd);   
    uint64_t value;
    int ret;
    pwrite (led_fd,"1", sizeof("1"),0);
    launch_timer(timer_fd);
    if(read(cur_fd,&value,8) == -1){
        printerror();
        syslog(LOG_INFO,"Error on k2");
    }
    ret = get_freq(freq_fd);
    if(ret > 1){
        set_freq(freq_fd,ret -1);
    }
}
void event_func_k3(int cur_fd, int led_fd, int freq_fd, int mode_fd,int cpu_temp_fd, int timer_fd){
	UNUSED(led_fd);
	UNUSED(freq_fd);
	UNUSED(cpu_temp_fd);
	UNUSED(timer_fd);
	uint64_t value;
    int ret;
    if(read(cur_fd,&value,8) == -1){
        syslog(LOG_INFO,"Error on k3");
    }
    if(first_catched_k3){
        ret = get_mode(mode_fd) > 0 ? 0 : 1;
        set_mode(mode_fd,ret);
    }
    else{
        first_catched_k3 = !first_catched_k3;
    }
}


void* thread_physic_interface_handler(void* arg)
{
	UNUSED(arg);
    int ret;
    int led = open_led();
    pwrite (led, "0", sizeof("0"), 0);
    int k1 = open_button_k1();
    int k2 = open_button_k2();
    int k3 = open_button_k3();
    int freq_fd = open_freq();
    int mode_fd = open_mode();
	int cpu_temp_fd = open_cpu_temp();

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
	int tfd_oled = timerfd_create(CLOCK_MONOTONIC, 0);
	launch_timer(tfd_oled);

    int epfd = epoll_create1(0);
    if(epfd == -1)
    {
        printerror();
    }

    struct ptr_event_epoll ptr_tfd = {tfd, event_func_tfd};
    struct epoll_event ev_tfd = {
        .events = EPOLLIN,
        .data.ptr = &ptr_tfd,
    };
	struct ptr_event_epoll ptr_tfd_oled = {tfd_oled, event_func_tfd_oled};
	struct epoll_event ev_tfd_oled = {
		.events = EPOLLIN,
		.data.ptr = &ptr_tfd_oled,
	};
    struct ptr_event_epoll ptr_k1 = {k1, event_func_k1};
    struct epoll_event ev_k1 = {
		.events = EPOLLPRI,
        .data.ptr = &ptr_k1,

	};
    struct ptr_event_epoll ptr_k2 = {k2,event_func_k2};
	struct epoll_event ev_k2 = {
		.events = EPOLLPRI,
        .data.ptr = &ptr_k2,
	};
    struct ptr_event_epoll ptr_k3 = {k3,event_func_k3};
	struct epoll_event ev_k3 = {
		.events = EPOLLPRI,
        .data.ptr = &ptr_k3,
    };

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev_tfd);
    if(ret == -1)
    {
        printerror();
    }
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, tfd_oled, &ev_tfd_oled);
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

    struct epoll_event events[5]; 
    while(is_running){
		ret = epoll_wait(epfd, events, 5, -1);
		if(ret == -1)
		{
			printerror();
		}

    	//https://stackoverflow.com/questions/1952175/how-can-i-call-a-function-using-a-function-pointer
        void(* event_func) ();
		for(int i = 0; i< ret; i++)
		{
            struct ptr_event_epoll *temp = events[i].data.ptr;
            event_func = temp->ptr;
            event_func(temp->fd,led,freq_fd,mode_fd,cpu_temp_fd,tfd);
        }

    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);

	int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

    // 1. fork off the parent process
    fork_process();

    // 2. create new session
    if (setsid() == -1) {
        syslog(LOG_ERR, "ERROR while creating new session");
        exit(1);
    }

    // 3. fork again to get rid of session leading process
    fork_process();

    // 4. capture all required signals
    struct sigaction act = {
        .sa_handler = catch_signal,
    };
    sigaction(SIGHUP, &act, NULL);   //  1 - hangup
    sigaction(SIGINT, &act, NULL);   //  2 - terminal interrupt
    sigaction(SIGQUIT, &act, NULL);  //  3 - terminal quit
    sigaction(SIGABRT, &act, NULL);  //  6 - abort
    //sigaction(SIGTERM, &act, NULL);  // 15 - termination
    sigaction(SIGTSTP, &act, NULL);  // 19 - terminal stop signal

    // 5. update file mode creation mask
    umask(0027);

    // 6. change working directory to appropriate place (Not /opt or not working)
    if (chdir("/") == -1) {
        syslog(LOG_ERR, "ERROR while changing to working directory");
        exit(1);
    }

    // 7. close all open file descriptors
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }

    // 8. redirect stdin, stdout and stderr to /dev/null
    if (open("/dev/null", O_RDWR) != STDIN_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stdin");
        exit(1);
    }
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stdout");
        exit(1);
    }
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        syslog(LOG_ERR, "ERROR while opening '/dev/null' for stderr");
        exit(1);
    }

    // 9. option: open syslog for message logging
    openlog(NULL, LOG_NDELAY | LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon has started...");

    // 13. implement daemon body...

	ssd1306_init();
    pthread_create(&thread_physic_interface, NULL, thread_physic_interface_handler, "thread_physic_interface_handler");

	//https://www.geeksforgeeks.org/socket-programming-cc/
	int freq_fd = open_freq();
    int mode_fd = open_mode();

	// Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(is_running)
    {
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        	perror("accept");
        	exit(EXIT_FAILURE);
    	}
		read(new_socket, buffer, 1024);
		if(strcmp(buffer,"stop") == 0)
		{
			is_running = false;
		}
		else if (strcmp(buffer,"set-mode") == 0)
		{
			int val;
			read(new_socket,buffer,1024);
			val = atoi(buffer);
			set_mode(mode_fd,val);
		}
		else if (strcmp(buffer,"set-freq") == 0)
		{
			int val;
			read(new_socket,buffer,1024);
			val = atoi(buffer);
			set_freq(freq_fd,val);
		}

	}

	// closing the connected socket
    close(new_socket);
  	// closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
  

  
    syslog(LOG_INFO,
           "daemon stopped. Number of signals catched=%d\n",
           signal_catched);
    closelog();

    return 0;
}