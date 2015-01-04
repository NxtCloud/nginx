#ifndef _FDS_OBJECT_H_
#define _FDS_OBJECT_H_

#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
        fds_req_t               *req;
        ngx_peer_connection_t   peer;

        ngx_mosec_t             timeout;
}fds_object_connect_t;

typedef struct {
        ngx_uchar_t             *key;
        ngx_uchar_t             *hash_key;
        ngx_int_t               offset;
        size_t                  len;
        ngx_uchar_t             *buf;
}fds_object_t;
/*TODO should be in fds_request.h*/
#define FDS_READ_OBJ 0x1
#define FDS_CONN_TIME_OUT 10
typedef struct {
        ngx_int_t               op;
        ngx_uchar_t             *key;
        ngx_uchar_t             *data
}fds_req_t;


ngx_int_t
fds_object_read(fds_object_t *o);
ngx_int_t
fds_object_write(fds_object_t *o);
#endif
