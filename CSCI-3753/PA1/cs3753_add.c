#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(int num1, int num2, int *result)
{
	int sum = num1 + num2;
	printk(KERN_ALERT "Value for num1 is %d\n", num1);
	printk(KERN_ALERT "Value for num2 is %d\n", num2);
	copy_to_user(result, &sum, sizeof(int));
	printk(KERN_ALERT "Result of cs3753_add syscall: ");
	return 0;
}
