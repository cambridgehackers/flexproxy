/* $Id: config.h,v 1.8 2004/06/21 12:17:00 cvs Exp $ */

#define FLEXPORT   1965
#define MLABPORT  34440

#define MAXBUF 32768
#define FLEXHOST "129.132.1.3" /* hinz.ethz.ch */

#define CONFDIR "/usr/local/etc"
#define CONFFILE "flexproxy.conf"

#define HOSTFILE "/usr/local/etc/hosts"

/*
 * Configuration file parameters
 *
 */
#define FLEX_HOST "remote_flex_host"
#define REMOTE_FLEX_PORT "remote_flex_port"
#define PROXY_PORT "local_proxy_port"
#define PROXY_HOSTS "proxy_hosts_file"

#ifdef DEBUG
#define logmessage(a, b) fprintf(stderr, b)
#else
#define logmessage(a, b)
#endif

/* define to enable ipv6 */
#undef INET6
