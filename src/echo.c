//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

static uint16_t checksum(uint16_t* addr) {
  int nleft = 8;
  int sum = 0;
  uint16_t* w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(uint8_t *)(&answer) = *(uint8_t *)w;
    sum += answer;
  }
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

static int setup_dest() {
  struct addrinfo hints;
  struct addrinfo* result;

  ft_memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  const int retval = getaddrinfo(ping.hostname, NULL, &hints, &result);
  if (retval != 0) {
    printf("Error: getaddrinfo() failed: %s\n", gai_strerror(retval));
    exit(1);
  }
  ft_memcpy(&ping.dest, result->ai_addr, sizeof(ping.dest));
  inet_ntop(AF_INET, &((struct sockaddr_in *)&ping.dest)->sin_addr, ping.ip, INET_ADDRSTRLEN);
  freeaddrinfo(result);
  return 0;
}

static int ping_send(const icmphdr* icmp_header) {
  const ssize_t retval = sendto(ping.fd, icmp_header, sizeof(*icmp_header), 0, &ping.dest, sizeof(ping.dest));
  if (retval < 0) {
    printf("Error: sendto() failed: %s\n", strerror(errno));
    exit(1);
  }
  ping.num_emit += 1;
  return 0;
}

static int32_t icmp_decode(const uint8_t* buf, const size_t len, icmphdr** header, struct ip** ip) {
  struct ip* ip_tmp = (struct ip *)buf;
  const size_t hlen = ip_tmp->ip_hl << 2;
  if (len < hlen + ICMP_MINLEN) {
    return -1;
  }

  icmphdr* icmp_header = (icmphdr *)(buf + hlen);
  *header = icmp_header;
  *ip = ip_tmp;
  const uint16_t chksum = (*header)->checksum;
  (*header)->checksum = 0;
  (*header)->checksum = checksum((uint16_t *)*header);
  if (chksum != (*header)->checksum)
    return 1;
  return 0;
}

static void ping_recv(const int option) {
  alarm(1);
  timeout = false;
  signal(SIGALRM, sig_handler);
  while (stop == false && timeout == false) {
    uint8_t buf[1024];
    struct ip* ip = NULL;
    icmphdr* header = NULL;
    struct iovec iov = {.iov_base = buf, .iov_len = sizeof(buf)};

    ft_memset(&ping.msg, 0, sizeof(ping.msg));
    ping.msg.msg_iov = &iov;
    ping.msg.msg_iovlen = 1;
    ssize_t retval = recvmsg(ping.fd, &ping.msg, MSG_DONTWAIT);
    if (timeout == true)
      break;
    if (retval == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        continue;
      printf("Error: recvfrom() failed: %s\n", strerror(errno));
      close(ping.fd);
      exit(1);
    }
    retval = icmp_decode(buf, retval, &header, &ip);
    if (retval == -1) {
      if (option & OPT_VERBOSE)
        fprintf(stderr, "packet received too short (%zd bytes) from %s\n", retval, ping.ip);
      break;
    }
    if (header->type != ICMP_ECHOREPLY) {
      break;
    }
    if (retval == 1) {
      if (option & OPT_VERBOSE)
        fprintf(stderr, "Checksum mismatch from %s\n", ping.ip);
      break;
    }
    if (ntohs(header->id) != ping.ident) {
      if (option & OPT_VERBOSE)
        fprintf(stderr, "Wrong identifier from %s\n", ping.ip);
      break;
    }
    ping.ttl = ip->ip_ttl;
    ping.num_recv += 1;
  }
  if (timeout == true) {
    if (option & OPT_VERBOSE)
      printf("Request timeout for icmp_seq %zu\n", ping.num_emit);
    ping.num_rept += 1;
    return;
  }
  if (stop == true)
    return;
  gettimeofday(&ping.current_time, NULL);
  printf("%zu bytes from %s: icmp_seq=%zu ttl=%d time=%.3f ms\n", sizeof(icmphdr), ping.ip, ping.num_emit - 1,
           ping.ttl, get_diff_time(&ping.old_time, &ping.current_time));
  ping.ttl = -1;
}

int ping_echo(char* hostname, const int option) {
  ping.hostname = hostname;
  ping.dest.sa_family = AF_INET;
  gettimeofday(&ping.start_time, NULL);
  setup_dest();
  printf("PING %s (%s): %zu data bytes", ping.hostname, ping.ip, sizeof(icmphdr));
  if (option & OPT_VERBOSE) {
    printf(", id=%#x = %u\n", ping.ident, ping.ident);
  }
  else {
    printf("\n");
  }
  while (stop == false) {
    icmphdr icmp_header;
    ft_memset(&icmp_header, 0, sizeof(icmp_header));
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.id = htons(ping.ident);
    icmp_header.seq = htons(ping.num_emit);
    icmp_header.checksum = 0;
    icmp_header.checksum = checksum((uint16_t *)&icmp_header);
    gettimeofday(&ping.old_time, NULL);
    ping_send(&icmp_header);
    ping_recv(option);
    const double diff = get_diff_time(&ping.old_time, &ping.current_time);
    if (diff < 1.0) {
      const double sleep_time = 1.0 - diff;
      usleep(sleep_time * 1000000);
    }
  }
  return 0;
}
