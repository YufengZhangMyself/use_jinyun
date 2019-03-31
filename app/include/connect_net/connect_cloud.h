/*
 * connect_cloud.h
 *
 *  Created on: 2019Äê3ÔÂ25ÈÕ
 *      Author: Feng
 */

#ifndef APP_INCLUDE_CONNECT_NET_CONNECT_CLOUD_H_
#define APP_INCLUDE_CONNECT_NET_CONNECT_CLOUD_H_

typedef struct socket_PDB {
	int32 sockfd;
	char *ip_addr;
	uint16_t ip_port;
}SOCKET_PDB;


//#define CLOUD_HTTP_SERVER      		"cn.ice.rdzhiyun.cn"
//#define CLOUD_HTTP_SERVER_PORT		8000

#define CLOUD_HTTP_SERVER      		"device.hekr.me"
#define CLOUD_HTTP_SERVER_PORT		9999


extern xTaskHandle tcp_server_task_handle;

extern void tcp_server_init(void);
extern SOCKET_PDB TCP_connect_get_socket( char *Url , uint16_t port);


#endif /* APP_INCLUDE_CONNECT_NET_CONNECT_CLOUD_H_ */
