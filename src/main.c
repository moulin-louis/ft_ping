//
// Created by loumouli on 12/15/23.
//


#include "ft_ping.h"

int option;
ping_t ping;

static void display_help() {
  printf("Usage: ft_ping [OPTION...] HOST ...\n");
  printf("Send ICMP ECHO_REQUEST packets to network hosts\n");
  printf("\n");
  printf("Options valid for all request types:\n");
  printf("\n");
  printf("  -v, --verbose\t\tverbose output\n");
  printf(" Options valid for --echo requets:\n");
  printf("\n");
  printf("  -?, --help\t\tgive this help list\n");
}

static int parse_arg(char **av) {
  for (uint32_t idx = 1; av[idx]; idx++) {
    if (ft_strcmp(av[idx], "-?") == 0) {
      display_help();
      return 1;
    }
    if (ft_strcmp(av[idx], "-v") == 0 || ft_strcmp(av[idx], "--verbose") == 0) {
      av[idx] = NULL;
      option |= OPT_VERBOSE;
    }
  }
  return 0;
}

static void ping_finish(void) {

}

static void ping_init(void) {
  ping.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (ping.fd < 0) {
    printf("Error: socket() failed: %s\n", strerror(errno));
    exit(1);
  }
}

int main(const int ac, char **av) {
  if (ac == 1) {
    printf("Wrong format\n");
    return 1;
  }
  const int retval = parse_arg(av);
  if (retval == 1)
    return 0;
  for (int32_t idx = 1; idx < ac; idx++) {
    if (av[idx]) {
      ping_init();
      ping_echo(av[idx], option);
      break;
    }
  }
  ping_finish();
  return 0;
}