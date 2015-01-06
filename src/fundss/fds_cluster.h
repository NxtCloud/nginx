#ifndef _FDS_CLUSTER_H_
#define _FDS_CLUSTER_H_

#include<stdio.h>
#include<ngx_config.h>
#include<ngx_core.h>

#define TIME_OUT  1000
#define BASE_ZNODE "/fundss"

typedef struct fds_node_t fds_node_t;
typedef struct fds_cluster_t fds_cluster_t;

struct fds_node_t {
	ngx_str_t address;
};


struct fds_cluster_t {
	ngx_str_t cluster_name;
	ngx_list_t *nodes;
};

fds_cluster_t fds_cluster;
fds_node_t *myself;

void fds_cluster_join(fds_node_t *node);
void fds_cluster_leave();
void fds_cluster_notify();

void fds_cluster_get_nodes_info(ngx_list_t *nodes);
	
#endif
