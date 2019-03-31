/*
 * connect_cloud.c
 *
 *  Created on: 2019��3��25��
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
 * ������connect_get_socket
 * ���ƣ�ͨ��������ȡsocket����
 * ������Url������
 * 		port���˿ں�
 * 		return_socket���õ����ӳɹ����׽���
 * ����ֵ��true���ɹ�
 * 		  fasle��ʧ��
 */
static bool connect_get_socket( char *Url , uint16_t port, SOCKET_PDB *return_socket )
{
	char loop_count = 0;//����ѭ��ִ�д���ͳ��
	int ret = 0;		//���ؽ���״̬�룬0Ϊ�ɹ�

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
	 * �ж��Ƿ�����WiFi
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
	 * ʹ��DNS����������ȡĿ��IP��ַ
	 */
	struct  hostent hostinfo;	//���ش������ݽ��
	char buf[101];				//����������ʱ������
	struct  hostent *result;	//�������ݽ��ָ��
	int h_errnop;				//���������

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
	 * ת��Ŀ��IP��ַ��ʽ
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
	 * ��ȡsocket�ͻ���
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
	 * ʹ��socket����Ŀ�������
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
	 * ����ͨ�Ų���
	 */
	return_socket->ip_addr = ipaddr;
	return_socket->ip_port = port;

	return true;
}

/*
 * ������TCP_connect_get_socket
 * ���ƣ���ȡ���ӳɹ���socket���ú���Ϊ��������
 * ������Url������
 * 		port���˿ں�
 * ����ֵ��return_socket
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
			//���Ӻ���ƴ���
			char *PA, *PB;
			PA = strstr(data_buf,"(+ ");
			if(PA)
			{
				PA = PA + strlen("(+ ");
				PB = PA + 2;
				switch(*PA)
				{
					case '0': os_printf("��ɫռ�ձȣ�%s\r\n",PB); break;
					case '1': os_printf("��ɫռ�ձȣ�%s\r\n",PB); break;
					case '2': os_printf("��ɫռ�ձȣ�%s\r\n",PB); break;
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

