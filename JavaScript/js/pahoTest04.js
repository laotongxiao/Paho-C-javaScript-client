/*
说明：
最简单的订阅与发布，完成就马上断开和服备器链接
*/
var username = "admin";
var passWord = "123456";
var hostname = "localhost";  //替换成的你百度实例地址
var port = "61623";    //使用WSS协议的接口地址
var path = "/mqtt";
var clientId = makeid();
 
var client = new Paho.Client(hostname, Number(port), path, clientId);
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;
var options = {
    invocationContext: { host: hostname, port: port, path: path, clientId: clientId }, //它的做用只是收积参数传到onConnect那边去
    //timeout: 5,
    keepAliveInterval: 60,
    //cleanSession: true,
    //useSSL: true,
    reconnect: true, //打开断开重连
    onSuccess: onConnect,
    //mqttVersion: 4
};
 
options.userName = username;
options.password = passWord;
client.connect(options);

function onConnect(context) {
  // Once a connection has been made, make a subscription and send a message.
  var connectionString = context.invocationContext.host + ":" + context.invocationContext.port + context.invocationContext.path; //接收上面context传来参数
  console.log("-------onConnect-------" + connectionString);
  lao_subscribe();
};
function onConnectionLost(responseObject) {
  console.log("-------onConnectionLost-------");
  //alert(JSON.stringify(responseObject, null, 4));//输出对象对调试代码很有用
  if (responseObject.errorCode !== 0)
	console.log("onConnectionLost:"+responseObject.errorMessage);
};
function onMessageArrived(message) {
  console.log("------onMessageArrived:-------"+message.payloadString); //收到发布
  //client.disconnect(); //关闭链接
};	
 
 //发布没有回调函数执行options onSuccess C语言中就有
 function lao_publish(Content) {
	console.log("-------lao_publish-------");
    var topic = "/World";
    var qos = 0;
    var message;
    var retain = false;
	var messageContent = Content;
    message = new Paho.Message(messageContent);
    message.destinationName = topic;
    message.qos = Number(qos);
    message.retained = retain;
    client.send(message);
}
 
 
function lao_subscribe(){
	console.log("-------lao_subscribe-------");
	var options = {
		invocationContext: { test01: "test01value", test02: "test02value"}, //它的做用只是收积参数传到onConnect那边去
		//timeout: 5,
		qos: 0, //打开断开重连
		onSuccess: onSuccessSubscribe,
		onFailure: onFailureSubscribe,
	};
    client.subscribe("/World", options); //订阅
}
function onSuccessSubscribe(context){
	console.log("-------onSuccessSubscribe-------" + "订阅成功/World");
	console.log("-------invocationContext-------" + context.invocationContext.test01);
}
 
function onFailureSubscribe(context){
	console.log("-------onFailureSubscribe-------" + "订阅失败/World");
	console.log("-------invocationContext-------" + context.invocationContext.test02);
} 
 
 
 
 
 
 
function makeid() {
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
 
    for (var i = 0; i < 15; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));
 
    return text;
}
