#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define REG_CURRENT_TASK _IOW('a', 'a', int32_t*)
#define SIGETX 44

/* Private variables */
static int quit = 0;

/* Private function prototypes */
static void ctrl_c_handler(int n, siginfo_t *info, void *unused);
static void sig_event_handler(int n, siginfo_t *info, void *unused);

/* Main function */
int main()
{
	int fd;
	int32_t value, number;
  struct sigaction act;

  /* set crtl+c handler */
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO | SA_RESETHAND;
  act.sa_sigaction = ctrl_c_handler;
  sigaction(SIGINT, &act, NULL);
  printf("Installed SIGINT = %d\n", SIGINT);
  
  act.sa_flags = SA_SIGINFO | SA_RESTART;
  act.sa_sigaction = sig_event_handler;
  sigaction(SIGETX, &act, NULL);
  printf("Installed SIGETX = %d\n", SIGETX);

  printf("Openning driver\n");
	fd = open("/dev/alman_device", O_RDWR);
	if (fd < 0) 
	{
		printf("Can't open device file\n");
		return -1;
	}

  printf("Registering app\n");
  if (ioctl(fd, REG_CURRENT_TASK, NULL)) 
  {
    printf("Failed call to ioctl\n");
    close(fd);
    return -1;
  }
	
  printf("Wait for signal...\n");
  while(!quit);

	printf("Closing driver\n");
  close(fd);
}

/* Private function definitions */
void ctrl_c_handler(int n, siginfo_t *info, void *unused)
{
  if (n != SIGINT) 
    return;
    
  printf("Received signal from app: ctrl-c\n");
  quit = 1;
}
 
void sig_event_handler(int n, siginfo_t *info, void *unused)
{
  if (n != SIGETX)
    return;
    
  printf ("Received signal from kernel: Value =  %u\n", info->si_int);
}
 
