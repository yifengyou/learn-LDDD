/*                                                   
 * sound.c   
 * ��¼�Ƽ�������Ƶ���ݣ����������ڴ滺�����У�Ȼ���ٽ��лطţ������еĹ��ܶ���ͨ����д/dev/dsp�豸�ļ������                                        
 */                                                  
#include <unistd.h>                                  
#include <fcntl.h>                                   
#include <sys/types.h>                               
#include <sys/ioctl.h>                               
#include <stdlib.h>                                  
#include <stdio.h>                                   
#include <linux/soundcard.h>                         
#define LENGTH 3    /* �洢���� */                   
#define RATE 8000   /* ����Ƶ�� */                   
#define SIZE 8      /* ����λ�� */                   
#define CHANNELS 1  /* ������Ŀ */                   
/* ���ڱ���������Ƶ���ݵ��ڴ滺���� */               
unsigned char buf[LENGTH*RATE*SIZE*CHANNELS/8];      
int main()                                           
{                                                    
  int fd;	/* �����豸���ļ������� */                 
  int arg;	/* ����ioctl���õĲ��� */                
  int status;   /* ϵͳ���õķ���ֵ */               
  /* �������豸 */                                 
  fd = open("/dev/dsp", O_RDWR);                     
  if (fd < 0) {                                      
    perror("open of /dev/dsp failed");               
    exit(1);                                         
  }                                                  
  /* ���ò���ʱ������λ�� */                         
  arg = SIZE;                                        
  status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);    
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_BITS ioctl failed");     
  if (arg != SIZE)                                   
    perror("unable to set sample size");             
  /* ���ò���ʱ��������Ŀ */                         
  arg = CHANNELS;                                    
  status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed"); 
  if (arg != CHANNELS)                               
    perror("unable to set number of channels");      
  /* ���ò���ʱ�Ĳ���Ƶ�� */                         
  arg = RATE;                                        
  status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);    
  if (status == -1)                                  
    perror("SOUND_PCM_WRITE_WRITE ioctl failed");    
  /* ѭ����ֱ������Control-C */                      
  while (1) {                                        
    printf("Say something:\n");                      
    status = read(fd, buf, sizeof(buf)); /* ¼�� */  
    if (status != sizeof(buf))                       
      perror("read wrong number of bytes");          
    printf("You said:\n");                           
    status = write(fd, buf, sizeof(buf)); /* �ط� */ 
    if (status != sizeof(buf))                       
      perror("wrote wrong number of bytes");         
    /* �ڼ���¼��ǰ�ȴ��طŽ��� */                   
    status = ioctl(fd, SOUND_PCM_SYNC, 0);           
    if (status == -1)                                
      perror("SOUND_PCM_SYNC ioctl failed");         
  }                                                  
}                                                    