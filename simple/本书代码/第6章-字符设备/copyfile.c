#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char* argv[]){
	FILE* fd1,*fd2;	/*�ļ�ָ��*/
	char buffer[1024]; /*���ݻ�����*/
	int num;

	if(argc!=3){
		printf("Usage : %s source-file dest-file",argv[0]);
		return 1;
	}

	if((fd1=fopen(argv[1],"r"))==NULL){/*�Զ���ʽ���ļ��������Ż�����δ���*/
		perror("Cannot open the file");
		return 1;
	}

	if((fd2=fopen(argv[2],"w"))==NULL){/*��д��ʽ���ļ�*/
		perror("Cannot create the destination file");
		return 1;
	}

    while(!feof(fd1))/*�����ļ�*/
	{
		num=fread(buffer,sizeof(char),1024,fd1);
		if(fwrite(buffer,sizeof(char),num,fd2)<num){
			perror("Cannot write the file content to the file");
			return 1;
		}
	}
	
	fclose(fd1);/*�ر��ļ�*/
	fclose(fd2);
	printf("copy file1 to file2 success\n");
	return 0;
}
