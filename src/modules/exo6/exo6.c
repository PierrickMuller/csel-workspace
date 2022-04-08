// skeleton.c
#include <linux/module.h> // needed by all modules
#include <linux/init.h> // needed for macros
#include <linux/kernel.h> // needed for debugging
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct task_struct *thread1;

int thread_fun (void* data)
{
	int counter;
	counter = 1;
	while (!kthread_should_stop()) {
		pr_info("Hi ! I'm a thread. This is my %d message !!",counter);
		counter++;
		ssleep(5);
	}
	return 0;
}

static int __init exo6_init(void)
{
	thread1 = kthread_run(thread_fun, NULL, "thread-1");
	return 0;

}
static void __exit exo6_exit(void)
{
	kthread_stop(thread1);
    pr_info ("Linux module skeleton unloaded\n");
}
module_init (exo6_init);
module_exit (exo6_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");