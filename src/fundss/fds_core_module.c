/*
    Copyright (C) Yang Zhang
*/
#include <ngx_config.h>
#include <ngx_core.h>
#include <fds.h>

static char *fds_core_server(ngx_conf_t *cf, ngx_command_t *cmd, void *dummy);
static char *fds_core_listen(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t fds_core_commands[]={
    { ngx_string("server"),
      FDS_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      fds_core_server,
      0,
      0,
      NULL},
    { ngx_string("listen"),
      FDS_SRV_CONF|NGX_CONF_1MORE,
      fds_core_listen,
      FDS_SRV_CONF_OFFSET,
      0,
      NULL },
};
static fds_module_t  fds_core_module_ctx = {

    NULL,        /* create main configuration */
    NULL,          /* init main configuration */


    NULL,         /* create location configuration */
    NULL            /* merge location configuration */
};
ngx_module_t fds_core_module = {
    NGX_MODULE_V1,
    &fds_core_module_ctx,             /* module context */
    fds_core_commands,                /* module directives */
    FDS_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
fds_core_server(ngx_conf_t *cf, ngx_command_t *cmd, void *dummy)
{
    char                        *rv;
    void                        *mconf;
    ngx_uint_t                  i;
    ngx_conf_t                  pcf;
    fds_module_t                *module;
    fds_conf_ctx_t              *ctx, *fds_ctx;
    fds_core_srv_conf_t         *cscf, **cscfp;
    fds_core_main_conf_t        *cmcf;

    ctx = ngx_pcalloc(cf->pool, sizeof(fds_conf_ctx_t));
    if(ctx == NULL){
        return NGX_CONF_ERROR;
    }
    fds_ctx = cf->ctx;
    ctx->main_conf = fds_ctx->main_conf;

    ctx->srv_conf = ngx_pcalloc(cf->pool, sizeof(void *) *fds_max_module);
    if(ctx->srv_conf == NULL){
        return NGX_CONF_ERROR;
    }

    for (i = 0; ngx_modules[i]; i++) {
        if(ngx_modules[i]->type != FDS_MODULE) {
            continue;
        }

        module = ngx_modules[i]->ctx;
        if (module->create_srv_conf) {
            mconf = module->create_srv_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            ctx->srv_conf[ngx_modules[i]->ctx_index] = mconf;
        }
    }
    

    cscf = ctx->srv_conf[fds_core_module.ctx_index];
    cscf->ctx = ctx;

    cmcf = ctx->main_conf[fds_core_module.ctx_index];
    
    cscfp = ngx_array_push(&cmcf->servers);
    if(cscfp == NULL){
        return NGX_CONF_ERROR;
    }

    *cscfp = cscf;

    pcf = *cf;
    cf->ctx = ctx;
    cf->cmd_type = FDS_SRV_CONF;
    
    rv = ngx_conf_parse(cf, NULL);

    *cf = pcf;
    return rv;
}
static char *
fds_core_listen(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    size_t                      len, off;
    in_port_t                   port;
    ngx_str_t                  *value;
    ngx_url_t                   u;
    ngx_uint_t                  i;
    struct sockaddr            *sa;
    fds_listen_t               *ls;
    struct sockaddr_in         *sin;
    fds_core_main_conf_t       *cmcf;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6        *sin6;
#endif

    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = value[1];
    u.listen = 1;

    if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "%s in \"%V\" of the \"listen\" directive",
                               u.err, &u.url);
        }

        return NGX_CONF_ERROR;
    }

    cmcf = fds_conf_get_module_main_conf(cf, fds_core_module);

    ls = cmcf->listen.elts;

    for (i = 0; i < cmcf->listen.nelts; i++) {

        sa = (struct sockaddr *) ls[i].sockaddr;

        if (sa->sa_family != u.family) {
            continue;
        }

        switch (sa->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            off = offsetof(struct sockaddr_in6, sin6_addr);
            len = 16;
            sin6 = (struct sockaddr_in6 *) sa;
            port = sin6->sin6_port;
            break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
        case AF_UNIX:
            off = offsetof(struct sockaddr_un, sun_path);
            len = sizeof(((struct sockaddr_un *) sa)->sun_path);
            port = 0;
            break;
#endif

        default: /* AF_INET */
            off = offsetof(struct sockaddr_in, sin_addr);
            len = 4;
            sin = (struct sockaddr_in *) sa;
            port = sin->sin_port;
            break;
        }

        if (ngx_memcmp(ls[i].sockaddr + off, u.sockaddr + off, len) != 0) {
            continue;
        }

        if (port != u.port) {
            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "duplicate \"%V\" address and port pair", &u.url);
        return NGX_CONF_ERROR;
    }

    ls = ngx_array_push(&cmcf->listen);
    if (ls == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(ls, sizeof(fds_listen_t));

    ngx_memcpy(ls->sockaddr, u.sockaddr, u.socklen);

    ls->socklen = u.socklen;
    ls->wildcard = u.wildcard;
    ls->ctx = cf->ctx;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    ls->ipv6only = 1;
#endif

    for (i = 2; i < cf->args->nelts; i++) {

        if (ngx_strcmp(value[i].data, "bind") == 0) {
            ls->bind = 1;
            continue;
        }

        if (ngx_strncmp(value[i].data, "ipv6only=o", 10) == 0) {
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
            struct sockaddr  *sa;
            u_char            buf[NGX_SOCKADDR_STRLEN];

            sa = (struct sockaddr *) ls->sockaddr;

            if (sa->sa_family == AF_INET6) {

                if (ngx_strcmp(&value[i].data[10], "n") == 0) {
                    ls->ipv6only = 1;

                } else if (ngx_strcmp(&value[i].data[10], "ff") == 0) {
                    ls->ipv6only = 0;
                } else {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "invalid ipv6only flags \"%s\"",
                                       &value[i].data[9]);
                    return NGX_CONF_ERROR;
                }

                ls->bind = 1;

            } else {
                len = ngx_sock_ntop(sa, ls->socklen, buf,
                                    NGX_SOCKADDR_STRLEN, 1);

                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "ipv6only is not supported "
                                   "on addr \"%*s\", ignored", len, buf);
            }

            continue;
#else
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "bind ipv6only is not supported "
                               "on this platform");
            return NGX_CONF_ERROR;
#endif
        }

        if (ngx_strncmp(value[i].data, "so_keepalive=", 13) == 0) {

            if (ngx_strcmp(&value[i].data[13], "on") == 0) {
                ls->so_keepalive = 1;

            } else if (ngx_strcmp(&value[i].data[13], "off") == 0) {
                ls->so_keepalive = 2;

            } else {

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
                u_char     *p, *end;
                ngx_str_t   s;

                end = value[i].data + value[i].len;
                s.data = value[i].data + 13;

                p = ngx_strlchr(s.data, end, ':');
                if (p == NULL) {
                    p = end;
                }

                if (p > s.data) {
                    s.len = p - s.data;

                    ls->tcp_keepidle = ngx_parse_time(&s, 1);
                    if (ls->tcp_keepidle == (time_t) NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                s.data = (p < end) ? (p + 1) : end;

                p = ngx_strlchr(s.data, end, ':');
                if (p == NULL) {
                    p = end;
                }

                if (p > s.data) {
                    s.len = p - s.data;

                    ls->tcp_keepintvl = ngx_parse_time(&s, 1);
                    if (ls->tcp_keepintvl == (time_t) NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                s.data = (p < end) ? (p + 1) : end;

                if (s.data < end) {
                    s.len = end - s.data;

                    ls->tcp_keepcnt = ngx_atoi(s.data, s.len);
                    if (ls->tcp_keepcnt == NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                if (ls->tcp_keepidle == 0 && ls->tcp_keepintvl == 0
                    && ls->tcp_keepcnt == 0)
                {
                    goto invalid_so_keepalive;
                }

                ls->so_keepalive = 1;

#else

                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "the \"so_keepalive\" parameter accepts "
                                   "only \"on\" or \"off\" on this platform");
                return NGX_CONF_ERROR;

#endif
            }

            ls->bind = 1;

            continue;

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
        invalid_so_keepalive:
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid so_keepalive value: \"%s\"",
                               &value[i].data[13]);
            return NGX_CONF_ERROR;
#endif
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "the invalid \"%V\" parameter", &value[i]);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}
