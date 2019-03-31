# 使用ESP8266_RTOS_SDK创建项目连接氦氪云 #

----------

ESP8266 SDK 是基于 FreeRTOS.
   

## 注册 ##

1、创建注册任务 “cloud_register_task”

2、使用 “TCP_connect_get_socket” 获取连接完成的 socket

3、发送一串包含 “login” 注册字符到云端，等待云端响应，若响应数据中包含 “(getall )” 字符串
      则注册成功

  
## 设置 ##

1、注册成功后，发送包含 “setTermDetail” 字符串对设备做适当的配置

2、配置无需等待应答成功

## 使用 ##

1、创建数据接收任务 “user_tcp_recv_task” ，实时检测是否收到数据

2、使用注册过程获取的 socket 就可以直接通信，氦氪云是使用明文通信的

