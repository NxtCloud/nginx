/*
	Copyright (C) Yang Zhang
*/
#ifndef _FDS_CORE_H_
#define _FDS_CORE_H_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>

#define FDS_MODULE		0x00555244
#define FDS_MAIN_CONF		0x02000000
typedef struct {
	void			**main_conf;
	void			**srv_conf;
} fds_conf_ctx_t;
typedef struct {
	u_char			sockaddr[NGX_SOCKADDRLEN];
	socklen_t		socklen;

	/* server ctx */
	fds_conf_ctx_t		*ctx;

	unsigned		bind:1;
	unsigned		wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
	unsigned		ipv6only:1;
#endif
	unsigned		so_keepalive:2;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
	int			tcp_keepidle;
	int	 		tcp_keepintvl;
	int	 		tcp_keepcnt;
#endif
} fds_listen_t;

typedef struct {
	fds_conf_ctx_t		*ctx;
	ngx_str_t		addr_text;
} fds_addr_conf_t;

typedef struct {
	in_addr_t		addr;
	fds_addr_conf_t 	conf;
} fds_in_addr_t;


#if (NGX_HAVE_INET6)

typedef struct {
	struct in6_addr 	addr6;
	fds_addr_conf_t 	conf;
} fds_in6_addr_t;

#endif

typedef struct {
	void 			*addrs;
	ngx_uint_t		naddrs;
} fds_port_t;


typedef struct {
	int	 		family;
	in_port_t		port;
	ngx_array_t		addrs;
} fds_conf_port_t;


typedef struct {
	struct sockaddr		*sockaddr;
	socklen_t		socklen;

	fds_conf_ctx_t		*ctx;

	unsigned		bind:1;
	unsigned		wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
	unsigned		ipv6only:1;
#endif
	unsigned		so_keepalive:2;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
	int			tcp_keepidle;
	int			tcp_keepintvl;
	int			tcp_keepcnt;
#endif
} fds_conf_addr_t;


typedef struct {
	ngx_array_t		servers;	 
	ngx_array_t		listen;	
} fds_core_main_conf_t;

typedef struct {
    ngx_msec_t              timeout;
    ngx_msec_t              resolver_timeout;

    ngx_flag_t              so_keepalive;

    ngx_str_t               server_name;

    u_char                 *file_name;
    ngx_int_t               line;

    ngx_resolver_t         *resolver;

    /* server ctx */
    fds_conf_ctx_t    *ctx;
} fds_core_srv_conf_t;

typedef struct {

    void                       *(*create_main_conf)(ngx_conf_t *cf);
    char                       *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void                       *(*create_srv_conf)(ngx_conf_t *cf);
    char                       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev,
                                      void *conf);
} fds_module_t;
#endif
