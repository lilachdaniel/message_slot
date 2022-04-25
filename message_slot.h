#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 235

/* Set the message of the device driver */
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)


#define DEVICE_RANGE_NAME "message_slot"
#define MAX_BUFF_SIZE 128
#define MAX_MINOR 256



#endif
