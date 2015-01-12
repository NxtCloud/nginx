/*
	Copyright (C) Yang Zhang
*/
#include <ngx_config.h>
#include <ngx_core.h>
#include <fds.h>

static char *fds_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t fds_add_ports(ngx_conf_t *cf, ngx_array_t *ports, fds_listen_t *listen);
static char *fds_optimize_servers(ngx_conf_t *cf, ngx_array_t *ports);

static ngx_command_t  fds_commands[] = {

	{ ngx_string("fds"),
	NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
	fds_block,
	0,
	0,
	NULL },

	ngx_null_command
};

static ngx_core_module_t  fds_module_ctx = {
	ngx_string("fds"),
	NULL,
	NULL
};


ngx_module_t  fds_module = {
	NGX_MODULE_V1,
	&fds_module_ctx,				/* module context */
	fds_commands,				   /* module directives */
	NGX_CORE_MODULE,					   /* module type */
	NULL,								  /* init master */
	NULL,								  /* init module */
	NULL,								  /* init process */
	NULL,								  /* init thread */
	NULL,								  /* exit thread */
	NULL,								  /* exit process */
	NULL,								  /* exit master */
	NGX_MODULE_V1_PADDING
};

ngx_uint_t fds_max_module;
static char *
fds_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	char *			rv;
	ngx_uint_t		mi, m, s, i;
	ngx_array_t		ports;
	ngx_conf_t		pcf;
	fds_listen_t		*listen;
	fds_module_t	*module;
	fds_conf_ctx_t		*ctx;
	fds_core_srv_conf_t	**cscfp;	
	fds_core_main_conf_t	*cmcf;

	ctx = ngx_pcalloc(cf->pool, sizeof(fds_conf_ctx_t));
	if (ctx == NULL) {
		return NGX_CONF_ERROR;
	}

	*(fds_conf_ctx_t **) conf = ctx;

	fds_max_module = 0;
	for (m = 0; ngx_modules[m]; m++) {
		if (ngx_modules[m]->type != FDS_MODULE) {
			continue;
		}

		ngx_modules[m]->ctx_index = fds_max_module++;
	}

	ctx->main_conf = ngx_pcalloc(cf->pool,
				sizeof(void *) * fds_max_module);
	if (ctx->main_conf == NULL) {
		return NGX_CONF_ERROR;
	}
	ctx->srv_conf = ngx_pcalloc(cf->pool,
				sizeof(void *) * fds_max_module);
	if (ctx->srv_conf == NULL) {
		return NGX_CONF_ERROR;
	}

	for (m = 0; ngx_modules[m]; m++) {
		if (ngx_modules[m]->type != FDS_MODULE) {
			continue;
		}

		module = ngx_modules[m]->ctx;
		mi = ngx_modules[m]->ctx_index;

		if (module->create_main_conf) {
			ctx->main_conf[mi] = module->create_main_conf(cf);
			if (ctx->main_conf[mi] == NULL) {
				return NGX_CONF_ERROR;
			}
		}

		if (module->create_srv_conf) {
			ctx->srv_conf[mi] = module->create_srv_conf(cf);
			if (ctx->srv_conf[mi] == NULL) {
				return NGX_CONF_ERROR;
			}
		}
	}

	pcf = *cf;
	cf->ctx = ctx;
	cf->module_type = FDS_MODULE;
	cf->cmd_type = FDS_MAIN_CONF;

	rv = ngx_conf_parse(cf, NULL);
	if(rv != NGX_CONF_OK){
		*cf = pcf;
		return rv;
	}

	
	cmcf = ctx->main_conf[fds_core_module.ctx_index];
	cscfp = cmcf->servers.elts;
	
	
	for (m = 0; ngx_modules[m]; m++) {
		if (ngx_modules[m]->type != FDS_MODULE) {
			continue;
		}

		module = ngx_modules[m]->ctx;
		mi = ngx_modules[m]->ctx_index;

		cf->ctx = ctx;

		if (module->init_main_conf) {
			rv = module->init_main_conf(cf, ctx->main_conf[mi]);
			if (rv != NGX_CONF_OK) {
				*cf = pcf;
				return rv;
			}
		}

		for (s = 0; s < cmcf->servers.nelts; s++) {

		/* merge the server{}s' srv_conf's */

			cf->ctx = cscfp[s]->ctx;

			if (module->merge_srv_conf) {
				rv = module->merge_srv_conf(cf,
					ctx->srv_conf[mi],
					cscfp[s]->ctx->srv_conf[mi]);
				if (rv != NGX_CONF_OK) {
					*cf = pcf;
					return rv;
				}
			}
		}
	}

	*cf = pcf;

	if (ngx_array_init(&ports, cf->temp_pool, 4, sizeof(fds_conf_port_t))
			!= NGX_OK)
	{
		return NGX_CONF_ERROR;
	}

	listen = cmcf->listen.elts;

	for (i = 0; i < cmcf->listen.nelts; i++) {
		if (fds_add_ports(cf, &ports, &listen[i]) != NGX_OK) {
			return NGX_CONF_ERROR;
		}
	}

	return fds_optimize_servers(cf, &ports);
}

static ngx_int_t
fds_add_ports(ngx_conf_t *cf, ngx_array_t *ports, 
	fds_listen_t *listen)
{
	in_port_t		p;
	ngx_uint_t		i;
	struct sockaddr	   	*sa;
	struct sockaddr_in	*sin;
	fds_conf_port_t  	*port;
	fds_conf_addr_t  	*addr;
#if (NGX_HAVE_INET6)
	struct sockaddr_in6   	*sin6;
#endif

	sa = (struct sockaddr *) &listen->sockaddr;

	switch (sa->sa_family) {

#if (NGX_HAVE_INET6)
	case AF_INET6:
		sin6 = (struct sockaddr_in6 *) sa;
		p = sin6->sin6_port;
		break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
	case AF_UNIX:
		p = 0;
		break;
#endif

	default: /* AF_INET */
		sin = (struct sockaddr_in *) sa;
		p = sin->sin_port;
			break;
		}

	port = ports->elts;
	for (i = 0; i < ports->nelts; i++) {
		if (p == port[i].port && sa->sa_family == port[i].family) {
			port = &port[i];
			goto found;
		}
	}

	port = ngx_array_push(ports);
	if (port == NULL) {
		return NGX_ERROR;
	}

	port->family = sa->sa_family;
	port->port = p;

   	if (ngx_array_init(&port->addrs, cf->temp_pool, 2,
			sizeof(fds_conf_addr_t))
		!= NGX_OK)
	{
		return NGX_ERROR;
	}

found:

	addr = ngx_array_push(&port->addrs);
	if (addr == NULL) {
		return NGX_ERROR;
	}


	addr->sockaddr = (struct sockaddr *) &listen->sockaddr;
	addr->socklen = listen->socklen;
	addr->ctx = listen->ctx;
	addr->bind = listen->bind;
	addr->wildcard = listen->wildcard;
	addr->so_keepalive = listen->so_keepalive;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
	addr->tcp_keepidle = listen->tcp_keepidle;
	addr->tcp_keepintvl = listen->tcp_keepintvl;
	addr->tcp_keepcnt = listen->tcp_keepcnt;
#endif
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
	addr->ipv6only = listen->ipv6only;
#endif

	return NGX_OK;

}
static char *
fds_optimize_servers(ngx_conf_t *cf, ngx_array_t *ports)
{
	
	return NULL;
}
