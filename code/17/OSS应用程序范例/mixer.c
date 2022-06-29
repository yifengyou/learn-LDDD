/*                                                                   
 * mixer.c  
 * �Ը��ֻ���ͨ����������е��ڣ������еĹ��ܶ�ͨ����д/dev/mixer�豸�ļ������                                                         
 */                                                                  
#include <unistd.h>                                                  
#include <stdlib.h>                                                  
#include <stdio.h>                                                   
#include <sys/ioctl.h>                                               
#include <fcntl.h>                                                   
#include <linux/soundcard.h>                                         
/* �����洢���п��û����豸������ */                                 
const char *sound_device_names[] = SOUND_DEVICE_NAMES;               
int fd;                  /* �����豸����Ӧ���ļ������� */            
int devmask, stereodevs; /* ��������Ϣ��Ӧ��λͼ���� */              
char *name;                                                          
/* ��ʾ�����ʹ�÷��������п��õĻ����豸 */                         
void usage()                                                         
{                                                                    
  int i;                                                             
  fprintf(stderr, "usage: %s <device> <left-gain%%> <right-gain%%>\n"
	  "       %s <device> <gain%%>\n\n"                                
	  "Where <device> is one of:\n", name, name);                      
  for (i = 0 ; i < SOUND_MIXER_NRDEVICES ; i++)                      
    if ((1 << i) & devmask) /* ֻ��ʾ��Ч�Ļ����豸 */               
      fprintf(stderr, "%s ", sound_device_names[i]);                 
  fprintf(stderr, "\n");                                             
  exit(1);                                                           
}                                                                    
int main(int argc, char *argv[])                                     
{                                                                    
  int left, right, level;  /* �������� */                            
  int status;              /* ϵͳ���õķ���ֵ */                    
  int device;              /* ѡ�õĻ����豸 */                      
  char *dev;               /* �����豸������ */                      
  int i;                                                             
  name = argv[0];                                                    
  /* ��ֻ����ʽ�򿪻����豸 */                                       
  fd = open("/dev/mixer", O_RDONLY);                                 
  if (fd == -1) {                                                    
    perror("unable to open /dev/mixer");                             
    exit(1);                                                         
  }                                                                  
                                                                     
  /* �������Ҫ����Ϣ */                                             
  status = ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask);            
  if (status == -1)                                                  
    perror("SOUND_MIXER_READ_DEVMASK ioctl failed");                 
  status = ioctl(fd, SOUND_MIXER_READ_STEREODEVS, &stereodevs);      
  if (status == -1)                                                  
    perror("SOUND_MIXER_READ_STEREODEVS ioctl failed");              
  /* ����û����� */                                                 
  if (argc != 3 && argc != 4)                                        
    usage();                                                         
  /* �����û�����Ļ��������� */                                     
  dev = argv[1];                                                     
  /* ȷ�������õ��Ļ����豸 */                                       
  for (i = 0 ; i < SOUND_MIXER_NRDEVICES ; i++)                      
    if (((1 << i) & devmask) && !strcmp(dev, sound_device_names[i])) 
      break;                                                         
  if (i == SOUND_MIXER_NRDEVICES) { /* û���ҵ�ƥ���� */             
    fprintf(stderr, "%s is not a valid mixer device\n", dev);        
    usage();                                                         
  }                                                                  
  /* ���ҵ���Ч�Ļ����豸 */                                         
  device = i;                                                        
  /* ��ȡ����ֵ */                                                   
  if (argc == 4) {                                                   
    /* �������������� */                                           
    left  = atoi(argv[2]);                                           
    right = atoi(argv[3]);                                           
  } else {                                                           
    /* ����������Ϊ��� */                                         
    left  = atoi(argv[2]);                                           
    right = atoi(argv[2]);                                           
  }                                                                  
                                                                     
  /* �Է��������豸����������Ϣ */                                   
  if ((left != right) && !((1 << i) & stereodevs)) {                 
    fprintf(stderr, "warning: %s is not a stereo device\n", dev);    
  }                                                                  
                                                                     
  /* ������������ֵ�ϵ�ͬһ������ */                                 
  level = (right << 8) + left;                                       
                                                                     
  /* �������� */                                                     
  status = ioctl(fd, MIXER_WRITE(device), &level);                   
  if (status == -1) {                                                
    perror("MIXER_WRITE ioctl failed");                              
    exit(1);                                                         
  }                                                                  
  /* ��ô��������ص��������������� */                               
  left  = level & 0xff;                                              
  right = (level & 0xff00) >> 8;                                     
  /* ��ʾʵ�����õ����� */                                           
  fprintf(stderr, "%s gain set to %d%% / %d%%\n", dev, left, right); 
  /* �رջ����豸 */                                                 
  close(fd);                                                         
  return 0;                                                          
}                                                                    