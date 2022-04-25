#!/bin/sh
#compile everything
#compile user code
echo compiling user code...
gcc -O3 -Wall -std=c11 message_sender.c -o sender
gcc -O3 -Wall -std=c11 message_reader.c -o reader
#compile kernel module
echo compiling kernel module...
make clean
make all
#compile tests
echo compiling tests...
gcc -O3 -Wall -std=c11 ex3_tester.c -o amits_test
gcc -O3 -Wall -std=c11 tester.c -o matans_test
gcc -O3 -Wall -std=c11 tester2.c -o matans_test2
#remove kernel module and device
echo removing kernel module and devices...
sudo rm /dev/matan
sudo rm /dev/matan2
sudo rm /dev/test0
sudo rm /dev/test1
sudo rmmod message_slot
#add kernel module and new device
echo adding kernel module and new devices...
sudo insmod message_slot.ko
sudo mknod /dev/test0 c 235 0
sudo mknod /dev/test1 c 235 1
sudo mknod /dev/matan c 235 2
sudo mknod /dev/matan2 c 235 3
sudo chmod 777 /dev/test0
sudo chmod 777 /dev/test1
sudo chmod 777 /dev/matan
sudo chmod 777 /dev/matan2
#run sender
echo running amits test...
./amits_test
echo running matans first test...
./matans_test /dev/matan
echo running matans second test...
./matans_test2 /dev/matan2
echo done!
