//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

static int setup_dest(char* hostname) {
  struct addrinfo hints;
  struct addrinfo* result;

  ping.hostname = hostname;
  ping.dest.sa_family = AF_INET;
  gettimeofday(&ping.start_time, NULL);
  ft_memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  const int retval = getaddrinfo(ping.hostname, NULL, &hints, &result);
  if (retval != 0) {
    if (retval == -2)
      fprintf(stderr, "./ft_ping: unknown host\n");
    else
      fprintf(stderr, "./ft_ping: %s\n", gai_strerror(retval));
    close(ping.fd);
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
    printf("./ft_ping: %s\n", strerror(errno));
    close(ping.fd);
    exit(1);
  }
  ping.num_emit += 1;
  return 0;
}

static void setup_icmp(icmphdr* hdr) {
  ft_memset(hdr, 0, sizeof(*hdr));
  hdr->type = ICMP_ECHO;
  hdr->code = 0;
  hdr->id = htons(ping.ident);
  hdr->seq = htons(ping.num_emit);
  hdr->checksum = 0;
  hdr->checksum = checksum((uint16_t *)hdr);
}

static int perform_recv(const int option) {
  uint8_t buf[1024];
  struct ip* ip = NULL;
  icmphdr* header = NULL;
  struct iovec iov = {.iov_base = buf, .iov_len = sizeof(buf)};

  ft_memset(&ping.msg, 0, sizeof(ping.msg));
  ping.msg.msg_iov = &iov;
  ping.msg.msg_iovlen = 1;
  ssize_t retval = recvmsg(ping.fd, &ping.msg, MSG_DONTWAIT);
  if (retval == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return -1;
    printf("./ft_ping: %s\n", strerror(errno));
    close(ping.fd);
    exit(1);
  }
  ping.datalen = retval;
  retval = icmp_decode(buf, retval, &header, &ip);
  if (retval == -1) {
    if (option & OPT_VERBOSE)
      fprintf(stderr, "packet received too short (%zd bytes) from %s\n", retval, ping.ip);
    return 2;
  }
  if (header->type != ICMP_ECHOREPLY)
    return 3;
  if (retval == 1) {
    if (option & OPT_VERBOSE)
      fprintf(stderr, "Checksum mismatch from %s\n", ping.ip);
    return 4;
  }
  if (ntohs(header->id) != ping.ident) {
    if (option & OPT_VERBOSE)
      fprintf(stderr, "Wrong identifier from %s\n", ping.ip);
    return 5;
  }
  ping.ttl = ip->ip_ttl;
  ping.num_recv += 1;
  return 0;
}

static void ping_recv(const int option) {
  alarm(1);
  timeout = false;
  signal(SIGALRM, sig_handler);
  while (timeout == false) {
    const int retval = perform_recv(option);
    if (retval == 0) {
      break;
    }
  }
  if (stop == true)
    return;
  if (timeout == true) {
    if (option & OPT_VERBOSE)
      printf("Request timeout for icmp_seq %zu\n", ping.num_emit);
    ping.num_rept += 1;
    return;
  }
  gettimeofday(&ping.current_time, NULL);
  printf("%zu bytes from %s: icmp_seq=%zu ttl=%d time=%.3f ms\n", ping.datalen, ping.ip, ping.num_emit - 1, ping.ttl,
         get_diff_time(&ping.old_time, &ping.current_time));
}

int ping_echo(char* hostname, const int option) {
  setup_dest(hostname);
  printf("PING %s (%s): %zu data bytes", ping.hostname, ping.ip, sizeof(icmphdr) + sizeof(struct ip) + 8);
  if (option & OPT_VERBOSE)
    printf(", id=%#x = %u\n", ping.ident, ping.ident);
  else printf("\n");
  while (stop == false) {
    icmphdr icmp_header;
    setup_icmp(&icmp_header);
    gettimeofday(&ping.old_time, NULL);
    ping_send(&icmp_header);
    ping_recv(option);
    const double diffMS = get_diff_time(&ping.old_time, &ping.current_time);
    if (diffMS < (1.0 * 1000.0)) {
      const double sleep_time = 1.0 - diffMS;
      usleep(sleep_time * 1000000);
    }
  }
  return 0;
}
