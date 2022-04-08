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
#include <linux/slab.h>


#define USING_CLASS
//#define USING_MISC
//#define USING_PLATFORM

#define BUFFER_SZ 10000

struct skeleton_config {
    int id;
    long ref;
    char name[30];
    char descr[30];
};

static char** buffers = 0;
static dev_t skeleton_dev;
static struct cdev skeleton_cdev;
static struct skeleton_config config;
static int val;
static int instances = 3;


module_param(instances, int, 0);

static int skeleton_open(struct inode* i, struct file* f)
{
    pr_info("skeleton : open operation... major:%d, minor:%d\n",
            imajor(i),
            iminor(i));
    #ifndef USING_MISC
        if (iminor(i) >= instances) {
            return -EFAULT;
        }
    #endif

    if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0) {
        pr_info("skeleton : opened for reading & writing...\n");
    } else if ((f->f_mode & FMODE_READ) != 0) {
        pr_info("skeleton : opened for reading...\n");
    } else if ((f->f_mode & FMODE_WRITE) != 0) {
        pr_info("skeleton : opened for writing...\n");
    }

    //Here we have a problem with miscdevice, because minor may be out of range since it is automatically set
    //In order to fix that, we set only one buffer for MISC device
    #ifndef USING_MISC
        f->private_data = buffers[iminor(i)];
    #else 
        f->private_data = buffers[0];
    #endif 
    pr_info("skeleton: private_data=%p\n", f->private_data);

    return 0;
}

static int skeleton_release(struct inode* i, struct file* f)
{
    pr_info("skeleton: release operation...\n");

    return 0;
}

static ssize_t skeleton_read(struct file* f,
                             char __user* buf,
                             size_t count,
                             loff_t* off)
{
    // compute remaining bytes to copy, update count and pointers
    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off);
    char* ptr         = (char*)f->private_data + *off;
    if (count > remaining) count = remaining;
    *off += count;

    // copy required number of bytes
    if (copy_to_user(buf, ptr, count) != 0) count = -EFAULT;

    pr_info("skeleton: read operation... read=%ld\n", count);

    return count;
}

static ssize_t skeleton_write(struct file* f,
                              const char __user* buf,
                              size_t count,
                              loff_t* off)
{
    // compute remaining space in buffer and update pointers
    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off);

    // check if still remaining space to store additional bytes
    if (count >= remaining) count = -EIO;

    // store additional bytes into internal buffer
    if (count > 0) {
        char* ptr = f->private_data + *off;
        *off += count;
        ptr[count] = 0;  // make sure string is null terminated
        if (copy_from_user(ptr, buf, count)) count = -EFAULT;
    }

    pr_info("skeleton: write operation... private_data=%p, written=%ld\n",
            f->private_data,
            count);

    return count;
}

static struct file_operations skeleton_fops = {
    .owner   = THIS_MODULE,
    .open    = skeleton_open,
    .read    = skeleton_read,
    .write   = skeleton_write,
    .release = skeleton_release,
};

ssize_t sysfs_show_val(struct device* dev,
                       struct device_attribute* attr,
                       char* buf)
{
    sprintf(buf, "%d\n", val);
    return strlen(buf);
}
ssize_t sysfs_store_val(struct device* dev,
                        struct device_attribute* attr,
                        const char* buf,
                        size_t count)
{
    val = simple_strtol(buf, 0, 10);
    return count;
}
DEVICE_ATTR(val, 0664, sysfs_show_val, sysfs_store_val);

ssize_t sysfs_show_cfg(struct device* dev,
                       struct device_attribute* attr,
                       char* buf)
{
    sprintf(buf,
            "%d %ld %s %s\n",
            config.id,
            config.ref,
            config.name,
            config.descr);
    return strlen(buf);
}
ssize_t sysfs_store_cfg(struct device* dev,
                        struct device_attribute* attr,
                        const char* buf,
                        size_t count)
{
    sscanf(buf,
           "%d %ld %s %s",
           &config.id,
           &config.ref,
           config.name,
           config.descr);
    return count;
}
DEVICE_ATTR(cfg, 0664, sysfs_show_cfg, sysfs_store_cfg);

#ifdef USING_MISC
static struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "mydriver51",
    .fops = &skeleton_fops,
    //.mode  = 0,
};
#endif

#ifdef USING_PLATFORM
static void sysfs_dev_release(struct device* dev)
{
    pr_info("skeleton - sysfs dev release\n");
}

static struct platform_device platform_device = {
    .name = "mydriver51", .id = -1, .dev.release = sysfs_dev_release};
#endif

#ifdef USING_CLASS
static struct class* sysfs_class;
static struct device* sysfs_device;
#endif

static int __init skeleton_init(void)
{
    int status = 0;
    int i;

#ifdef USING_MISC
    if (status == 0) status = misc_register(&misc_device);
    if (status == 0)
        status = device_create_file(misc_device.this_device, &dev_attr_val);
    if (status == 0)
        status = device_create_file(misc_device.this_device, &dev_attr_cfg);
    instances = 1;
#endif

#ifndef USING_MISC
    status = alloc_chrdev_region(&skeleton_dev, 0, instances, "mydriver51");
    if (status == 0) {
        cdev_init(&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status              = cdev_add(&skeleton_cdev, skeleton_dev, instances);
    }
#endif

    if (status == 0) {
        buffers = kzalloc(sizeof(char*) * instances, GFP_KERNEL);
        for (i = 0; i < instances; i++)
            buffers[i] = kzalloc(BUFFER_SZ, GFP_KERNEL);
    }

#ifdef USING_PLATFORM
    //We have to add it here because alloc_chrdev_region is called before and not constant
    platform_device.dev.devt = skeleton_dev;
    if (status == 0) status = platform_device_register(&platform_device);
    if (status == 0)
        status = device_create_file(&platform_device.dev, &dev_attr_val);
    if (status == 0)
        status = device_create_file(&platform_device.dev, &dev_attr_cfg);
#endif

#ifdef USING_CLASS
    sysfs_class  = class_create(THIS_MODULE, "mydriver51");
    sysfs_device = device_create(sysfs_class, NULL, skeleton_dev, NULL, "mydriver51");
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_val);
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_cfg);
#endif

    pr_info("Linux module skeleton loaded\n");
    return 0;
}

static void __exit skeleton_exit(void)
{
    int i;

#ifdef USING_MISC
    misc_deregister(&misc_device);
#endif

#ifndef USING_MISC
    cdev_del(&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, instances);
#endif 

#ifdef USING_PLATFORM
    device_remove_file(&platform_device.dev, &dev_attr_cfg);
    device_remove_file(&platform_device.dev, &dev_attr_val);
    platform_device_unregister(&platform_device);
#endif

#ifdef USING_CLASS
    device_remove_file(sysfs_device, &dev_attr_val);
    device_remove_file(sysfs_device, &dev_attr_cfg);
    device_destroy(sysfs_class, 0);
    class_destroy(sysfs_class);
#endif

    for (i = 0; i < instances; i++) kfree(buffers[i]);
    kfree(buffers);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Daniel Gachet <daniel.gachet@hefr.ch>");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");