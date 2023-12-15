//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

static uint16_t checksum(uint16_t *addr, const int len) {
  int nleft = len;
  int sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(uint8_t *) (&answer) = *(uint8_t *) w;
    sum += answer;
  }
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

static int setup_dest() {
  struct addrinfo hints;
  struct addrinfo *result;
  ft_memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;
  int retval = getaddrinfo(ping.hostname, NULL, &hints, &result);
  if (retval != 0) {
    printf("Error: getaddrinfo() failed: %s\n", gai_strerror(retval));
    exit(1);
  }
  struct sockaddr_in *sockaddr_in = (struct sockaddr_in *)result->ai_addr;
  ping.dest = *(sockaddr*)sockaddr_in;
  return 0;
}

static int ping_send(const icmphdr* icmp_header) {
  (void)icmp_header;
  ssize_t retval = sendto(ping.fd, icmp_header, sizeof(*icmp_header), 0, &ping.dest, sizeof(ping.dest));
  if (retval < 0) {
    printf("Error: sendto() failed: %s\n", strerror(errno));
    exit(1);
  }
  dprintf(1, "retval: %zd\n", retval);
  return 0;
}

static int ping_recv(void) {
  return 0;
}

int ping_echo(char *hostname, int option) {
  printf("Will try to send echo request to %s\n", hostname);
  (void)option;
  ping.hostname = hostname;
  ping.dest.sa_family = AF_INET;
  ping.ident = getpid();
  setup_dest();
  while (true) {
    icmphdr icmp_header;
    ft_memset(&icmp_header, 0, sizeof(icmp_header));
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.un.echo.id = htonl(getpid());
    icmp_header.un.echo.sequence = htonl(ping.num_emit);
    icmp_header.checksum = 0;
    icmp_header.checksum = checksum((uint16_t *)&icmp_header, sizeof(icmp_header));
    ping_send(&icmp_header);
    ping.num_emit += 1;
    ping_recv();
    break;
  }

  return 0;
}