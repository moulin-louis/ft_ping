//
// Created by loumouli on 12/16/23.
//

#include "ft_ping.h"

double	get_diff_time(const timeval* s1, const timeval* s2) {
  return (double)(s2->tv_sec - s1->tv_sec) * 1000.0 + (double)(s2->tv_usec - s1->tv_usec) / 1000.0;
}