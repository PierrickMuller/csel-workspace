/* skeleton.c */
#include <linux/cdev.h>   /* needed for char device driver */
#include <linux/device.h> /* needed for sysfs handling */
#include <linux/fs.h>     /* needed for device drivers */
#include <linux/init.h>   /* needed for macros */
#include <linux/kernel.h> /* needed for debugging */
#include <linux/miscdevice.h>
#include <linux/module.h>          /* needed by all modules */
#include <linux/platform_device.h> /* needed for sysfs handling */
#include <linux/uaccess.h>         /* needed to copy data to/from user */
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/thermal.h>


#define CPU_THERMAL_NAME "cpu-thermal"
#define LED_GPIO 10

static int mode;
static int freq; 
static int led_value;
struct thermal_zone_device *cpu_zone;

static struct timer_list my_timer;

/*Function to change led value in timer*/
static void change_led_value(int value)
{
    gpio_set_value(LED_GPIO,value);
}

/*Timer callback (Based on http://yannik520.github.io/linux_driver_code/timer/timer_example.html)*/

static void timer_callback(struct timer_list *timer)
{
    int ret, temp;

	//printk("%s called (%ld)\n", __func__, jiffies);
    change_led_value(led_value);
    led_value = led_value ? 0 : 1;

    if(mode == 0)
    {
        ret = thermal_zone_get_temp(cpu_zone,&temp);
        if (temp < 35000)
        {
            freq = 2;
        }
        else if (temp < 40000)
        {
            freq = 5;
        }
        else if (temp < 45000)
        {
            freq = 10;
        }
        else 
        {
            freq = 20;
        }
    }
    mod_timer(&my_timer,jiffies + msecs_to_jiffies(1000 / freq));
}

ssize_t sysfs_show_mode(struct device* dev,
                       struct device_attribute* attr,
                       char* buf)
{
    sprintf(buf, "%d\n", mode);
    return strlen(buf);
}
ssize_t sysfs_store_mode(struct device* dev,
                        struct device_attribute* attr,
                        const char* buf,
                        size_t count)
{
    mode = simple_strtol(buf, 0, 10);
    return count;
}
DEVICE_ATTR(mode, 0664, sysfs_show_mode, sysfs_store_mode);

ssize_t sysfs_show_freq(struct device* dev,
                       struct device_attribute* attr,
                       char* buf)
{
    sprintf(buf, "%d\n", freq);
    return strlen(buf);
}
ssize_t sysfs_store_freq(struct device* dev,
                        struct device_attribute* attr,
                        const char* buf,
                        size_t count)
{
    freq = simple_strtol(buf, 0, 10);
    return count;
}
DEVICE_ATTR(freq, 0664, sysfs_show_freq, sysfs_store_freq);

ssize_t sysfs_show_temp(struct device* dev,
                       struct device_attribute* attr,
                       char* buf)
{
    int temp;
    thermal_zone_get_temp(cpu_zone,&temp);
    sprintf(buf, "%d\n", (temp / 1000) );
    return strlen(buf);
}
DEVICE_ATTR(temp,0444,sysfs_show_temp,NULL);


static struct class* sysfs_class;
static struct device* sysfs_device;

static int __init skeleton_init(void)
{
    int status = 0;

    /*Set base value for mode and frequency*/
    mode = 0;
    freq = 2;

    /*Initialize sysfs class and device*/
    sysfs_class  = class_create(THIS_MODULE, "Ventilateur_class");
    sysfs_device = device_create(sysfs_class, NULL, 0, NULL, "Ventilateur_device");
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_mode);
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_freq);
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_temp);
    
    /*Initialise cpu_zone*/
    cpu_zone = thermal_zone_get_zone_by_name(CPU_THERMAL_NAME);

    /*Initialise GPIO*/
    status = gpio_request(LED_GPIO, "LED");
    gpio_direction_output(LED_GPIO, 0 );

    /*Initialisation du timer*/
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer,jiffies + msecs_to_jiffies(1000/freq));
    
    
    
    pr_info("Linux module skeleton loaded\n");
    return 0;
}


static void __exit skeleton_exit(void)
{
    device_remove_file(sysfs_device, &dev_attr_mode);
    device_remove_file(sysfs_device, &dev_attr_freq);
    device_remove_file(sysfs_device, &dev_attr_temp);
    device_destroy(sysfs_class, 0);
    class_destroy(sysfs_class);

    /*Free led GPIO*/
    change_led_value(0);
    gpio_free(LED_GPIO);
    /*Delete timer*/
    del_timer(&my_timer);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Daniel Gachet <daniel.gachet@hefr.ch>");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
