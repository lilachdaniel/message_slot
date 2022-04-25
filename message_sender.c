#include "message_slot.h"    

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int fd, channel_id, msg_len;
	char *file_path, *msg;
	
	/* validate correct number of arguments */
	if (argc != 4) { 
		perror("invalid number of arguments");
		exit(1); 
	}
	
	/* take arguments */
	file_path = argv[1];
	channel_id = atoi(argv[2]);
	msg = argv[3];
	msg_len = strlen(msg);
		
	/* the flow */
	if ((fd = open(file_path, O_WRONLY)) < 0) { /* open message slot device */
		perror("failed to open file");
		exit(1);
	}
	
	if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) { /* set channel to channel_id */
		perror("failed to set channel");
		exit(1);
	}
	
	if (write(fd, msg, msg_len) < 0) { /* write the specified message to the message slot file */
		perror("failed to write msg to channel");
		exit(1);
	}
	
	/* finish */
	close(fd);
	exit(0);
}
