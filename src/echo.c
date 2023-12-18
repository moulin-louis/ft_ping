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
  inet_ntop(AF_INET, &((struct sockaddr_in *)&ping.dest)->sin_addr, ping.ip, INET_ADDRSTRLEN);
  return 0;
}

static void setup_icmp(icmphdr* hdr) {
  ft_memset(hdr, 0, sizeof(*hdr));
  hdr->icmp_type = ICMP_ECHO;
  hdr->icmp_code = 0;
  hdr->icmp_id = htons(ping.ident);
  hdr->icmp_seq = htons(ping.num_emit);
  hdr->icmp_cksum = 0;
  hdr->icmp_cksum = checksum((uint16_t *)hdr);
}

static void setup_iphdr(struct ip* ip_hdr) {
  ip_hdr->ip_v = 4;
  ip_hdr->ip_hl = 5;
  ip_hdr->ip_tos = 0;
  ip_hdr->ip_len = htons(sizeof(struct ip) + sizeof(icmphdr));
  ip_hdr->ip_id = ping.num_emit;
  ip_hdr->ip_off = htons(0x4000);
  ip_hdr->ip_ttl = htons(ping.sys_ttl);
  ip_hdr->ip_p = IPPROTO_ICMP;
  ip_hdr->ip_sum = 0;
  ip_hdr->ip_src.s_addr = 0;
  inet_pton(AF_INET, "0.0.0.0", &ip_hdr->ip_src);
  ip_hdr->ip_dst = ((struct sockaddr_in *)&ping.dest)->sin_addr;
  ip_hdr->ip_sum = checksum((uint16_t *)ip_hdr);
}

static int ping_send() {
  const ssize_t retval = sendto(ping.fd, ping.packet, sizeof(ping.packet), 0, &ping.dest, sizeof(ping.dest));
  if (retval < 0)
    exit_error(NULL);
  ping.num_emit += 1;
  return 0;
}

static int perform_recv(const int option) {
  uint8_t buf[1024];
  struct ip* ip = NULL;
  icmphdr* header = NULL;
  struct iovec iov = {.iov_base = buf, .iov_len = sizeof(buf)};

  ft_memset(&ping.msg, 0, sizeof(ping.msg));
  ping.msg.msg_iov = &iov;
  ping.msg.msg_iovlen = 1;
  *__errno_location() = 0;
  ssize_t retval = recvmsg(ping.fd, &ping.msg, MSG_DONTWAIT);
  if (retval == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return -1;
    exit_error(NULL);
  }
  ping.datalen = retval;
  retval = icmp_decode(buf, retval, &header, &ip);
  if (retval == -1) {
    if (option & OPT_VERBOSE) {
      fprintf(stderr, "packet received too short (%zd bytes) from %s\n", retval, ping.ip);
      icmp_error_log();
    }
    return 2;
  }
  if (header->icmp_type == ICMP_TIMXCEED || header->icmp_type == ICMP_TIME_EXCEEDED) {
    char ip_src[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip->ip_src, ip_src, INET_ADDRSTRLEN);
    char hostname_src[NI_MAXHOST];

    ip_to_hostname(ip_src, hostname_src);
    fprintf(stderr, "%zu bytes from %s (%s): Time to live exceeded\n", ping.datalen, hostname_src, ip_src);
    icmp_error_log();
    return 3;
  }
  if (header->icmp_type != ICMP_ECHOREPLY)
    return 4;
  if (retval == 1) {
    if (option & OPT_VERBOSE) {
      fprintf(stderr, "Checksum mismatch from %s\n", ping.ip);
      icmp_error_log();
    }
    return 5;
  }
  if (ntohs(header->icmp_id) != ping.ident) {
    if (option & OPT_VERBOSE) {
      fprintf(stderr, "Wrong identifier from %s\n", ping.ip);
      icmp_error_log();
    }
    return 6;
  }
  ping.recv_ttl = ip->ip_ttl;
  ping.num_recv += 1;
  return 0;
}

static void ping_recv(const int option) {
  alarm(MAXWAIT);
  timeout = false;
  signal(SIGALRM, sig_handler);
  while (timeout == false) {
    const int retval = perform_recv(option);
    if (retval == 0)
      break;
    if (retval == 3)
      return;
  }
  gettimeofday(&ping.current_time, NULL);
  if (stop == true)
    return;
  if (timeout == true) {
    ping.num_rept += 1;
    return;
  }
  printf("%zu bytes from %s: icmp_seq=%zu ttl=%d time=%.3f ms\n", ping.datalen, ping.ip, ping.num_emit - 1, ping.recv_ttl,
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

int ping_echo(char* hostname, const int option) {
  setup_dest(hostname);
  printf("PING %s (%s): %zu data bytes", ping.hostname, ping.ip, sizeof(icmphdr) + sizeof(struct ip) + 8);
  if (option & OPT_VERBOSE)
    printf(", id=%#x = %u\n", ping.ident, ping.ident);
  else printf("\n");
  while (stop == false) {
    icmphdr icmp_header;
    setup_icmp(&icmp_header);
    struct ip ip_hdr;
    setup_iphdr(&ip_hdr);
    memcpy(ping.packet, &ip_hdr, sizeof(ip_hdr));
    memcpy(ping.packet + sizeof(ip_hdr), &icmp_header, sizeof(icmp_header));
    ping.icmp_header = icmp_header;
    gettimeofday(&ping.old_time, NULL);
    ping_send();
    ping_recv(option);
    handle_time_wait();
  }
  return 0;
}
