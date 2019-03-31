/*
 * connect_cloud.c
 *
 *  Created on: 2019年3月25日
 *      Author: Feng
 */

#include "esp_common.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/esp_libc.h"

#include "connect_net/cloud_register.h"
#include "connect_net/connect_cloud.h"


/*
 * 函数：connect_get_socket
 * 名称：通过域名获取socket连接
 * 参数：Url：域名
 * 		port：端口号
 * 		return_socket：得到连接成功的套接字
 * 返回值：true：成功
 * 		  fasle：失败
 */
static bool connect_get_socket( char *Url , uint16_t port, SOCKET_PDB *return_socket )
{
	char loop_count = 0;//函数循环执行次数统计
	int ret = 0;		//返回解析状态码，0为成功

	if(return_socket == NULL)
	{
		printf("Error : No socket recv addr ! \n");
		return false;
	}
	if(Url == NULL)
	{
		printf("Error : url is null ! \n");
		return false;
	}

	/*
	 * 判断是否连接WiFi
	 */
	STATION_STATUS StaStatus;
	do{
		if(loop_count++ > 0)
		{
			if(loop_count >= 10)
			{
				loop_count = 1;
				printf("Error : driver no connect wifi !\n");
			}
			vTaskDelay( 1000 / portTICK_RATE_MS );
		}
		StaStatus = wifi_station_get_connect_status( );
	}while( StaStatus != STATION_GOT_IP );

	/*
	 * 使用DNS解析域名获取目标IP地址
	 */
	struct  hostent hostinfo;	//返回储存数据结果
	char buf[101];				//解析过程临时缓存区
	struct  hostent *result;	//返回数据结果指针
	int h_errnop;				//储存错误码

	loop_count = 0;
	do{
		if(loop_count++ > 0)
		{
			if(loop_count >= 20)
			{
				printf("Error : DNS analysis error ! \n");
				return false;
			}
			printf("Error : Again DNS Url ! \n");
			vTaskDelay( 1000 / portTICK_RATE_MS );
		}
		ret = gethostbyname_r( Url,  &hostinfo,  buf,  100,  &result , &h_errnop);
	}while( (ret != 0) || (result == NULL) );

	/*
	 * 转换目标IP地址格式
	 */
	printf("Host:%s\r\n" , Url );
	int i = 0;
	char *ipaddr = NULL;
	for( i = 0; hostinfo.h_addr_list[i]; i++ )
	{
		ipaddr = inet_ntoa( *(struct in_addr*)hostinfo.h_addr_list[i] );
		if( ipaddr != NULL )
		{
			printf("Host addr is:%s\n",  ipaddr );
			break;
		}
		else
		{
			printf("Error : get ip fail ! \r\n");
			return false;
		}
	}

	/*
	 * 获取socket客户端
	 */
	loop_count = 0;
	do{
		if(loop_count++ > 0)
		{
			printf("Error : client socket fail!\r\n");
			if(loop_count > 3) return false;
		}
		return_socket->sockfd = socket( PF_INET, SOCK_STREAM, 0 );
	}while( return_socket->sockfd == -1 );
	printf("CliSock:%d!\r\n" , return_socket->sockfd );

	/*
	 * 使用socket连接目标服务器
	 */
	struct sockaddr_in address;
	memset( &address , 0 , sizeof( address ) );

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr( ipaddr );
	address.sin_port = htons(port);
	address.sin_len = sizeof(address);

	loop_count = 0;
	do{
		if(loop_count++ > 0)
		{
			if(loop_count > 3) return false;
			printf("Error : client cannot connect server!\r\n");
			vTaskDelay( 1000 / portTICK_RATE_MS );
		}
		ret = connect( return_socket->sockfd, (struct sockaddr *)&address, sizeof(address) );
	}while( ret == -1 );
	printf("client connect server successful!\r\n");

	/*
	 * 保存通信参数
	 */
	return_socket->ip_addr = ipaddr;
	return_socket->ip_port = port;

	return true;
}

/*
 * 函数：TCP_connect_get_socket
 * 名称：获取连接成功的socket，该函数为阻塞函数
 * 参数：Url：域名
 * 		port：端口号
 * 返回值：return_socket
 */
SOCKET_PDB ICACHE_FLASH_ATTR
TCP_connect_get_socket( char *Url , uint16_t port)
{
	SOCKET_PDB return_socket;
	while(connect_get_socket( Url, port, &return_socket ) == false);
	return return_socket;
}





xTaskHandle tcp_server_task_handle;

void ICACHE_FLASH_ATTR
user_tcp_recv_task(void *pvParameters)
{
	SOCKET_PDB *msg_socket = (SOCKET_PDB *) pvParameters;

	char * data_buf;
	data_buf = (char *)malloc( 1024 * sizeof(char) );
	if( data_buf == NULL )
	{
		printf("client buf get fail!\r\n");
		free( data_buf );
		vTaskDelete(NULL);
		return;
	}
	memset(data_buf, 0, 1024 * sizeof(char) );

	int ReadLen;
	for( ; ; )
	{
		memset(data_buf, 0, 1024 * sizeof(char) );
		ReadLen = read( msg_socket->sockfd , data_buf , 1024 );
		printf("TCP Recv: %s \r\n\r\n",  data_buf);

		if( ReadLen > 0 )
		{
			//连接氦氪云处理
			char *PA, *PB;
			PA = strstr(data_buf,"(+ ");
			if(PA)
			{
				PA = PA + strlen("(+ ");
				PB = PA + 2;
				switch(*PA)
				{
					case '0': os_printf("红色占空比：%s\r\n",PB); break;
					case '1': os_printf("蓝色占空比：%s\r\n",PB); break;
					case '2': os_printf("绿色占空比：%s\r\n",PB); break;
				}
			}
		}
	}

	free( data_buf );
	vTaskDelete(NULL);
}


void tcp_server_task(void *pvParameters)
{
	SOCKET_PDB msg_socket;
	char *RecvBuffer;
	printf("tcp server Task start !!! \r\n");

	RecvBuffer = malloc( sizeof( char ) * 1024 );
	if( RecvBuffer == NULL )
	{
		printf("tcp server Task error !!! \r\n");
		vTaskDelete(NULL);
		return;
	}

	xTaskCreate(cloud_register_task, "cloud_register_task", 256, &msg_socket, 3, NULL);
	vTaskSuspend(tcp_server_task_handle);
	xTaskCreate(user_tcp_recv_task, "user_tcp_recv_task", 256, &msg_socket, 4, NULL);

	unsigned char temp =0;
	for( ; ; )
	{
		memset(RecvBuffer, 0, 1024);
		if(++temp>250) temp =0;
		sprintf(RecvBuffer,"(mcastTermFormatState \"temp\" %d)\r\n", temp);
		write( msg_socket.sockfd ,  RecvBuffer , strlen(RecvBuffer) );
		printf("HeartBeatCmd_Sended : %s", RecvBuffer);

		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
	free( RecvBuffer );
	close(msg_socket.sockfd);
	vTaskDelete(NULL);
}


void tcp_server_init(void)
{
	xTaskCreate(tcp_server_task, "tcp_server_task", 256, NULL, 3, &tcp_server_task_handle);
}

