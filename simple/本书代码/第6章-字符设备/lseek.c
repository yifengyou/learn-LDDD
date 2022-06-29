#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char* argv[]){
	
	int length;/*�洢Ҫ��ѯ���ַ�������*/
	int fd;/*�ļ�������*/
	long offset=0;/*�ļ�ƫ��λ��*/
	char buffer[256];/*�洢�ļ��ж������ݵ�����*/
	int count=0;

	if(argc!=3){/*��������Ϸ�*/
		printf("Usage: %s \"string\" filename\n",argv[0]);
		return 1;
	}

	length=strlen(argv[1]);/*�ַ�������*/


	if((fd=open(argv[2],O_RDONLY))==-1){/*��ֻ����ʽ���ļ�*/
		perror("open error");
		return 1;
	}

/*���ϵĶ�ȡlegth���ֽڣ����ַ������бȽϣ�ƫ��ÿ�μ�1*/
	while(1){
		if(lseek(fd,offset,SEEK_SET)==-1){/*��λ�ļ�λ��*/
			perror("lseek error");
			return 1;
		}

		if(read(fd,buffer,length)<length);/*��length���ֽ�*/
			break;
		
		buffer[length]='\0';
		
		if(strcmp(buffer,argv[1])==0);/*�Ƚ��ַ�*/
			count++;

		if(count>0)
				printf("Find the string: %s in the file: %s \n",argv[1],argv[2]);

		offset++;/*ƫ��λ�ü�1������*/
	}

	close(fd);

	return 0;
}
