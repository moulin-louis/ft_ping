//
// Created by loumouli on 12/15/23.
//

#ifndef FT_PING_H
#define FT_PING_H

#include <libft.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#define MAXWAIT 10
#define OPT_VERBOSE 0x020

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;

typedef struct {
  int fd;
  int type;
  size_t ping_count;
  timeval start_time;
  timeval old_time;
  timeval current_time;
  size_t interval;
  sockaddr dest;
  char* hostname;
  char ip[INET_ADDRSTRLEN];
  size_t datalen;
  uint16_t ident;
  struct msghdr msg;
  int32_t ttl;
  size_t num_emit;
  size_t num_recv;
  size_t num_rept;
} ping_t;

typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t id;
  uint16_t seq;
} icmphdr;

extern ping_t ping;
extern volatile char stop;
extern bool timeout;

int ping_echo(char* hostname, int option);

double get_diff_time(const timeval* s1, const timeval* s2);

void sig_handler(int signal);

uint16_t checksum(uint16_t* addr);

int32_t icmp_decode(const uint8_t* buf, size_t len, icmphdr** header, struct ip** ip);

#endif //FT_PING_H
