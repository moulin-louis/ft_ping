//
// Created by loumouli on 12/15/23.
//

#ifndef FT_PING_H
#define FT_PING_H

#include <libft.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
# include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <fcntl.h>

#define ICMP_MINLEN 8
#define ICMP_ECHO 8
#define ICMP_TIMXCEED 11
#define ICMP_TIME_EXCEEDED 11
#define ICMP_ECHOREPLY 0
#define MAXWAIT 1 // max time to wait for a packet
#define OPT_VERBOSE 0x020 // verbose option

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;


typedef struct
{
  unsigned char icmp_type;		/* type of message, see below */
  unsigned char icmp_code;		/* type sub code */
  unsigned short icmp_cksum;		/* ones complement cksum of struct */
  union
  {
    unsigned char ih_pptr;		/* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr;	/* ICMP_REDIRECT */
    struct ih_idseq
    {
      unsigned short icd_id;
      unsigned short icd_seq;
    } ih_idseq;
    int ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU discovery as per rfc 1191 */
    struct ih_pmtu
    {
      unsigned short ipm_void;
      unsigned short ipm_nextmtu;
    } ih_pmtu;

    /* ICMP_ROUTERADV -- RFC 1256 */
    struct ih_rtradv
    {
      unsigned char irt_num_addrs;	/* Number of addresses following the msg */
      unsigned char irt_wpa;		/* Address Entry Size (32-bit words) */
      unsigned short irt_lifetime;	/* Lifetime */
    } ih_rtradv;

  } icmp_hun;
#define icmp_pptr	icmp_hun.ih_pptr
#define icmp_gwaddr	icmp_hun.ih_gwaddr
#define icmp_id		icmp_hun.ih_idseq.icd_id
#define icmp_seq	icmp_hun.ih_idseq.icd_seq
#define icmp_void	icmp_hun.ih_void
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_num_addrs  icmp_hun.ih_rtradv.irt_num_addrs
#define icmp_wpa        icmp_hun.ih_rtradv.irt_wpa
#define icmp_lifetime   icmp_hun.ih_rtradv.irt_lifetime

  union
  {
    struct id_ts		/* ICMP_TIMESTAMP, ICMP_TIMESTAMPREPLY */
    {
      n_time its_otime;		/* Originate timestamp */
      n_time its_rtime;		/* Recieve timestamp */
      n_time its_ttime;		/* Transmit timestamp */
    } id_ts;
    struct id_ip		/* Original IP header */
    {
      struct ip idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    unsigned long id_mask;		/* ICMP_ADDRESS, ICMP_ADDRESSREPLY */
    char id_data[1];
  } icmp_dun;
#define icmp_otime	icmp_dun.id_ts.its_otime
#define icmp_rtime	icmp_dun.id_ts.its_rtime
#define icmp_ttime	icmp_dun.id_ts.its_ttime
#define icmp_ip		icmp_dun.id_ip.idi_ip
#define icmp_mask	icmp_dun.id_mask
#define icmp_data	icmp_dun.id_data
} icmphdr;

typedef struct {
  int fd; // file descriptor of the socket
  uint16_t ident; // identifier for the icmp header
  sockaddr dest; // destination of the packet
  sockaddr srcl; // source of the packet
  char* hostname; // hostname of the destination in string format
  char ip[INET_ADDRSTRLEN]; // ip address of the destination in string format
  struct msghdr msg; // message received for rcvmsg
  size_t datalen; // size of the data received
  int32_t sys_ttl; // time to live of the system
  int32_t recv_ttl; // time to live in the packet header received
  timeval current_time; // current time
  timeval old_time; // time before sending the packet
  icmphdr icmp_header; // icmp header
  uint8_t packet[sizeof(icmphdr) + sizeof(struct ip) + 8];
  double min; // min of the diffMS
  double max; // max of the diffMS
  double avg; // average of the diffMS
  double sumq; // sum of the square of the diffMS for the standard deviation
  size_t num_emit; // number of packet emitted
  size_t num_recv; // number of packet received
  size_t num_rept; // number of packet repeated
} ping_t;

extern ping_t ping;
extern volatile char stop;
extern bool timeout;

int ping_echo(char* hostname, int option);

double get_diff_time(const timeval* s1, const timeval* s2);

void sig_handler(int signal);

uint16_t checksum(uint16_t* addr);

int32_t icmp_decode(const uint8_t* buf, size_t len, icmphdr** header, struct ip** ip);

int32_t hostname_to_sockaddr(const char* hostname, void* result_ptr);

void exit_error(const char* msg);

int32_t ip_to_hostname(const char* ip, char* result_str);

double nsqrt(double a, double prec);

void icmp_error_log();

void icmp_hexdump(void *data, const size_t len);

#endif //FT_PING_H
