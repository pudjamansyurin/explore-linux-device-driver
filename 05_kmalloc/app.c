#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 1024

enum ops {
	ops_read,
	ops_write,
};

static char wr_buf[BUF_SIZE];
static char rd_buf[BUF_SIZE];

static void process(enum ops operation, int fd, char *buf, size_t sz);

int main()
{
	int fd;
	char answer;

	fd = open("/dev/alman_device", O_RDWR);
	if (fd < 0) {
		printf("Can't open device file\n");
		return -1;
	}

	while (1) {
		printf("\n\nWhat do you want?\n");
		printf("1. Write\n");
		printf("2. Read\n");
		printf("3. Exit\n");
		printf("Anwser = ");
		scanf(" %c", &answer);

		switch (answer) {
		case '1':
			printf("Input: ");
			scanf(" %[^\t\n]s", wr_buf);
			process(ops_write, fd, wr_buf, strlen(wr_buf) + 1);
			break;
		case '2':
			process(ops_read, fd, rd_buf, BUF_SIZE);
			break;
		case '3':
			close(fd);
			exit(1);
			break;
		default:
			printf("Invalid answer of %c\n", answer);
			break;
		}
	}

	close(fd);
}

static void process(enum ops operation, int fd, char *buf, size_t sz)
{
	int reading = operation == ops_read;

	printf("Data %s... ", reading ? "reading" : "writing");
	if (reading)
		read(fd, buf, sz);
	else
		write(fd, buf, sz);
	printf("Done!\n");
	printf("%c %s\n", reading ? '<' : '>', buf);
}
