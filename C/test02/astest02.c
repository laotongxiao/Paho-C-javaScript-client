/*
 * 发布信息
 * 断网重连conn_opts.automaticReconnect = 1; 把onConnectFailure中的finished = 0;设成这样
 * 把表示异常退，所有exit(EXIT_FAILURE)都去掉;
 * 注意（重连是没有再次进入onConnect这里提示了）
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include "MQTTAsync.h"

#define ADDRESS     "tcp://localhost:61613" //"tcp://localhost:61613"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define USERNAME    "admin"
#define PASWORD    "123456"

volatile MQTTAsync_token deliveredtoken;

int finished = 0;

void connlost(void *context, char *cause)
{
    printf("--------connlost---------\n");
    finished = 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
    printf("--------onDisconnect---------\n");
    printf("Successful disconnection\n");
    finished = 1;
}


void onSend(void* context, MQTTAsync_successData* response)
{
    printf("--------onSend--------\n");
    printf("Message with token value %d delivery confirmed\n", response->token);

}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("--------onConnectFailure--------\n");
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 0;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
    printf("--------onConnect--------\n");
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Successful connection\n");

    opts.onSuccess = onSend;
    opts.context = client;

    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;
    if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
    }
}


int main(int argc, char* argv[])
{
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_token token;
    int rc;
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connlost, NULL, NULL);//表示connlost丢失就也会退出和服务器连接，等于exit(EXIT_FAILURE)
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.automaticReconnect = 1;
    //conn_opts.connectTimeout = 6;
    conn_opts.username = USERNAME;
    conn_opts.password = PASWORD;
    rc = MQTTAsync_connect(client, &conn_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
    }

    printf("Waiting for publication of %s\n"
           "on topic %s for client with ClientID: %s\n",
           PAYLOAD, TOPIC, CLIENTID);

    while (!finished)
#if defined(WIN32) || defined(WIN64)
        Sleep(100);
#else
    usleep(10000L);
#endif
    //MQTTAsync_destroy(&client);
    return rc;
}