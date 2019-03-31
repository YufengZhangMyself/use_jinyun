/*
 * cloud_register.c
 *
 *  Created on: 2019年3月27日
 *      Author: Feng
 */

#include "esp_common.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/esp_libc.h"

#include "connect_net/cloud_register.h"
#include "connect_net/connect_cloud.h"



void cloud_register_task(void *pvParameters)
{
	char *RecvBuffer;
	SOCKET_PDB *msg_socket = (SOCKET_PDB *) pvParameters;

	RecvBuffer = malloc( sizeof( char ) * 1024 );
	if( RecvBuffer == NULL )
	{
		printf("tcp server Task error !!! \r\n");
		free( RecvBuffer );
		vTaskDelete(NULL);
		return;
	}

	*msg_socket = TCP_connect_get_socket( CLOUD_HTTP_SERVER, CLOUD_HTTP_SERVER_PORT);

	//向氦氪云发送注册设备请求
	memset(RecvBuffer, 0, 1024 * sizeof(char) );
	sprintf(RecvBuffer,"(login \"My8266_8F_ccd29be70139\" \"code\" \"azBZTWJHK3l6ZkRwMmFjWEVNVU9PcmwzcjNYY1BNNXpYc0ZNd1BpN2s3T0hJWHRIL0NJZDVJNkhmVjdiRmRwdkFJ\" \"DEVICE\")\r\n");
	write( msg_socket->sockfd ,  RecvBuffer , strlen(RecvBuffer) );
	printf("HeartBeatCmd_Sended : %s \n", RecvBuffer);

	int ReadLen;
	memset(RecvBuffer, 0, 1024 * sizeof(char) );
	ReadLen = read( msg_socket->sockfd , RecvBuffer , 1024 );
	printf("TCP Recv: %s \r\n\r\n",  RecvBuffer);

	if( ReadLen > 0 )
	{
		if(strncmp(RecvBuffer,"(getall )",strlen(RecvBuffer))==0)
		{
			memset(RecvBuffer, 0, 1024 * sizeof(char) );
			sprintf(RecvBuffer,"(setTermDetail \"pid\" \"1\" \"mid\" \"2\" \"cid\" \"1\" \"provider\" \"LSA\" \"category\" \"yuba\" \"model\" \"phone\" \"400-800-999\")\r\n");
			write( msg_socket->sockfd ,  RecvBuffer , strlen(RecvBuffer) );
			printf("HeartBeatCmd_Sended : %s \n", RecvBuffer);
		}
	}

	free( RecvBuffer );
	vTaskResume(tcp_server_task_handle);
	vTaskDelete(NULL);
}

