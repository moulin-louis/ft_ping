//
// Created by loumouli on 12/15/23.
//

#include "ft_ping.h"

uint32_t option;
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
  const double stddev = ping.sumq / (ping.num_recv + ping.num_rept) - ping.avg * ping.avg;

  printf("--- %s ping statistics ---\n", ping.hostname);
  printf("%zu packets transmitted, %zu packets received, %zu%% packet loss\n",
         ping.num_emit, ping.num_recv, (ping.num_emit - ping.num_recv) * 100 / ping.num_emit);
  if (ping.num_recv)
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
           ping.min, ping.avg, ping.max, nsqrt(stddev, 0.0005));
}

static void ping_init(void) {
  const int ph = 1;
  ping.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (ping.fd < 0)
    exit_error(NULL);
  if (setsockopt(ping.fd, SOL_SOCKET,SO_BROADCAST, &ph, sizeof(ph))) //enable broadcast
    exit_error(NULL);
  //enable ip header
  if (setsockopt(ping.fd, IPPROTO_IP, IP_HDRINCL, &ph, sizeof(ph))) //enable ip header
    exit_error(NULL);
  const int32_t fd = open("/proc/sys/net/ipv4/ip_default_ttl", O_RDONLY); //get ttl
  if (fd < 0)
    exit_error(NULL);
  char buf[1024];
  const ssize_t len = read(fd, buf, sizeof(buf));
  close(fd);
  if (len < 0)
    exit_error(NULL);
  buf[len] = 0;
  ping.sys_ttl = ft_atoi(buf); //convert ttl to int
  if (ping.sys_ttl == -1)
    exit_error("Atoi error");
}

int main(const int ac, char** av) {
  if (ac == 1) {
    fprintf(stderr, "ft_ping: missing host operand\n");
    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
    return 1;
  }
  ft_memset(&ping, 0, sizeof(ping));
  ping.min = MAXWAIT + 10;
  const int retval = parse_arg(av);
  if (retval == 1)
    return 0;
  signal(SIGINT, sig_handler);
  ping.ident = getpid();
  bool performed = false;
  for (int32_t idx = 1; idx < ac; idx++) {
    if (av[idx]) {
      ping_init();
      ping_echo(av[idx]);
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
