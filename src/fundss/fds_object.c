/*
	Copyright (C) Yang Zhang
*/

static void
fds_object_read_handler();

ngx_int_t
fds_object_read(fds_object_t *o)
{
	fds_object_connect_t *c;
	ngx_int_t rc;

	c = ngx_palloc(env->pool, sizeof(fds_object_connect_t));
	c->req = ngx_palloc(env->pool, sizeof(fds_req_t));
	/*TODO memzero*/
	c->req->op = FDS_READ_OBJ;
	c->req->key = o->key;
	c->timeout = FDS_CONN_TIMEOUT;	

	/*TODO need to implement hash and peer_get*/
	o->hash_key = fds_hash(o->key);
	c->peer = fds_peer_get(o->hash_key);


	rc = ngx_event_connect_peer(&c->peer);
	o->peer->connection->data = c->req;

	/*TODO implement fds_object_read_handler*/
	o->peer->connection->read->handler = fds_object_read_handler;

	/*TODO*/
	o->peer->connection->send(o->peer->connection, c->req, sizeof(c->req));
	ngx_handle_read_event(o->peer->connection->read, 0);
}

