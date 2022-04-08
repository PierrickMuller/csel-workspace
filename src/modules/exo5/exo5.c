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



#define CHIPID_BASE_ADDR 0x01c14000
#define CPUTEMP_BASE_ADDR 0x01c25080
#define MACADDR_BASE_ADDR 0x01c30050
#define CHIPID_OFFSET 0x1000 // 32*4 bits = 16 bytes
#define CPUTEMP_OFFSET 0x4 // 32 bits = 4 bytes
#define MACADDR_OFFSET 0x8 // 32*2 bits = 8 bytes

static char* text= "dummy help";
module_param(text, charp, 0);

static int elements= 1;
module_param(elements, int, 0664);

static bool myParameter = true;
module_param(myParameter,bool,0664);

static struct resource* res[3]={[0]=0,};

static int __init exo5_init(void)
{

    unsigned char* regs[3]={[0]=0,};
	unsigned int chipid[4]={[0]=0,};
	long temp = 0;
	unsigned int addr[2] = {[0]=0,};
    
    pr_info ("Linux module skeleton loaded\n");
    pr_info ("text: %s\nelements: %d\nmyParameter: %d\n", text, elements,myParameter);

    pr_info ("Requesting memory region for Chip ID\n"); 
    pr_info ("Addr : 0x%x, len : 0x%x",CHIPID_BASE_ADDR,CHIPID_OFFSET);
    res[0] = request_mem_region(CHIPID_BASE_ADDR,CHIPID_OFFSET,"allwiner h5");
    if ((res[0] == 0))// || (res[1] == 0) ||(res[2] == 0))
		pr_info ("Error while reserving memory region... [0]=%d, [1]=%d, [2]=%d\n",	res[0]==0, res[1]==0, res[2]==0);

    regs[0] = ioremap (0x01c14000, 0x1000);
	regs[1] = ioremap (0x01C25000, 0x1000);
	regs[2] = ioremap (0x01C30000, 0x1000);

	if ((regs[0] == 0) || (regs[1] == 0) ||(regs[2] == 0)) {
		pr_info ("Error while trying to map processor register...\n");
		return -EFAULT;
	}
	chipid[0] = ioread32 (regs[0]+0x200);
	chipid[1] = ioread32 (regs[0]+0x204);
	chipid[2] = ioread32 (regs[0]+0x208);
	chipid[3] = ioread32 (regs[0]+0x20c);
	pr_info("chipid=%08x'%08x'%08x'%08x\n",
	 	chipid[0], chipid[1], chipid[2], chipid[3]);

	temp = -1191 * (int)ioread32(regs[1]+0x80) / 10 + 223000;
	pr_info ("temperature=%ld (%d)\n", temp, ioread32(regs[1]+0x80));

	addr[0]=ioread32(regs[2]+0x50);
	addr[1]=ioread32(regs[2]+0x54);
	pr_info("mac-addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			(addr[1]>> 0) & 0xff,
			(addr[1]>> 8) & 0xff,
			(addr[1]>>16) & 0xff,
			(addr[1]>>24) & 0xff,
			(addr[0]>> 0) & 0xff,
			(addr[0]>> 8) & 0xff
	);

	iounmap (regs[0]);
	iounmap (regs[1]);
	iounmap (regs[2]);

	return 0;

    /*pr_info ("Requesting memory region for CPU Temp\n");
    pr_info ("Addr : 0x%x, len : 0x%x",CPUTEMP_BASE_ADDR,CPUTEMP_OFFSET);
    if(request_mem_region(CPUTEMP_BASE_ADDR,CPUTEMP_OFFSET,"DEV_CPUTEMP") == NULL){
        pr_err ("Error : Cant request memory region for CPU_TEMP");
        return 1;
    }
    
    pr_info ("Requesting memory region for MAC ADDR\n");
    pr_info ("Addr : 0x%x, len : 0x%x",MACADDR_BASE_ADDR,MACADDR_OFFSET);
    if(request_mem_region(MACADDR_BASE_ADDR,MACADDR_OFFSET,"DEV_CPUTEMP") == NULL){
        pr_err ("Error : Cant request memory region for MAC ADDRESS");
        return 1;
    }

    pr_info ("Creating Virtual Mapping for IO\n");
    void* chipId = ioremap(CHIPID_BASE_ADDR,CHIPID_OFFSET);

    pr_info ("test");

    return 0;*/
}
static void __exit exo5_exit(void)
{
    /*release_mem_region (CHIPID_BASE_ADDR, CHIPID_OFFSET);
    release_mem_region (CPUTEMP_BASE_ADDR, CPUTEMP_OFFSET);
    release_mem_region (MACADDR_BASE_ADDR, MACADDR_OFFSET);*/
    pr_info ("Linux module skeleton unloaded\n");
    if (res[0] != 0) release_mem_region (0x01c14000, 0x1000);
	//release_mem_region (0x01C25000, 0x1000);
	//release_mem_region (0x01C30000, 0x1000);
}
module_init (exo5_init);
module_exit (exo5_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");