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
  printf("hostname: %s\n", ping.hostname);
  int retval = getaddrinfo(ping.hostname, NULL, &hints, &result);
  if (retval != 0) {
    printf("Error: getaddrinfo() failed: %s\n", gai_strerror(retval));
    exit(1);
  }
  ft_memcpy(&ping.dest, result->ai_addr, sizeof(ping.dest));
  printf("ip addres = %s\n", inet_ntoa(((struct sockaddr_in *)&ping.dest)->sin_addr));
  freeaddrinfo(result);
  return 0;
}

static int ping_send(const icmphdr* icmp_header) {
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
  const pid_t pid = getpid();
  while (true) {
    icmphdr icmp_header;
    ft_memset(&icmp_header, 0, sizeof(icmp_header));
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    // dprintf(1, "BEFORE WRITING UNIIN:\n");
    // hexdump(&icmp_header, sizeof(icmp_header),0);
    printf("htonl pid = %d\n", htons(pid));
    icmp_header.id = htons(pid);
    // dprintf(1, "AFTEr WRITING UNIIN:\n");
    // hexdump(&icmp_header, sizeof(icmp_header),0);
    icmp_header.seq = htons(ping.num_emit);
    icmp_header.checksum = 0;
    icmp_header.checksum = checksum((uint16_t *)&icmp_header, sizeof(icmp_header));
    dprintf(1, "BEFORE SENDING:\n");
    hexdump(&icmp_header, sizeof(icmp_header),0);
    ping_send(&icmp_header);
    ping.num_emit += 1;
    ping_recv();
    sleep(1);
  }

  return 0;
}