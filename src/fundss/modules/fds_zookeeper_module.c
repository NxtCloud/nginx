#include<zookeeper/zookeeper.h>
#include"fds_cluster.h"


#define MAX_PATH_LEN 100
typedef struct String_vector vector_t;

static inline ZOOAPI ngx_int_t zookeeper_create_node(const char *path, ngx_int_t flag);
static inline ZOOAPI ngx_int_t zookeeper_delete_node(const char *path);
static inline ZOOAPI ngx_int_t zookeeper_exists_node(const char *path);
static inline ZOOAPI ngx_int_t zookeeper_get_childrens(const char *path, vector_t *strings);


static zhandle_t *zhandle;

static inline void fds_cluster_init(const char *zk_host){
	ngx_int_t rc, flag;
	char path[MAX_PATH_LEN];

	zhandle = zookeeper_init(zk_host, NULL, 10000, NULL, "fundss zookeeper", 0);
	if (!zhandle){
		printf("error\n");
		goto out;
		
	}
	
	ngx_int_t ngx_int_terval = 100, retry = 0;
	
        ngx_int_t max_retry = 1000 / ngx_int_terval;
        while (zoo_state(zhandle) != ZOO_CONNECTED_STATE) {
		printf("retry ...");		
                usleep(ngx_int_terval * 1000);
                if (++retry >= max_retry) {
			goto out;
                }
        }
		
	flag = 0;
	rc = zookeeper_create_node(path, flag);
	printf("%d\n", (int)rc);	
out:
	exit(1);
}

void fds_cluster_join(fds_node_t *node){	
	
	char path[MAX_PATH_LEN];
	
	snprintf(path, MAX_PATH_LEN, BASE_ZNODE"/%s",node->address);
	ngx_int_t rc =  zookeeper_create_node(path, ZOO_EPHEMERAL);
	printf("rc = %d, a = %d\n",(int)rc, ZNOCHILDRENFOREPHEMERALS);

}

void fds_cluster_leave(fds_node_t *node){

	char path[MAX_PATH_LEN];
	

	snprintf(path, MAX_PATH_LEN, BASE_ZNODE"/%s",node->address);
	ngx_int_t rc = zookeeper_delete_node(path);
	printf("%d\n", (int)rc);	

}


void fds_cluster_get_nodes_info(ngx_list_t *nodes){
	vector_t strings;
	ngx_int_t rc;
	char path[MAX_PATH_LEN];

	rc = zookeeper_get_childrens(path, &strings);	

	printf("%d\n",(int)rc);			
	printf("children count %d\n",strings.count); 

}


static inline ZOOAPI ngx_int_t zookeeper_create_node(const char *path, ngx_int_t flag)
{
        ngx_int_t rc;
	struct ACL_vector *ACL= &ZOO_OPEN_ACL_UNSAFE;
        do{
                rc = zoo_create(zhandle, path, "", 0, ACL, flag, NULL, 0);
        } while (rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS);

        return rc; 
}

static inline ZOOAPI ngx_int_t zookeeper_delete_node(const char *path){
	ngx_int_t rc;
	do{
		rc = zoo_delete(zhandle, path, 0);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );

	return rc;
}


static inline ZOOAPI ngx_int_t zookeeper_exists_node(const char *path){
	ngx_int_t rc;
	struct Stat stat;
	do{
		rc = zoo_exists(zhandle, path, 0, &stat);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;

}

static inline ZOOAPI ngx_int_t zookeeper_get_childrens(const char *path, vector_t *strings){
	ngx_int_t rc;
	do{
		rc = zoo_get_children(zhandle, path, 0, strings);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;
}

