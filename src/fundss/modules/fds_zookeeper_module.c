#include<zookeeper/zookeeper.h>
#include"fds_cluster.h"


#define MAX_PATH_LEN 100
typedef struct String_vector vector_t;

static inline ZOOAPI int zookeeper_create_node(const char *path, int flag);
static inline ZOOAPI int zookeeper_delete_node(const char *path);
static inline ZOOAPI int zookeeper_exists_node(const char *path);
static inline ZOOAPI int zookeeper_get_childrens(const char *path, vector_t *strings);


static zhandle_t *zhandle;

static inline void fds_cluster_init(const char *zk_host){
	int rc;

	zhandle = zookeeper_init(zk_host, NULL, 10000, NULL, "fundss zookeeper", 0);
	if (!zhandle){
		printf("error\n");
		goto out;
		
	}
	
	int interval = 100, retry = 0;
	
        int max_retry = 1000 / interval;
        while (zoo_state(zhandle) != ZOO_CONNECTED_STATE) {
		printf("retry ...");		
                usleep(interval * 1000);
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
	
	char path[MAX_PATH_LEN];
	
	snprintf(path, MAX_PATH_LEN, BASE_ZNODE"/%s",node->address);
	printf("%s join...\n", node->address);
	printf("%s\n",path);
	int rc =  zookeeper_create_node(path, ZOO_EPHEMERAL);
	printf("rc = %d, a = %d\n",rc, ZNOCHILDRENFOREPHEMERALS);

}

void fds_cluster_leave(fds_node_t *node){

	char path[MAX_PATH_LEN];
	snprintf(path, MAT_PATH_LEN,BASE_ZNODE"/%s",node->address);
	int rc = zookeeper_delete_node(path);

}


void fds_cluster_get_nodes_info(){
	struct vector_t strings;
	int rc;	
	rc = zookeeper_get_childrens(BASE_ZNODE, &strings);	
	
	printf("children count %d\n",strings.count); 

}


static inline ZOOAPI int zookeeper_create_node(const char *path, int flag)
{
        int rc;
	struct ACL_vector *ACL= &ZOO_OPEN_ACL_UNSAFE;
        do{
                rc = zoo_create(zhandle, path, "", 0, ACL, flag, NULL, 0);
        } while (rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS);
        return rc; 
	printf("%d\n",rc);
}

static inline ZOOAPI int zookeeper_delete_node(const char *path){
	int rc;
	do{
		rc = zoo_delete(zhandle, path, 0);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );

	return rc;
}


static inline ZOOAPI int zookeeper_exists_node(const char *path){
	int rc;
	struct Stat stat;
	do{
		rc = zoo_exists(zhandle, path, 0, &stat);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;

}

static inline ZOOAPI int zookeeper_get_childrens(const char *path, struct vector_t *strings){
	int rc;
	do{
		rc = zoo_get_children(zhandle, path, 0, strings);
	}while(rc == ZOPERATIONTIMEOUT || rc == ZCONNECTIONLOSS );
	
	return rc;
}

