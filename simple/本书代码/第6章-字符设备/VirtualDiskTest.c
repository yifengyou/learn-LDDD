/*
 * File Name: gatiemeTest.c
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void main()
{
   int fileno;/*�����ļ�������*/
   int number;
   char data[]="one two three four five six";/*д��gatieme������*/
   char str[1024];/*�û��ռ仺��*/
   int len;

   fileno = open("/dev/gatieme",O_RDWR);/*�Զ�д��ʽ���豸�ļ�*/

   if (fileno == -1)/*���ļ�ʧ��*/
   {
   	printf("open gatieme device errr!\n");
	return 0;
   }

   write(fileno,data,strlen(data));/*������д���豸*/
   close(fileno);/*�ر��豸�ļ�*/

   fileno=open("/dev/gatieme",O_RDWR);/*�Զ�д��ʽ���豸�ļ�*/
   len=read(fileno,str,1024);/*�����豸�е�����*/
   str[len]='\0';
   printf("%s\n",str);/*��ʾ�豸������*/
   close(fileno);/*�ر��豸�ļ�*/

   fileno=open("/dev/gatieme",O_RDWR);/*�Զ�д��ʽ���豸�ļ�*/
   lseek(fileno,4,SEEK_SET);/*���ļ�ָ�����4�ֽڣ���ǰλ�õ��ַ�Ϊt*/
   len=read(fileno,str,1024);/*�����豸�е�����*/
   str[len]='\0';
   printf("%s\n",str);/*��ʾ�豸������*/
   close(fileno);/*�ر��豸�ļ�*/

   return 0;
}
