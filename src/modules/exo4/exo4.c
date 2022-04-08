// skeleton.c
#include <linux/module.h> // needed by all modules
#include <linux/init.h> // needed for macros
#include <linux/kernel.h> // needed for debugging
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/list.h>
#include <linux/slab.h> 

struct element {
    // some members
    char* text;
    int id;
    struct list_head node;
};

// definition of the global list
static LIST_HEAD (my_list);

void alloc_ele(const char * text, int id){
    struct element* ele;
    ele = kmalloc(sizeof(*ele), GFP_KERNEL);
    ele->text = kstrdup(text, GFP_KERNEL);
    ele->id = id;
    if (ele != NULL)
        list_add_tail(&ele->node, &my_list); // add element at the end of the list
}

static char* text= "dummy help";
module_param(text, charp, 0);

static int elements= 1;
module_param(elements, int, 0664);

static bool myParameter = true;
module_param(myParameter,bool,0664);

static int __init exo4_init(void)
{
    int i = 0;
    pr_info ("Linux module exo4 loaded\n");
    pr_info ("text: %s\nelements: %d\nmyParameter: %d\n", text, elements,myParameter);
    for(i = 0; i < elements; i++)
    {
        alloc_ele(text, i);
        pr_info("Alloc elem %d with text %s\n",i,text);
    }
    return 0;
}
static void __exit exo4_exit(void)
{
    struct element* ele;
    pr_info ("Linux module exo4 unloaded\n");
    
    list_for_each_entry(ele,&my_list,node){
        pr_info("Freeing elem %d\n",ele->id);
        kfree(ele->text);
        kfree(ele);
    }
}
module_init (exo4_init);
module_exit (exo4_exit);
MODULE_AUTHOR ("Pierrick Muller <pierrick.muller@he-arc.ch");
MODULE_DESCRIPTION ("Module exo4");
MODULE_LICENSE ("GPL");