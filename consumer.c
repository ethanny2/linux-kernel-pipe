#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLEN 100

int main(int argc, char *argv[])
{

	int fd;
	char numstr[MAXLEN];
	int num_read;

	if( argc != 1) {
		printf("Usage: %s  ", argv[0]);
		exit(0);
	}

	if ( (fd = open("/dev/numpipe", O_RDONLY)) < 0) {
		perror(""); printf("error opening %s\n", argv[1]);
		exit(0);
	}

	while(1) {
		// read a line
		ssize_t ret = read(fd, &num_read, sizeof(int));
		if( ret > 0) {
			//printf("Number read: %d ", ret);
			printf("*************************\n");
			printf("Number read from buffer device: %d\n",num_read);
			printf("Bytes read: %ld\n", ret);
			printf("*************************\n");
		} else {
			fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
			perror("");
			sleep(1);
		}

	}
	close(fd);

	return 0;
}

