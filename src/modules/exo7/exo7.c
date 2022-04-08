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
#include <linux/wait.h>

struct task_struct *threads[2];

DECLARE_WAIT_QUEUE_HEAD(queue_for_threads);
static atomic_t is_kicked;


int thread_fun1 (void* data)
{
	int ret;
	pr_info("Thread 1 started\n");
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(queue_for_threads,(atomic_read(&is_kicked) != 0)
					|| kthread_should_stop());
		if(ret == -ERESTARTSYS){
			pr_info ("skeleton thread_1 has been interrupted\n");
			break;
		}
		atomic_dec (&is_kicked);
		pr_info ("skeleton thread_1 has been kicked\n");
	}
	return 0;
}

int thread_fun2 (void* data)
{
	wait_queue_head_t queue;
	//int ret;
	pr_info("Thread 2 started\n");
	init_waitqueue_head (&queue);
	while(!kthread_should_stop()){
		ssleep(5);
		pr_info ("skeleton thread_2 timout elapsed...\n");
		atomic_set (&is_kicked, 1);
		wake_up_interruptible (&queue_for_threads);
	}
	return 0;
}

static int __init exo7_init(void)
{

	atomic_set(&is_kicked,0);
	threads[0] = kthread_run(thread_fun1, NULL, "thread-1");
	threads[1] = kthread_run(thread_fun2, NULL, "thread-2");
	return 0;

}
static void __exit exo7_exit(void)
{
	kthread_stop(threads[0]);
	kthread_stop(threads[1]);
    pr_info ("Linux module skeleton unloaded\n");
}
module_init (exo7_init);
module_exit (exo7_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");