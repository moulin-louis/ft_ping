//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

int option;
ping_t ping;
volatile char stop;
bool timeout;

void sig_handler(const int signal) {
  if (signal == SIGINT) {
    stop = true;
  }
  if (signal == SIGALRM) {
    timeout = true;
  }
}

static void display_help() {
  printf("Usage: ft_ping [OPTION...] HOST ...\n");
  printf("Send ICMP ECHO_REQUEST packets to network hosts\n");
  printf("\n");
  printf(" Options valid for all request types:\n");
  printf("\n");
  printf("  -v, --verbose\t\tverbose output\n\n");
  printf(" Options valid for --echo requets:\n");
  printf("\n");
  printf("  -?, --help\t\tgive this help list\n\n");
}

static int parse_arg(char** av) {
  for (uint32_t idx = 1; av[idx]; idx++) {
    if (ft_strcmp(av[idx], "-?") == 0 || ft_strcmp(av[idx], "--help") == 0) {
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
  printf("--- %s ping statistics ---\n", ping.hostname);
  printf("%zu packets transmitted, %zu packets received, %zu%% packet loss\n",
         ping.num_emit, ping.num_recv, (ping.num_emit - ping.num_recv) * 100 / ping.num_emit);
}

static void ping_init(void) {
  const int ph = 0;
  ping.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (ping.fd < 0) {
    printf("./ft_ping: %s\n", strerror(errno));
    exit(1);
  }
  if (setsockopt(ping.fd, SOL_SOCKET,SO_BROADCAST, &ph, sizeof(ph))) {
    printf("./ft_ping: %s\n", strerror(errno));
    close(ping.fd);
    exit(1);
  }
}

int main(const int ac, char** av) {
  if (ac == 1) {
    fprintf(stderr, "ft_ping: missing host operand\n");
    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
    return 1;
  }
  ft_memset(&ping, 0, sizeof(ping));
  const int retval = parse_arg(av);
  if (retval == 1)
    return 0;
  signal(SIGINT, sig_handler);
  ping.ident = getpid();
  bool performed = false;
  for (int32_t idx = 1; idx < ac; idx++) {
    if (av[idx]) {
      ping_init();
      ping_echo(av[idx], option);
      performed = true;
      break;
    }
  }
  if (performed == false) {
    fprintf(stderr, "ft_ping: missing host operand\n");
    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
    return 1;
  }
  ping_finish();
  close(ping.fd);
  return 0;
}
