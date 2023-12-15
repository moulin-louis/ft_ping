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

#define MAXWAIT 10
#define OPT_VERBOSE 0x020

int ping_echo(char* hostname, int option);

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;
typedef struct icmphdr icmphdr;

typedef struct {
  int fd;
  int type;
  size_t ping_count;
  timeval start_time;
  size_t interval;
  sockaddr dest;
  char* hostname;
  size_t datalen;
  size_t ident;
  sockaddr from;
  size_t num_emit;
  size_t num_recv;
  size_t num_rept;
} ping_t;

extern ping_t ping;

#endif //FT_PING_H
