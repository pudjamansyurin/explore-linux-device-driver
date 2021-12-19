#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#define WR_VALUE _IOW('a', 'a', int32_t*)
#define RD_VALUE _IOR('a', 'b', int32_t*)

int main()
{
	int fd;
	int32_t value, number;

	fd = open("/dev/alm_device", O_RDWR);
	if (fd < 0) 
	{
		printf("Can't open device file\n");
		return -1;
	}

	printf("> ");
	scanf(" %d", &number);
	printf("Writing value to driver\n");
	ioctl(fd, WR_VALUE, (int32_t*) &number);

	printf("Reading value from driver\n");
	ioctl(fd, RD_VALUE, (int32_t*) &value);
	printf("< %d\n", value);
	
	printf("Closing driver\n");
	close(fd);
}


