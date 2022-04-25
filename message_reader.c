#include "message_slot.h"    

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	int fd, channel_id, msg_len;
	char *file_path;
	char msg[MAX_BUFF_SIZE];
	
	/* validate correct number of arguments */
	if (argc != 3) {
		perror("invalid number of arguments");
		exit(1); 
	}
	
	/* take arguments */
	file_path = argv[1];
	channel_id = atoi(argv[2]);
	
	/* the flow */
	if ((fd = open(file_path, O_RDONLY)) < 0) { /* open message slot device */
		perror("failed to open file");
		exit(1);
	}
	
	if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) { /* set channel to channel_id */
		perror("failed to set channel");
		exit(1);
	}
	
	if ((msg_len = read(fd, msg, MAX_BUFF_SIZE)) < 0) { /* read a message from the message slot file to msg */
		perror("failed to read msg to channel");
		exit(1);
	}
	
	/* finish */
	close(fd);
	
	if (write(1, msg, msg_len) < 0) { /* print msg to stdout */
		perror("faild to print msg");
		exit(1);
	}
	
	exit(0);
}
