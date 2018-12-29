#include "../libc/stdio.h"
#include "../libc/stdlib.h"
#include "../libc/string.h"
#include "klog.h"
#include <stdint.h>
#include "pppp.h"
FILE* ser0;


#define NTP_OFFSET 2208988800
#define SECS_YEAR 31556952
#define SECS_MONTH 2629746
#define SECS_DAY 86400
#define SECS_HOUR 3600
#define SECS_MIN 60

void send_16(uint16_t num) {
  fputc(num&0xFF,ser0);
  fputc((num&0xFF00)>>8,ser0);
}

void send_32(uint32_t num) {
  fputc(num&0xFF,ser0);
  fputc((num&0xFF00)>>8,ser0);
  fputc((num&0xFF0000)>>16,ser0);
  fputc((num&0xFF000000)>>24,ser0);
}

void read_32() {

}


void pppp_init() {
  ser0=fopen("/dev/ttyS0","r+");
  if (ser0==NULL) {
    klog("ERROR","No serial port, cannot initialize PPPP driver");
  }
  uint8_t ip[4];
  fputc(0,ser0);
  fread(&ip,sizeof(uint8_t),4,ser0);
  klog("INFO","PPPP IP:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
  uint32_t sock_id=udp_new();
  char* url="example.com";
  char* labels[4];
  for (int i=0;i<4;i++) {
    labels[i]=malloc(strlen(url)+1);
    labels[i][0]='\0';
  }
  int i=0;
  int label_offset=0;
  for (int j=0;j<strlen(url);j++) {
    char ch=url[j];
    if (ch=='.') {
      i++;
      label_offset=0;
    } else {
      labels[i][label_offset]=ch;
      label_offset++;
    }
  }
  char* header="\xaa\xaa\x01\0\0\x01\0\0\0\0\0\0";
  char* footer="\0\0\x01\0\x01";
  fputc(4,ser0);
  send_32(sock_id);
  fputc(8,ser0);
  fputc(8,ser0);
  fputc(8,ser0);
  fputc(8,ser0);
  send_16(53);
  send_16(12+strlen(url)+1+5);
  for(uint16_t i=0;i<12;i++) {
    fputc(header[i],ser0);
  }
  for (int i=0;i<4;i++) {
    if (labels[i][0]=='\0') {
      continue;
    }
    fputc(strlen(labels[i]),ser0);
    fputs(labels[i],ser0);
  }
  for(uint16_t i=0;i<5;i++) {
    fputc(footer[i],ser0);
  }
  uint32_t length;
  char* packet=udp_recv(sock_id,&length);
  ip[3]=packet[length-1];
  ip[2]=packet[length-2];
  ip[1]=packet[length-3];
  ip[0]=packet[length-4];
  printf("IP of %s is %d.%d.%d.%d\n",url,ip[0],ip[1],ip[2],ip[3]);
  uint16_t http_sock=tcp_new(ip,80);
  char* msg="GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
  tcp_send(http_sock,strlen(msg),msg);
  int in_headers=1;
  char* line;
  while (1) {
    printf("GET");
    line=tcp_recv(http_sock,&length);
    if (!line) {
      printf("DONE\n");
      break;
    }
    printf("LINE",line);
  }
}

uint32_t udp_new() {
  fputc(1,ser0);
  uint32_t info[2];
  fread(&info,sizeof(uint32_t),2,ser0);
  return info[0];
}

void udp_send(uint32_t sock_id,char* ip,uint16_t port,uint16_t len,void* datav) {
  char* data=(char*)datav;
  fputc(4,ser0);
  send_32(sock_id);
  fputc(ip[0],ser0);
  fputc(ip[1],ser0);
  fputc(ip[2],ser0);
  fputc(ip[3],ser0);
  send_16(port);
  send_16(len);
  for(uint16_t i=0;i<len;i++) {
    fputc(data[i],ser0);
  }
}

void* udp_recv(uint32_t sock_id,uint32_t* len) {
  fputc(5,ser0);
  send_32(sock_id);
  fread(len,sizeof(uint32_t),1,ser0);
  void* data=malloc(*len);
  fread(data,*len,1,ser0);
  return data;
}

uint32_t tcp_new(char* ip,uint16_t port) {
  fputc(7,ser0);
  fputc(ip[0],ser0);
  fputc(ip[1],ser0);
  fputc(ip[2],ser0);
  fputc(ip[3],ser0);
  send_16(port);
  uint32_t id;
  fread(&id,sizeof(uint32_t),1,ser0);
  return id;
}

void tcp_send(uint32_t sock_id,uint16_t len,void* datav) {
  char* data=(char*)datav;
  fputc(8,ser0);
  send_32(sock_id);
  send_16(len);
  for(uint16_t i=0;i<len;i++) {
    fputc(data[i],ser0);
  }
}

void* tcp_recv(uint32_t sock_id,uint32_t* len) {
  fputc(9,ser0);
  send_32(sock_id);
  fread(len,sizeof(uint32_t),1,ser0);
  if (len==0) {
    return NULL;
  }
  void* data=malloc(*len);
  // printf("Reading %d bytes\n",*len);
  fread(data,*len,1,ser0);
  return data;
}
