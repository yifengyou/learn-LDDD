#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

main()
{
	int fd,size;
	char s[]="Linux Programmer!\n";
	char buffer[80];

	fd=open("/dev/gatieme",O_WRONLY|O_CREAT);/*�Զ�д��ʽ���ļ�*/
	if(fd<=-1)
	{
		perror("open file error\n");
	}

	write(fd,s,sizeof(s));	/*д���ݵ��ļ�*/
	close(fd);

	fd=open("/dev/gatieme",O_RDONLY);
	if(fd<=-1)
	{
		perror("open file error\n");
	}

	size=read(fd,buffer,sizeof(buffer));/*���ļ���������*/
	close(fd);

	printf("%s\n",buffer);
}
