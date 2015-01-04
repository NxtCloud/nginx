/*
	Copyright (C) Yang Zhang
*/
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

static ngx_command_t  ngx_fds_core_commands[] = {}

fds_module_t  ngx_fds_core_module_ctx = {
    &fds_core_name,
    fds_core_create_conf,            /* create configuration */
    fds_core_init_conf,              /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


ngx_module_t  ngx_fds_core_module = {
    NGX_MODULE_V1,
    &ngx_fds_core_module_ctx,            /* module context */
    ngx_fds_core_commands,               /* module directives */
    NGX_FDS_MODULE,                      /* module type */
    NULL,                                  /* init master */
    fds_core_module_init,                 /* init module */
    fds_core_process_init,                /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};



static void *
fds_core_create_conf(ngx_cycle_t *cycle)
{
	fds_conf_t *fcf;
	
	fcf = ngx_palloc(cycle->pool, sizeof(fds_conf_t));
	if(fcf == NULL){
		return NULL;
	}
	
	/*TODO default conf*/
	return NULL;
}

static char *
fds_core_init_conf(ngx_cycle_t *cycle, void *conf)
{
	fds_conf_t *fcf = conf;
	/*TODO init conf*/
	return NULL;
}

static ngx_int_t
fds_core_init_module(ngx_cycle_t *cycle)
{
	void ***cf;
	fds_conf_t *fcf;

	cf = ngx_get_conf(cycle->conf_ctx, ngx_fds_module);
	fcf = (*cf)[ngx_fds_core_module.ctx_index];

	env = ngx_palloc(cycle->pool, sizeof(fds_env_t));
	env->pool = ngx_create_pool(FDS_POOL_DEFAULT_SIZE, cycle->log);	

	return 0;
}
