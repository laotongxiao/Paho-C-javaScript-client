/*
 * 订阅成功后发布信息
 * 它没有开启重连功参
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include "MQTTAsync.h"

#define ADDRESS     "tcp://localhost:61613"
#define CLIENTID    "test01"
#define TOPIC       "test02"
#define PAYLOAD     "Sub Hello World!"
#define PAYLOADSED   "Pus Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define USERNAME    "admin"
#define PASWORD    "123456"

volatile MQTTAsync_token deliveredtoken;

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
    printf("Successful disconnection\n");
    disc_finished = 1;
}

void onSend(void* context, MQTTAsync_successData* response)
{
    printf("-------onSend--------\n");
    printf("Pus succeeded\n");
    printf("Pus Message with token value %d delivery confirmed\n", response->token);
}

void onPublicationFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Publication failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
    printf("-------onSubscribe--------\n");
    //订阅成功后发布
    printf("Subscribe succeeded\n");
    printf("Subscribe Message with token value %d delivery confirmed\n", response->token);
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    opts.onSuccess = onSend;
    opts.onFailure = onPublicationFailure;
    opts.context = client;
    pubmsg.payload = PAYLOADSED;
    pubmsg.payloadlen = strlen(PAYLOADSED);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;
    if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
    printf("-------onConnect--------\n");
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Successful connection\n");

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;

    deliveredtoken = 0;
    //订阅
    if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char* argv[])
{
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    int rc;
    int ch;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.username = USERNAME;
    conn_opts.password = PASWORD;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    while	(!subscribed)
#if defined(WIN32) || defined(WIN64)
        Sleep(100);
#else
    usleep(10000L);
#endif

    if (finished)
        goto exit;

    do
    {
        ch = getchar();
    } while (ch!='Q' && ch != 'q');

    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    while	(!disc_finished)
#if defined(WIN32) || defined(WIN64)
        Sleep(100);
#else
    usleep(10000L);
#endif

    exit:
    MQTTAsync_destroy(&client);
    return rc;
}
