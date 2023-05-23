#include <linux/module.h>  
#include <linux/slab.h>
MODULE_LICENSE("GPL");
/*TODO: allocate 1KB for kmallocmem1*/  
unsigned char *kmallocmem1;  
/*TODO: allocate 8KB for kmallocmem2*/  
unsigned char *kmallocmem2;   
/*TODO: allocate max-allowed size of memory for kmallocmem3*/  
unsigned char *kmallocmem3;  
/*TODO: allocate memory lager than max-allowed size for kmallocmem4*/  
unsigned char *kmallocmem4;

static int __init mem_module_init(void)
{
    printk(KERN_INFO "Start kmalloc!\n");

    kmallocmem1 = (unsigned char*)kmalloc(1024, GFP_KERNEL);
    if (!kmallocmem1) {
        printk(KERN_ALERT "Failed to allocate kmallocmem1!\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "kmallocmem1 addr= %p\n", (unsigned long)kmallocmem1);

    kmallocmem2 =  (unsigned char*)kmalloc(8192, GFP_KERNEL);
    if (!kmallocmem2) {
        printk(KERN_ALERT "Failed to allocate kmallocmem2!\n");
        kfree(kmallocmem1);
        return -ENOMEM;
    }
    printk(KERN_INFO "kmallocmem2 addr= %p\n", (unsigned long)kmallocmem2);

    kmallocmem3 =  (unsigned char*)kmalloc((size_t)(-1), GFP_KERNEL);
    if (!kmallocmem3) {
        printk(KERN_ALERT "Failed to allocate kmallocmem3!\n");
        kfree(kmallocmem2);
        kfree(kmallocmem1);
        return -ENOMEM;
    }
    printk(KERN_INFO "kmallocmem3 addr= %p\n", (unsigned long)kmallocmem3);

    kmallocmem4 = (unsigned char*)kmalloc((size_t)(-1) + 1, GFP_KERNEL);
    if (!kmallocmem4) {
        printk(KERN_ALERT "Failed to allocate kmallocmem4!\n");
        kfree(kmallocmem3);
        kfree(kmallocmem2);
        kfree(kmallocmem1);
        return -ENOMEM;
    }

    return 0;
}

static void __exit mem_module_exit(void)
{
    printk(KERN_INFO "Exit kmalloc!\n");

    kfree(kmallocmem4);
    kfree(kmallocmem3);
    kfree(kmallocmem2);
    kfree(kmallocmem1);
}

module_init(mem_module_init);
module_exit(mem_module_exit);