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
  const uint16_t chksum = (*header)->checksum;
  (*header)->checksum = 0;
  (*header)->checksum = checksum((uint16_t *)*header);
  if (chksum != (*header)->checksum)
    return 1;
  return 0;
}
