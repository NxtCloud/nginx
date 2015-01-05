/*
	Copyright (C) Yang Zhang
*/
#include <ngx_config.h>
#include <ngx_core.h>
#include <fds_core.h>

static char *fds_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *fds_core_create_conf(ngx_cycle_t *cycle);
static char *fds_core_init_conf(ngx_cycle_t *cycle, void *conf);
static ngx_int_t fds_core_module_init(ngx_cycle_t *cycle);

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
    fds_core_init_conf
};


ngx_module_t  fds_module = {
    NGX_MODULE_V1,
    &fds_module_ctx,                /* module context */
    fds_commands,                   /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_str_t  fds_core_name = ngx_string("fds_core");

static ngx_command_t  fds_core_commands[] = {};

fds_module_t  fds_core_module_ctx = {
    &fds_core_name,
    fds_core_create_conf,            /* create configuration */
    fds_core_init_conf,              /* init configuration */

    {}
};


ngx_module_t  fds_core_module = {
    NGX_MODULE_V1,
    &fds_core_module_ctx,            /* module context */
    fds_core_commands,               /* module directives */
    FDS_MODULE,                      /* module type */
    NULL,                                  /* init master */
    fds_core_module_init,                 /* init module */
    NULL,
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static char *
fds_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	return NGX_CONF_OK;
}
static void *
fds_core_create_conf(ngx_cycle_t *cycle)
{
	/*TODO default conf*/
	return NULL;
}

static char *
fds_core_init_conf(ngx_cycle_t *cycle, void *conf)
{
	/*TODO init conf*/
	return NULL;
}

static ngx_int_t
fds_core_module_init(ngx_cycle_t *cycle)
{
	env = ngx_palloc(cycle->pool, sizeof(fds_env_t));
	env->pool = ngx_create_pool(FDS_POOL_DEFAULT_SIZE, cycle->log);	

	return 0;
}
