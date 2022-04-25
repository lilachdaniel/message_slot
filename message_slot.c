#undef __KERNEL__
#define __KERNEL__
#undef __MODULE__
#define __MODULE__

#include "message_slot.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/types.h>



MODULE_LICENSE("GPL");

typedef struct chan_llist {
	unsigned long chan_id;
	char chan_msg[MAX_BUFF_SIZE];
	int len;
	struct chan_llist* next;
} chan_llist;

typedef struct msg_slot {
	chan_llist* channels;
} msg_slot;


static msg_slot* all_msg_slots[257];

chan_llist *create_channel(unsigned long channel_id);
chan_llist *find_channel(msg_slot* slot, unsigned long channel_id);

/**************************************************
**************** DEVICE FUNCTIONS *****************
**************************************************/

/* Checks if it has already created a data
   structure for the file being opened, and create one if not */
static int device_open(struct inode* inode, struct file* file) {
	int minor = iminor(inode);
	if (all_msg_slots[minor] != NULL) { /* already opened */
		return 0;
	}
	
	if ((all_msg_slots[minor] = (msg_slot *)kmalloc(sizeof(msg_slot), GFP_KERNEL)) < 0) { /* open */
  			printk("failed to allocate memory for new msg slot\n");
  			return -1;
  		}
  	all_msg_slots[minor]->channels = NULL;
  	
  	return 0;
}

/* Associates the passed channel id with the file descriptor it was invoked on */
static long device_ioctl(struct file* file, unsigned int command, unsigned long parameter) {
	chan_llist *new_channel, *first_channel;
	msg_slot *slot;
	/* check for invalid inputs */
	if (file == NULL) { return -EINVAL; } /* invalid file */

	if (file->f_inode == NULL) { return -EINVAL; } /* invalid inode */ 

	if (command != MSG_SLOT_CHANNEL) { return -EINVAL; }

	if (parameter == 0) { return -EINVAL; }

	/* add channel */
	slot = all_msg_slots[iminor(file->f_inode)];
	
	if (find_channel(slot, parameter) == NULL) { /* channel does not exist yet */
		if ((new_channel = create_channel(parameter)) == NULL) { return -1; } /* failed to create channel */
		
		if ((first_channel = slot->channels) == NULL) { /* new_channel is the first channel in the message_slot */
			slot->channels = new_channel;
		}
		else {
			new_channel->next = slot->channels;
			slot->channels = new_channel;
		}
	}

	/* switch channels */
	file->private_data = (void *) parameter;

	return 0;
}

/* Receives channel_id and creates a channel with that channel_id and an empty message */
chan_llist *create_channel(unsigned long channel_id) {
	chan_llist *new_channel = (chan_llist *)kmalloc(sizeof(chan_llist), GFP_KERNEL);
	
	if (new_channel == NULL) { return NULL; } /* failed to create channel */
	
	new_channel->chan_id = channel_id;
	new_channel->len = 0;
	new_channel->next = NULL;
	
	return new_channel;

}

static int device_release(struct inode* inode, struct file* file) {
	return 0;
}

/* Reads the last message written on the channel into the user’s buffer. 
   Returns the number of bytes read, unless an error occurs */
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
	int i;
	
	unsigned long channel_id;
	chan_llist* channel;
	
	if (file == NULL) { return -EINVAL; } /* invalid file */

	if (file->f_inode == NULL) { return -EINVAL; } /* invalid inode */

	if (file->private_data == NULL) { return -EINVAL; } /* no channel has been set on the file descriptor */
	channel_id = (unsigned long) (file->private_data);

	channel = find_channel(all_msg_slots[iminor(file->f_inode)], channel_id); /* not supposed to return NULL */

	if (channel->len == 0) {return -EWOULDBLOCK; } /* no message exists on the channel */

	if (length < channel->len) { return -ENOSPC; } /* buffer length provided is too small to hold the last message written on the channel */

	for (i = 0; i < (channel->len); ++i) {
		if (put_user(channel->chan_msg[i], &buffer[i]) != 0) { return -EFAULT; }
	}
	
	return channel->len;
}

/* Finds channel. If does not exist, returns NULL */
chan_llist *find_channel(msg_slot* slot, unsigned long channel_id) {
	chan_llist* curr = slot->channels;

	while (curr != NULL) {
		/* check if curr is a channel with that id */
		if (curr->chan_id == channel_id) { return curr; } 
		
		/* continue */
		curr = curr->next;
	}

	return curr;
}


/* Writes a non-empty message of up to 128 bytes from the user’s buffer to the channel. 
   Returns the number of bytes written, unless an error occurs */
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
	int i;
	
	unsigned long channel_id;
	chan_llist* channel;

	if (file == NULL) { return -EINVAL; } /* invalid file */
	
	if (file->f_inode == NULL) { return -EINVAL; } /* invalid inode */
	
	if (file->private_data == NULL) { return -EINVAL; } /* no channel has been set on the file descriptor */
	channel_id = (unsigned long) (file->private_data);

	channel = find_channel(all_msg_slots[iminor(file->f_inode)], channel_id); /* not supposed to return NULL */

	if (length == 0 || length > MAX_BUFF_SIZE) { return -EMSGSIZE; } /* invalid length */
	
	for (i = 0; i < length; ++i) {
		if (get_user(channel->chan_msg[i], &buffer[i]) != 0) { return -EFAULT; }
	}
	
	/* update len */
	channel->len = length;

	return length;
}


/**************************************************
****************** DEVICE SETUP *******************
**************************************************/

struct file_operations Fops ={
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release = device_release,
};


static int __init msg_slots_init(void) {
	int rc = -1;
	printk("in msg_slot_init\n");

	/* register character device */
  	rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);

  	if(rc < 0) { /* error! */
    		printk(KERN_ALERT "%s registraion failed for  %d\n", DEVICE_RANGE_NAME, MAJOR_NUM);
    		return rc;
  	}
  	printk( "Registeration is successful. ");
	printk( "If you want to talk to the device driver,\n" );
	printk( "you have to create a device file:\n" );
	printk( "mknod /dev/%s c %d 0\n", DEVICE_RANGE_NAME, MAJOR_NUM );
	printk( "You can echo/cat to/from the device file.\n" );
	printk( "Dont forget to rm the device file and "
          "rmmod when you're done\n" );
  	
  	return 0; /* success! */
}


static void __exit msg_slots_cleanup(void){
	unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}



module_init(msg_slots_init);
module_exit(msg_slots_cleanup);




















