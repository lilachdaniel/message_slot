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
#remove kernel module and devices
echo removing kernel module and devices...
sudo rm /dev/slot0
sudo rmmod message_slot
#add kernel module and new device
echo adding kernel module and devices...
sudo insmod message_slot.ko
echo kernel module added.
sudo mknod /dev/slot0 c 235 0
sudo chmod 777 /dev/slot0
#run sender
echo trying to use module...
./sender /dev/slot0 1 fuck 
./reader /dev/slot0 1
echo done!
