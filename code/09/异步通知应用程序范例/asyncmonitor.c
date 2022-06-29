/*======================================================================
    A test program to access /dev/second
    This example is to help understand async IO

    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

/*���յ��첽���źź�Ķ���*/
void input_handler(int signum)
{
  printf("receive a signal from globalfifo,signalnum:%d\n",signum);
}

main()
{
  int fd, oflags;
  fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
  if (fd !=  - 1)
  {
    //�����ź���������
    signal(SIGIO, input_handler); //��input_handler()����SIGIO�ź�
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);
    while(1)
    {
    	sleep(100);
    }
  }
  else
  {
    printf("device open failure\n");
  }
}
