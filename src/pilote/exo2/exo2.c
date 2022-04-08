// skeleton.c
#include <linux/module.h> // needed by all modules
#include <linux/init.h> // needed for macros
#include <linux/kernel.h> // needed for debugging
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#define BUFFER_SZ 10000
static char s_buffer[BUFFER_SZ];

static dev_t exo2_devt;
static struct cdev exo2_cdev;


static int exo2_open(struct inode* i, struct file* f){
    pr_info ("Exo2 : open operation... major:%d, minor:%d\n",
        imajor (i), iminor(i));
    if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0) {
        pr_info ("Exo2 : opened for reading & writing...\n");
    } else if ((f->f_mode & FMODE_READ) != 0) {
        pr_info ("Exo2 : opened for reading...\n");
    } else if ((f->f_mode & FMODE_WRITE) != 0) {
        pr_info ("Exo2 : opened for writing...\n");
    }
    pr_info("Open operation : Done");
    return 0;
}

static int exo2_release(struct inode* i, struct file* f){
    pr_info("Releas operation : Done");
    return 0;
}

static ssize_t exo2_read(struct file* f, char* __user buf,size_t count, loff_t* off){
   // compute remaining bytes to copy, update count and pointers
    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off);
    char* ptr         = s_buffer + *off;
    if (count > remaining) count = remaining;
    *off += count;

    // copy required number of bytes
    if (copy_to_user(buf, ptr, count) != 0) count = -EFAULT;

    pr_info("skeleton: read operation... read=%ld\n", count);

    return count; 
}

static ssize_t exo2_write(struct file* f, const char* __user buf, size_t count, loff_t* off){
 // compute remaining space in buffer and update pointers
    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off);

    pr_info("skeleton: at%ld\n", (unsigned long)(*off));

    // check if still remaining space to store additional bytes
    if (count >= remaining) count = -EIO;

    // store additional bytes into internal buffer
    if (count > 0) {
        char* ptr = s_buffer + *off;
        *off += count;
        ptr[count] = 0;  // make sure string is null terminated
        if (copy_from_user(ptr, buf, count)) count = -EFAULT;
    }

    pr_info("skeleton: write operation... written=%ld\n", count);

    return count;
}

static struct file_operations exo2_fops = {
.owner = THIS_MODULE,
.open = exo2_open,
.read = exo2_read,
.write = exo2_write,
.release = exo2_release,
};

static int __init exo2_init(void)
{
    int ret;

    ret = alloc_chrdev_region (&exo2_devt, 0, 1, "mydriver02");
    if (ret != 0){
        pr_err("Error ! Problem with alloc_chrdev_region\n");
        return 1;
    }
    cdev_init(&exo2_cdev,&exo2_fops);
    exo2_cdev.owner = THIS_MODULE;
    ret = cdev_add(&exo2_cdev,exo2_devt,1);
    if (ret != 0){
        pr_err("Error ! Problem with cdev_add");
        return 1;
    }
    pr_info ("Linux driver exo2 loaded\n");
    return 0;
}

static void __exit exo2_exit(void)
{
    cdev_del(&exo2_cdev);
    unregister_chrdev_region(exo2_devt,1);
    pr_info ("Linux driver exo2 unloaded\n");
}

module_init (exo2_init);
module_exit (exo2_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Driver exo2");
MODULE_LICENSE ("GPL");