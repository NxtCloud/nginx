/*
	Copyright (C) Yang Zhang
*/
#ifndef _FDS_CORE_H_
#define _FDS_CORE_H_

#define FDS_POOL_DEFAULT_SIZE 2048

typedef struct {
        ngx_pool_t *pool;
}fds_env_t;

typedef struct {
}fds_conf_t;

typedef struct {
} fds_actions_t;

typedef struct {
    ngx_str_t              *name;

    void                 *(*create_conf)(ngx_cycle_t *cycle);
    char                 *(*init_conf)(ngx_cycle_t *cycle, void *conf);

    fds_actions_t     actions;
} fds_module_t;

static fds_env_t *env;
#endif
