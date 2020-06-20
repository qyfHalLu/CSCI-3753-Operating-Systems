### Qiuyang Fu

IdentiKey: 108667420

University email: qifu3807@colorado.edu

### Contents: all files in /home/kernel/linux-hwe-4.15.0

a. arch/x86/kernel/cs3753_add.c

    System call 334.

b. arch/x86/kernel/Makefile

    Makefile.

c. arch/x86/entry/syscalls/syscall_64.tbl

    Table of Kernel system calls.

d. include/linux/syscalls.h

    Header file of system calls.

e. /var/log/syslog

    printk messages from system calls. Use sudo tail /var/log/syslog to or type dmesg to check the printk outputs

f. Source code for your test program

  1. test_helloworld.c

  2. test_add.c

        
## To Run:
  1. Make sure all files are in /home/kernel/linux-hwe-4.15.0
  2. Use commands
  
        gcc -o testhello test_helloworld.c
        gcc -o test_add test_add.c

  3. Run test
  
        ./testhello
        ./test_add

  4. Check printk outputs
  
        dmesg
        sudo tail /var/log/syslog