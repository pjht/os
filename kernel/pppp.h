#ifndef PPP_H
#define PPP_H

#include <stdint.h>

void pppp_init();
uint32_t udp_new();
void udp_send(uint32_t sock_id,char* ip,uint16_t port,uint16_t len,void* datav);
void* udp_recv(uint32_t sock_id,uint32_t* len);
uint32_t tcp_new(char* ip,uint16_t port);
void tcp_send(uint32_t sock_id,uint16_t len,void* datav);
void* tcp_recv(uint32_t sock_id,uint32_t* len);
#endif
