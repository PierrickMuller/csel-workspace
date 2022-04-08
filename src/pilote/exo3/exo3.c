/* exo3.c */
#include <linux/cdev.h>        /* needed for char device driver */
#include <linux/fs.h>          /* needed for device drivers */
#include <linux/init.h>        /* needed for macros */
#include <linux/kernel.h>      /* needed for debugging */
#include <linux/module.h>      /* needed by all modules */
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/slab.h>        /* needed for dynamic memory management */
#include <linux/uaccess.h>     /* needed to copy data to/from user */

static int instances = 3;
module_param(instances, int, 0);

#define BUFFER_SZ 10000
static char** buffers = 0;

static int exo3_open(struct inode* i, struct file* f)
{
    pr_info("exo3 : open operation... major:%d, minor:%d\n",
            imajor(i),
            iminor(i));

    if (iminor(i) >= instances) {
        return -EFAULT;
    }

    if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0) {
        pr_info("exo3 : opened for reading & writing...\n");
    } else if ((f->f_mode & FMODE_READ) != 0) {
        pr_info("exo3 : opened for reading...\n");
    } else if ((f->f_mode & FMODE_WRITE) != 0) {
        pr_info("exo3 : opened for writing...\n");
    }

    f->private_data = buffers[iminor(i)];
    pr_info("exo3: private_data=%p\n", f->private_data);

    return 0;
}

static int exo3_release(struct inode* i, struct file* f)
{
    pr_info("exo3: release operation...\n");

    return 0;
}

static ssize_t exo3_read(struct file* f,
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

    pr_info("exo3: read operation... read=%ld\n", count);

    return count;
}

static ssize_t exo3_write(struct file* f,
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

    pr_info("exo3: write operation... private_data=%p, written=%ld\n",
            f->private_data,
            count);

    return count;
}

static struct file_operations exo3_fops = {
    .owner   = THIS_MODULE,
    .open    = exo3_open,
    .read    = exo3_read,
    .write   = exo3_write,
    .release = exo3_release,
};

static dev_t exo3_dev;
static struct cdev exo3_cdev;

static int __init exo3_init(void)
{
    int i;
    int status = -EFAULT;

    if (instances <= 0) return -EFAULT;

    status = alloc_chrdev_region(&exo3_dev, 0, instances, "mymodule");
    if (status == 0) {
        cdev_init(&exo3_cdev, &exo3_fops);
        exo3_cdev.owner = THIS_MODULE;
        status              = cdev_add(&exo3_cdev, exo3_dev, instances);
    }

    if (status == 0) {
        buffers = kzalloc(sizeof(char*) * instances, GFP_KERNEL);
        for (i = 0; i < instances; i++)
            buffers[i] = kzalloc(BUFFER_SZ, GFP_KERNEL);
    }

    pr_info("Linux module exo3 loaded\n");
    pr_info("The number of instances: %d\n", instances);
    return status;
}

static void __exit exo3_exit(void)
{
    int i;

    cdev_del(&exo3_cdev);
    unregister_chrdev_region(exo3_dev, instances);

    for (i = 0; i < instances; i++) kfree(buffers[i]);
    kfree(buffers);

    pr_info("Linux module exo3 unloaded\n");
}

module_init(exo3_init);
module_exit(exo3_exit);

MODULE_AUTHOR("Daniel Gachet <daniel.gachet@hefr.ch>");
MODULE_DESCRIPTION("Module exo3");
MODULE_LICENSE("GPL");
