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
#include <linux/interrupt.h>	/* needed for interrupt handling */
#include <linux/gpio.h>			/* needed for i/o handling */



irqreturn_t short_interrupt_GPIO(int irq, void *dev_id)
{

	pr_info("Gpio interrupt, value : %s\n",(char*) dev_id);
	
	return IRQ_HANDLED;
}

static int __init exo8_init(void)
{
	int ret;

	ret = gpio_request(0,"gpio_a.0-k1");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}
	ret = request_irq(gpio_to_irq(0),short_interrupt_GPIO,IRQF_SHARED,"gpio_a.0-k1","gpio_a.0-k1");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}

	ret = gpio_request(2,"gpio_a.0-k2");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}
	ret = request_irq(gpio_to_irq(2),short_interrupt_GPIO,IRQF_SHARED,"gpio_a.0-k2","gpio_a.0-k2");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}

	ret = gpio_request(3,"gpio_a.0-k3");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}
	ret = request_irq(gpio_to_irq(3),short_interrupt_GPIO,IRQF_SHARED,"gpio_a.0-k3","gpio_a.0-k3");
	if (ret != 0){
		pr_info("Error !");
		return 0;
	}

	return 0;

}
static void __exit exo8_exit(void)
{
	gpio_free(0);
	free_irq(gpio_to_irq(0),"gpio_a.0-k1");

	gpio_free(2);
	free_irq(gpio_to_irq(2),"gpio_a.0-k2");

	gpio_free(3);
	free_irq(gpio_to_irq(3),"gpio_a.0-k3");

    pr_info ("Linux module skeleton unloaded\n");
}
module_init (exo8_init);
module_exit (exo8_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");