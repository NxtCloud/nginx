#include<zookeeper/zookeeper.h>
#include"fds_cluster.h"

typedef struct String_vector vector_t;

static inline ZOOAPI ngx_int_t zookeeper_create_node(ngx_str_t *path, ngx_int_t flag);
static inline ZOOAPI ngx_int_t zookeeper_delete_node(ngx_str_t *path);
static inline ZOOAPI ngx_int_t zookeeper_exists_node(ngx_str_t *path);
static inline ZOOAPI ngx_int_t zookeeper_get_childrens(ngx_str_t *path, vector_t *strings);


static zhandle_t *zhandle;

static inline void fds_cluster_init(ngx_str_t *zk_host){
	ngx_int_t rc;

	zhandle = zookeeper_init(zk_host, NULL, 10000, NULL, "fundss zookeeper", 0);
	if (!zhandle){
		prngx_int_tf("error\n");
		goto out;
		
	}
	
	ngx_int_t ngx_int_terval = 100, retry = 0;
	
        ngx_int_t max_retry = 1000 / ngx_int_terval;
        while (zoo_state(zhandle) != ZOO_CONNECTED_STATE) {
		prngx_int_tf("retry ...");		
                usleep(ngx_int_terval * 1000);
                if (++retry >= max_retry) {
			goto out;
                }
        }
		
		
	rc = zookeeper_create_node(BASE_ZNODE, 0);
	return;
out:
	exit(1);
}

void fds_cluster_join(fds_node_t *node){	
	
	ngx_str_t path;
	
	snprngx_int_tf(path, 100, BASE_ZNODE"/%s",node->address);
	prngx_int_tf("%s join...\n", node->address);
	prngx_int_tf("%s\n",path);
	ngx_int_t rc =  zookeeper_create_node(path, ZOO_EPHEMERAL);
	prngx_int_tf("rc = %d, a = %d\n",rc, ZNOCHILDRENFOREPHEMERALS);

}

void fds_cluster_leave(fds_node_t *node){

	ngx_str_t path;
	snprngx_int_tf(path,100,BASE_ZNODE"/%s",node->address);
	ngx_int_t rc = zookeeper_delete_node(path);

}


void fds_cluster_get_nodes_info(){
	struct vector_t strings;
	ngx_int_t rc;	
	rc = zookeeper_get_childrens(BASE_ZNODE, &strings);	
	
	prngx_int_tf("children count %d\n",strings.count); 

}


static inline ZOOAPI ngx_int_t zookeeper_create_node(ngx_str_t *path, ngx_int_t flag)
{
        ngx_int_t rc;
	struct ACL_vector *ACL= &ZOO_OPEN_ACL_UNSAFE;
        do{
                rc = zoo_create(zhandle, path, "", 0, ACL, flag, NULL, 0);
        } while (rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS);
        return rc; 
	prngx_int_tf("%d\n",rc);
}

static inline ZOOAPI ngx_int_t zookeeper_delete_node(ngx_str_t *path){
	ngx_int_t rc;
	do{
		rc = zoo_delete(zhandle, path, 0);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );

	return rc;
}


static inline ZOOAPI ngx_int_t zookeeper_exists_node(ngx_str_t *path){
	ngx_int_t rc;
	struct Stat stat;
	do{
		rc = zoo_exists(zhandle, path, 0, &stat);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;

}

static inline ZOOAPI ngx_int_t zookeeper_get_childrens(ngx_str_t *path, struct vector_t *strings){
	ngx_int_t rc;
	do{
		rc = zoo_get_children(zhandle, path, 0, strings);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;
}

