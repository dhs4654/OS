/* Hello from Kernel! */  
#include <linux/module.h>  
MODULE_LICENSE("GPL");  
static char* name = "lxh";  
module_param(name, charp, 0644);  
MODULE_PARM_DESC(name, "char* param\n");  
static char* id = "22920202202797";  
module_param(id, charp, 0644);  
MODULE_PARM_DESC(id, "char* param\n");  
static int age = 21;  
module_param(age, int, 0644);  
MODULE_PARM_DESC(age, "int param\n");  

void hello_student(char* name, char id, int age) {
    printk(KERN_ALERT "My name is %s, student id is %d. I am %d years old.\n", name, id, age);
}

void my_magic_number(int id, int age) {
    int sum = 0;
    int i = 0;
    while (id[i]) {
        sum += id[i] - '0';
        i += 1;
    }
    sum = sum + age;
    int magicNumber = sum % 10;
    printk(KERN_ALERT "My magic number is %d.\n", magicNumber);
}
int __init hello_init(void)  
{  
    printk(KERN_ALERT "Init module.\n");  
    hello_student(name, id, age);  
    my_magic_number(id, age);
    return 0;  
}  
void __exit hello_exit(void)  
{  
    printk(KERN_ALERT "Exit module.\n");  
}  
module_init(hello_init);  
module_exit(hello_exit);  