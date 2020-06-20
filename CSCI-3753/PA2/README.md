### Qiuyang Fu

IdentiKey: 108667420

University email: qifu3807@colorado.edu

###Submission Contents:
####hello folder
1. helloModule.c

    example program

2. Makefile
    
    example Makefile
    
####assignment folder
1. pa2_char_driver.c
    
    pa2 driver 

2. test_cases.c
  
    test cases for pa2

3. Makefile
  
    simple Makefile

###To Run:
  1. Put all files in modules folder,
  2. Create the module object:

        make -C /lib/modules/$(uname -r)/build M=$PWD modules

  3. Compile the test app:
    
        gcc -o test_cases test_cases.c
  
  4. Create the device file:
    
        sudo mknod -m 777 /dev/pa2_character_device c 375 0
  
  5. Install the module: 
    
        sudo insmod pa2_char_driver.ko
  
  6. Confirm installations: 
    
        lsmod 
        cat /proc/devices
  
  7. Run test_cases:
    
        ./test_cases
  
  8. When finished uninstall module using the following command:

        sudo rmmod pa2_char_driver