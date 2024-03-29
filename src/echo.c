//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

static int setup_dest(char* hostname) {
  ping.hostname = hostname;
  ping.dest.sa_family = AF_INET;
  const int retval = hostname_to_sockaddr(ping.hostname, &ping.dest);
  if (retval != 0)
    exit_error(retval == -2 ? "unknown host" : gai_strerror(retval));
  inet_ntop(AF_INET, &((struct sockaddr_in*)&ping.dest)->sin_addr, ping.ip, INET_ADDRSTRLEN);
  return 0;
}

static void setup_icmp(icmphdr* hdr) {
  memset(hdr, 0, sizeof(*hdr));
  hdr->icmp_type = ICMP_ECHO;
  hdr->icmp_code = 0;
  hdr->icmp_id = htons(ping.ident);
  hdr->icmp_seq = htons(ping.num_emit);
  hdr->icmp_cksum = 0;
  hdr->icmp_cksum = checksum((uint16_t*)hdr);
}

static int ping_send() {
  const ssize_t retval = sendto(ping.fd, ping.packet, sizeof(ping.packet), 0, &ping.dest, sizeof(ping.dest));
  if (retval < 0)
    exit_error(NULL);
  ping.num_emit += 1;
  return 0;
}

static int perform_recv() {
  uint8_t buf[1024];
  struct ip* ip = NULL;
  icmphdr* header = NULL;
  struct iovec iov = {.iov_base = buf, .iov_len = sizeof(buf)};

  memset(&ping.msg, 0, sizeof(ping.msg));
  ping.msg.msg_iov = &iov;
  ping.msg.msg_iovlen = 1;
  ssize_t retval = recvmsg(ping.fd, &ping.msg, MSG_DONTWAIT);
  if (retval == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 1;
    exit_error(NULL);
  }
  ping.datalen = retval;
  retval = icmp_decode(buf, retval, &header, &ip);
  if (retval == -1 || retval == 1 || ntohs(header->icmp_id) != ping.ident || header->icmp_type == ICMP_TIME_EXCEEDED || header->icmp_type != ICMP_ECHOREPLY) {
    if (option & OPT_VERBOSE)
      printf("%zu bytes from %s: type = %u, code = %u\n", ping.datalen, ping.ip, header->icmp_type, header->icmp_code);
    return 2;
  }
  ping.recv_ttl = ip->ip_ttl;
  ping.num_recv += 1;
  return 0;
}

static void ping_recv() {
  signal(SIGALRM, sig_handler);
  timeout = false;
  alarm(MAXWAIT);
  while (timeout == false) {
    const int retval = perform_recv();
    if (retval == 0)
      break;
    if (retval == 2)
      return;
  }
  gettimeofday(&ping.current_time, NULL);
  if (stop == true)
    return;
  if (timeout == true) {
    ping.num_rept += 1;
    return;
  }
  printf("%zu bytes from %s: icmp_seq=%zu ttl=%d time=%.3f ms\n", ping.datalen, ping.ip, ping.num_emit - 1,
         ping.recv_ttl,
         get_diff_time(&ping.old_time, &ping.current_time));
}

static void handle_time_wait() {
  const double diffMS = get_diff_time(&ping.old_time, &ping.current_time);
  ping.min = ping.min < diffMS ? ping.min : diffMS;
  ping.max = ping.max > diffMS ? ping.max : diffMS;
  const ssize_t total = ping.num_recv + ping.num_rept;
  ping.avg = (ping.avg * (total - 1) + diffMS) / total;
  ping.sumq += diffMS * diffMS;
  if (diffMS < 1.0 * 1000) {
    const double sleep_time = 1.0 * 1000 - diffMS;
    usleep(sleep_time * 1000000);
  }
}

int ping_echo(char* hostname) {
  setup_dest(hostname);
  printf("PING %s (%s): %zu data bytes", ping.hostname, ping.ip, sizeof(icmphdr) + sizeof(struct ip) + 8);
  if (option & OPT_VERBOSE)
    printf(", id=%#x = %u\n", ping.ident, ping.ident);
  else
    printf("\n");
  while (stop == false) {
    icmphdr icmp_header;
    setup_icmp(&icmp_header);
    memcpy(ping.packet, &icmp_header, sizeof(icmp_header));
    gettimeofday(&ping.old_time, NULL);
    ping_send();
    ping_recv();
    handle_time_wait();
  }
  return 0;
}
