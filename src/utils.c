//
// Created by loumouli on 12/16/23.
//

#include "ft_ping.h"

// get the difference between two timeval in milliseconds
double get_diff_time(const timeval* s1, const timeval* s2) {
  return (double)(s2->tv_sec - s1->tv_sec) * 1000.0 + (double)(s2->tv_usec - s1->tv_usec) / 1000.0;
}

uint16_t checksum(uint16_t* addr) {
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

int32_t icmp_decode(const uint8_t* buf, const size_t len, icmphdr** header, struct ip** ip) {
  struct ip* ip_tmp = (struct ip *)buf;
  const size_t hlen = ip_tmp->ip_hl << 2;
  if (len < hlen + ICMP_MINLEN) {
    return -1;
  }

  icmphdr* icmp_header = (icmphdr *)(buf + hlen);
  *header = icmp_header;
  *ip = ip_tmp;
  const uint16_t chksum = (*header)->icmp_cksum;
  (*header)->icmp_cksum = 0;
  (*header)->icmp_cksum = checksum((uint16_t *)*header);
  if (chksum != (*header)->icmp_cksum)
    return 1;
  return 0;
}

//RESULT SHOULD ALREADY BE ALOCATEDd
int32_t hostname_to_sockaddr(const char* hostname, void* result_ptr) {
  struct addrinfo hints;
  struct addrinfo* result;

  ft_memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  const int retval = getaddrinfo(hostname, NULL, &hints, &result);
  if (retval != 0)
    return retval;
  ft_memcpy(result_ptr, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
  return 0;
}

//convert ip to hostname
int32_t ip_to_hostname(const char* ip, char* result_str) {
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(sa.sin_addr));
  const int retval = getnameinfo((struct sockaddr *)&sa, sizeof(sa), result_str, NI_MAXHOST, NULL, 0, 0);
  if (retval != 0)
    return retval;
  return 0;
}

void exit_error(const char* msg) {
  fprintf(stderr, "./ft_ping: %s\n", msg ? msg : strerror(errno));
  close(ping.fd);
  exit(1);
}

double nabs(const double a) {
  return (a < 0) ? -a : a;
}

double nsqrt(const double a, const double prec) {
  double x0;

  if (a < 0)
    return 0;
  if (a < prec)
    return 0;
  double x1 = a / 2;
  do {
    x0 = x1;
    x1 = (x0 + a / x0) / 2;
  }
  while (nabs(x1 - x0) > prec);

  return x1;
}

void icmp_hexdump(void *data, const size_t len) {
  for (size_t j = 0; j < len; ++j)
    printf ("%02x%s", *((unsigned char *) data + j), (j % 2) ? " " : "");	/* Group bytes two by two.  */
}

void icmp_error_log(struct ip* ip, const icmphdr* header) {
  size_t hlen = ip->ip_hl << 2;
  const uint8_t* cp = (uint8_t*)header;
  printf("IP Hdr Dump:\n ");
  icmp_hexdump(ip, sizeof(*ip));
  printf("\n");
  printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
  printf(" %1x  %1x  %02x", ip->ip_v, ip->ip_hl, ip->ip_tos);
  printf (" %04x %04x",
    (ip->ip_len > 0x2000) ? ntohs (ip->ip_len) : ip->ip_len,
    ntohs (ip->ip_id));
  printf ("   %1x %04x", (ntohs (ip->ip_off) & 0xe000) >> 13,
    ntohs (ip->ip_off) & 0x1fff);
  printf ("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ntohs (ip->ip_sum));
  printf (" %s ", inet_ntoa(ip->ip_src));
  printf (" %s ", inet_ntoa (ip->ip_dst));
  while (hlen-- > sizeof (*ip))
    printf ("%02x", *cp++);
  printf ("\n");
  printf ("ICMP: type %u, code %u, size %lu", header->icmp_type, header->icmp_code, ntohs (ip->ip_len) - hlen);
  printf (", id 0x%04x, seq 0x%04x", header->icmp_id, header->icmp_seq);
  printf ("\n");

}