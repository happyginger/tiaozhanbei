__root const char INDEX_HTML[] ="<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>"\
"<html>"\
"<head>"\
"<meta http-equiv='content-type' content='text/html; charset=gb2312' />"\
"<title>HTML5 Camera</title>"\
"<style>"\
"body{font-family:'宋体',Arial;}"\
"p{margin-top:5px;margin-bottom:5px;}"\
"#body{width:760px; position:relative;margin:0 auto; text-align:center;}"\
"#body h3{font-size:x-large; font-weight:600;}"\
".filter-drop-shadow {"\
"-webkit-filter: drop-shadow(0 1px 15px rgba(0,0,0,.8));"\
"-moz-filter: drop-shadow(0 1px 15px rgba(0,0,0,.8));"\
"-ms-filter: drop-shadow(0 1px 15px rgba(0,0,0,.8));"\
"-o-filter: drop-shadow(0 1px 15px rgba(0,0,0,.8));"\
"filter: drop-shadow(0 1px 15px rgba(0,0,0,.8));"\
"}"\
".img{margin:0 auto; display:block;margin-bottom:10px; width:640px;height:480px;cursor:pointer;}"\
"</style>"\
"<script>"\
"var timer_is_on = 0;"\
"var t_update;"\
"function $(o) {return (document.getElementById(o));}"\
"function AJAX(url, callback){"\
"var req = AJAX_init();"\
"req.onreadystatechange = AJAX_processRequest;"\
"function AJAX_init() {"\
"if (window.XMLHttpRequest) {"\
"return new XMLHttpRequest();"\
"} else if (window.ActiveXObject) {"\
"return new ActiveXObject('Microsoft.XMLHTTP');"\
"}"\
"};"\
"function AJAX_processRequest() {"\
"if(req.readyState == 4) {"\
"if(req.status == 200) {"\
"if(callback)"\
"callback(req.responseText);"\
"}"\
"}"\
"};"\
"this.doGet = function() {"\
"req.open('GET', url, true);"\
"req.send(null);"\
"};"\
"this.doPost = function(body) {"\
"req.open('POST', url, true);"\
"req.setRequestHeader('Content-Type',"\
"'application/x-www-form-urlencoded');"\
"req.setRequestHeader('ISAJAX','yes');"\
"req.send(body);"\
"};"\
"};"\
"function updateCamCallback(o) {"\
"obj=$('cam');"\
"obj.width=640;obj.height=480;obj.src=o;"\
"}"\
"function updateCam(){"\
"var oUpdate=$('cam');"\
"t_update=setTimeout(function () {"\
"oUpdate.src='camera.jpg';},10);"\
"t_update=setTimeout('updateCam()',20);"\
"}"\
"function doUpdateCallback(o){"\
"if(o){"\
"if(o['camera']=='1'){"\
"timer_is_on=1;"\
"updateCam();"\
"}else if(o['camera']=='0'){"\
"clearTimeout(t_update);"\
"timer_is_on=0;"\
"}"\
"}"\
"}"\
"function doUpdate(){"\
"var oAjax;"\
"var val;"\
"if (!timer_is_on) {"\
"val='1';"\
"} else {"\
"val='0';"\
"}"\
"setTimeout(function(){oAjax=new AJAX('camera.cgi',function(t){try{eval(t);}catch(e){alert(e);}});"\
"oAjax.doPost('camera='+val);},10);"\
"}"\
"function showWeiboLogin() {"\
"var o = $('weiboLogin');"\
"var oWeibo = $('txtWeibo');"\
"if (o.style.display == 'block') { o.style.display = 'none'; oWeibo.value = '#智能硬件#'; oWeibo.focus(); }"\
"else { o.style.display = 'block'; $('txtWeiboId').focus(); oWeibo.value = '#W5500抽奖#@wiznet'; }"\
"}"\
"function postWeibo() {"\
"var id = $('txtWeiboId').value;"\
"var pwd = $('txtWeiboPwd').value;"\
"var weibo = $('txtWeibo').value;"\
"var oPost;"\
"setTimeout(function () {"\
"oPost = new AJAX('postweibo.cgi', function (t) { try { eval(t); } catch (e) { alert(e); } });"\
"oPost.doPost('id=' + id + '&pwd=' + pwd + '&weibo=' + encodeURI(encodeURI(weibo)));}, 10);"\
"}"\
"function postWeiboCallback(o) {"\
"var obj = $('lblMsg');"\
"if(o['post']=='1'){obj.innerHTML = '谢谢您的支持，获奖信息将在我们的官方微博公布。敬请关注！';$('txtWeiboId').value='';$('txtWeiboPwd').value='';$('txtWeibo').value='#W5500抽奖#@wiznet';}"\
"else{obj.innerHTML = '发送失败！请再试一次。';$('txtWeiboId').value='';$('txtWeiboPwd').value='';$('txtWeibo').value='#W5500抽奖#@wiznet';}"\
"}"\
"</script>"\
"</head>"\
"<body>"\
"<div id='body'>"\
"<h3>网络摄像头</h3>"\
"<!--p><img onclick='doUpdate();' src='test.jpg' class='img filter-drop-shadow' id='cam2' title='摆好姿势，轻击画面完成拍照！' alt='W5500照相机' /></p-->"\
"<p><canvas  class='img filter-drop-shadow' id='cam' title='摆好姿势，轻击画面完成拍照！' alt='W5500照相机'></canvas></p>"\
"<div id='output'></div>"\
"<script type='text/javascript'>"\
"var wsServer = 'ws://192.168.1.111:1818/camera';"\
"var canvas = document.getElementById('cam');"\
"var context = canvas.getContext('2d');"\
"var websocket = new WebSocket(wsServer);"\
"websocket.onopen = function (evt) {"\
"writeToScreen('已经建立连接');"\
"};"\
"websocket.onclose = function (evt) {"\
"writeToScreen('连接关闭');"\
"};"\
"websocket.onerror = function (evt) {"\
"alert('哎呦，出错啦！');"\
"};"\
"websocket.onmessage = function (evt) {"\
"var image = new Image();"\
"image.src = URL.createObjectURL(evt.data);"\
"image.onload = function () {"\
"if(image){context.clearRect(0, 0,canvas.width, canvas.height);"\
"context.drawImage(image, 0, 0, canvas.width, canvas.height);}"\
"};"\
"};"\
"websocket.onerror = function (evt) {"\
"alert(evt.message);"\
"};"\
"function sendMsg() {"\
"if (websocket.readyState == websocket.OPEN) {"\
"msg = document.getElementById('msg').value;"\
"websocket.send(msg);"\
"writeToScreen('发送成功!');"\
"} else {"\
"writeToScreen('连接失败!');"\
"}"\
"};"\
"function writeToScreen(message) {"\
"var pre = document.createElement('p');"\
"pre.style.wordWrap = 'break-word';"\
"pre.innerHTML += message;"\
"var output =document.getElementById('output');"\
"output.appendChild(pre);"\
"}"\
"</script>"\
"<div style='margin:5px 5px;'>"\
"&copy;Copyright 1998-2013 by WIZnet Team"\
"</div>"\
"</div>"\
"</body>"\
"</html>";

__root const char LOGO_PNG[2189]={0x82,0x7e,0x08,0x89,0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
0x00,0x00,0x00,0x67,0x00,0x00,0x00,0x2c,0x08,0x03,0x00,0x00,0x00,0x4f,0x00,0x56,
0x64,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,0x53,0x6f,0x66,0x74,0x77,0x61,0x72,
0x65,0x00,0x41,0x64,0x6f,0x62,0x65,0x20,0x49,0x6d,0x61,0x67,0x65,0x52,0x65,0x61,
0x64,0x79,0x71,0xc9,0x65,0x3c,0x00,0x00,0x03,0x20,0x69,0x54,0x58,0x74,0x58,0x4d,
0x4c,0x3a,0x63,0x6f,0x6d,0x2e,0x61,0x64,0x6f,0x62,0x65,0x2e,0x78,0x6d,0x70,0x00,
0x00,0x00,0x00,0x00,0x3c,0x3f,0x78,0x70,0x61,0x63,0x6b,0x65,0x74,0x20,0x62,0x65,
0x67,0x69,0x6e,0x3d,0x22,0xef,0xbb,0xbf,0x22,0x20,0x69,0x64,0x3d,0x22,0x57,0x35,
0x4d,0x30,0x4d,0x70,0x43,0x65,0x68,0x69,0x48,0x7a,0x72,0x65,0x53,0x7a,0x4e,0x54,
0x63,0x7a,0x6b,0x63,0x39,0x64,0x22,0x3f,0x3e,0x20,0x3c,0x78,0x3a,0x78,0x6d,0x70,
0x6d,0x65,0x74,0x61,0x20,0x78,0x6d,0x6c,0x6e,0x73,0x3a,0x78,0x3d,0x22,0x61,0x64,
0x6f,0x62,0x65,0x3a,0x6e,0x73,0x3a,0x6d,0x65,0x74,0x61,0x2f,0x22,0x20,0x78,0x3a,
0x78,0x6d,0x70,0x74,0x6b,0x3d,0x22,0x41,0x64,0x6f,0x62,0x65,0x20,0x58,0x4d,0x50,
0x20,0x43,0x6f,0x72,0x65,0x20,0x35,0x2e,0x30,0x2d,0x63,0x30,0x36,0x30,0x20,0x36,
0x31,0x2e,0x31,0x33,0x34,0x37,0x37,0x37,0x2c,0x20,0x32,0x30,0x31,0x30,0x2f,0x30,
0x32,0x2f,0x31,0x32,0x2d,0x31,0x37,0x3a,0x33,0x32,0x3a,0x30,0x30,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x22,0x3e,0x20,0x3c,0x72,0x64,0x66,0x3a,0x52,0x44,0x46,
0x20,0x78,0x6d,0x6c,0x6e,0x73,0x3a,0x72,0x64,0x66,0x3d,0x22,0x68,0x74,0x74,0x70,
0x3a,0x2f,0x2f,0x77,0x77,0x77,0x2e,0x77,0x33,0x2e,0x6f,0x72,0x67,0x2f,0x31,0x39,
0x39,0x39,0x2f,0x30,0x32,0x2f,0x32,0x32,0x2d,0x72,0x64,0x66,0x2d,0x73,0x79,0x6e,
0x74,0x61,0x78,0x2d,0x6e,0x73,0x23,0x22,0x3e,0x20,0x3c,0x72,0x64,0x66,0x3a,0x44,
0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6f,0x6e,0x20,0x72,0x64,0x66,0x3a,0x61,
0x62,0x6f,0x75,0x74,0x3d,0x22,0x22,0x20,0x78,0x6d,0x6c,0x6e,0x73,0x3a,0x78,0x6d,
0x70,0x3d,0x22,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x6e,0x73,0x2e,0x61,0x64,0x6f,
0x62,0x65,0x2e,0x63,0x6f,0x6d,0x2f,0x78,0x61,0x70,0x2f,0x31,0x2e,0x30,0x2f,0x22,
0x20,0x78,0x6d,0x6c,0x6e,0x73,0x3a,0x78,0x6d,0x70,0x4d,0x4d,0x3d,0x22,0x68,0x74,
0x74,0x70,0x3a,0x2f,0x2f,0x6e,0x73,0x2e,0x61,0x64,0x6f,0x62,0x65,0x2e,0x63,0x6f,
0x6d,0x2f,0x78,0x61,0x70,0x2f,0x31,0x2e,0x30,0x2f,0x6d,0x6d,0x2f,0x22,0x20,0x78,
0x6d,0x6c,0x6e,0x73,0x3a,0x73,0x74,0x52,0x65,0x66,0x3d,0x22,0x68,0x74,0x74,0x70,
0x3a,0x2f,0x2f,0x6e,0x73,0x2e,0x61,0x64,0x6f,0x62,0x65,0x2e,0x63,0x6f,0x6d,0x2f,
0x78,0x61,0x70,0x2f,0x31,0x2e,0x30,0x2f,0x73,0x54,0x79,0x70,0x65,0x2f,0x52,0x65,
0x73,0x6f,0x75,0x72,0x63,0x65,0x52,0x65,0x66,0x23,0x22,0x20,0x78,0x6d,0x70,0x3a,
0x43,0x72,0x65,0x61,0x74,0x6f,0x72,0x54,0x6f,0x6f,0x6c,0x3d,0x22,0x41,0x64,0x6f,
0x62,0x65,0x20,0x50,0x68,0x6f,0x74,0x6f,0x73,0x68,0x6f,0x70,0x20,0x43,0x53,0x35,
0x20,0x57,0x69,0x6e,0x64,0x6f,0x77,0x73,0x22,0x20,0x78,0x6d,0x70,0x4d,0x4d,0x3a,
0x49,0x6e,0x73,0x74,0x61,0x6e,0x63,0x65,0x49,0x44,0x3d,0x22,0x78,0x6d,0x70,0x2e,
0x69,0x69,0x64,0x3a,0x34,0x35,0x33,0x45,0x44,0x34,0x41,0x44,0x46,0x34,0x44,0x42,
0x31,0x31,0x45,0x32,0x38,0x38,0x44,0x38,0x45,0x33,0x30,0x44,0x44,0x43,0x45,0x39,
0x32,0x33,0x45,0x46,0x22,0x20,0x78,0x6d,0x70,0x4d,0x4d,0x3a,0x44,0x6f,0x63,0x75,
0x6d,0x65,0x6e,0x74,0x49,0x44,0x3d,0x22,0x78,0x6d,0x70,0x2e,0x64,0x69,0x64,0x3a,
0x34,0x35,0x33,0x45,0x44,0x34,0x41,0x45,0x46,0x34,0x44,0x42,0x31,0x31,0x45,0x32,
0x38,0x38,0x44,0x38,0x45,0x33,0x30,0x44,0x44,0x43,0x45,0x39,0x32,0x33,0x45,0x46,
0x22,0x3e,0x20,0x3c,0x78,0x6d,0x70,0x4d,0x4d,0x3a,0x44,0x65,0x72,0x69,0x76,0x65,
0x64,0x46,0x72,0x6f,0x6d,0x20,0x73,0x74,0x52,0x65,0x66,0x3a,0x69,0x6e,0x73,0x74,
0x61,0x6e,0x63,0x65,0x49,0x44,0x3d,0x22,0x78,0x6d,0x70,0x2e,0x69,0x69,0x64,0x3a,
0x34,0x35,0x33,0x45,0x44,0x34,0x41,0x42,0x46,0x34,0x44,0x42,0x31,0x31,0x45,0x32,
0x38,0x38,0x44,0x38,0x45,0x33,0x30,0x44,0x44,0x43,0x45,0x39,0x32,0x33,0x45,0x46,
0x22,0x20,0x73,0x74,0x52,0x65,0x66,0x3a,0x64,0x6f,0x63,0x75,0x6d,0x65,0x6e,0x74,
0x49,0x44,0x3d,0x22,0x78,0x6d,0x70,0x2e,0x64,0x69,0x64,0x3a,0x34,0x35,0x33,0x45,
0x44,0x34,0x41,0x43,0x46,0x34,0x44,0x42,0x31,0x31,0x45,0x32,0x38,0x38,0x44,0x38,
0x45,0x33,0x30,0x44,0x44,0x43,0x45,0x39,0x32,0x33,0x45,0x46,0x22,0x2f,0x3e,0x20,
0x3c,0x2f,0x72,0x64,0x66,0x3a,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6f,
0x6e,0x3e,0x20,0x3c,0x2f,0x72,0x64,0x66,0x3a,0x52,0x44,0x46,0x3e,0x20,0x3c,0x2f,
0x78,0x3a,0x78,0x6d,0x70,0x6d,0x65,0x74,0x61,0x3e,0x20,0x3c,0x3f,0x78,0x70,0x61,
0x63,0x6b,0x65,0x74,0x20,0x65,0x6e,0x64,0x3d,0x22,0x72,0x22,0x3f,0x3e,0x6c,0xef,
0x2e,0x56,0x00,0x00,0x00,0x30,0x50,0x4c,0x54,0x45,0x0b,0x07,0x07,0xff,0xff,0xfe,
0xd2,0x15,0x2f,0x6f,0x6d,0x6d,0xe5,0x79,0x87,0xc1,0xbf,0xbf,0xf3,0xbf,0xc5,0xe7,
0xe5,0xe6,0xdc,0x45,0x59,0xf7,0xec,0xee,0xed,0x9d,0xa8,0xf8,0xd8,0xdc,0xf9,0xf6,
0xf6,0x9f,0x9d,0x9d,0xd6,0xd5,0xd5,0x3c,0x39,0x39,0x97,0x8e,0x62,0x8b,0x00,0x00,
0x04,0xc3,0x49,0x44,0x41,0x54,0x78,0xda,0xa4,0x57,0x09,0x92,0xe3,0x20,0x0c,0xc4,
0xc3,0x15,0xb0,0x04,0xff,0xff,0xed,0xea,0xe2,0xb0,0xd7,0x4e,0x4d,0xed,0x92,0x71,
0x26,0xb1,0x85,0x5a,0x47,0x4b,0x22,0xee,0x18,0x0b,0x32,0x1c,0xff,0xbd,0x00,0x9d,
0x73,0xf1,0xe1,0x81,0x1b,0x1f,0x0a,0xf6,0xb8,0x03,0x41,0xce,0xad,0x45,0x5a,0xf9,
0x2b,0x7e,0xb9,0x3e,0x05,0xcf,0x38,0xf0,0x8e,0x93,0x49,0x00,0x4d,0xb6,0xe4,0x1c,
0x11,0x7b,0x77,0xb2,0x7a,0xc7,0xf6,0x8a,0xd4,0xbc,0xf7,0xbb,0xda,0x4c,0x9b,0x7a,
0x7b,0xf7,0xa7,0xf1,0xf3,0x7c,0x90,0x0f,0x84,0xe0,0xbb,0x01,0x78,0x8f,0x5e,0xbe,
0xf9,0x37,0x9f,0x38,0x4c,0xb4,0x6f,0xe1,0x7e,0xc7,0x89,0xac,0xd9,0x43,0x5c,0x3e,
0x78,0x8c,0x19,0x58,0x39,0x5d,0xe4,0x6a,0xc7,0xf2,0x18,0x35,0xc6,0xf1,0x5b,0xd8,
0xbe,0xe3,0x64,0x51,0x8f,0x6a,0x1d,0x43,0x4c,0xa5,0x20,0xb1,0xe1,0xa7,0x3e,0x3f,
0xa5,0xdd,0xbb,0x19,0xee,0x69,0xf0,0x3b,0x8e,0x28,0xea,0xe5,0xc8,0xd8,0x31,0x02,
0xcc,0x18,0x35,0xc4,0x18,0xd1,0x7c,0x44,0xf8,0x5f,0x9c,0x22,0x9a,0x3c,0xa8,0xf9,
0xb9,0x8d,0x64,0x80,0xe2,0x23,0xaf,0x18,0x9f,0x76,0x17,0x0e,0xe9,0x4e,0xe3,0xe8,
0xbe,0xe0,0x64,0xa7,0x38,0x20,0x2c,0xa0,0xc0,0xe5,0x45,0x1e,0xe7,0x94,0x6c,0xcf,
0x3c,0x28,0xfe,0x56,0x2e,0xe2,0x4f,0x06,0xe2,0xeb,0x8d,0xdc,0x4e,0x8a,0xab,0x13,
0x01,0x88,0x5d,0x83,0x68,0x86,0x03,0x5d,0xc3,0xf9,0xbe,0xf2,0xbd,0x5c,0x04,0x07,
0x25,0xd4,0xf8,0xc0,0xb7,0x42,0x15,0xe9,0x9d,0xad,0x3e,0x25,0xf8,0x56,0xa7,0x52,
0x7d,0xad,0x1f,0x89,0xac,0x9f,0xb6,0x43,0x9e,0x4a,0x38,0x10,0x8f,0xfd,0x60,0xb0,
0xc1,0xe3,0x6c,0x08,0xad,0x2b,0xc7,0xaf,0x29,0xd8,0x61,0xe2,0xe0,0x08,0x59,0x4a,
0x31,0xf7,0x3b,0xcc,0xb5,0x2d,0xb8,0xdd,0x38,0x62,0x5b,0xa6,0x1a,0xc8,0x4c,0x34,
0x1c,0x51,0x1c,0x1c,0xb9,0x41,0x90,0x19,0xb8,0x24,0x48,0x78,0x49,0x4b,0x85,0x7b,
0xcc,0x2f,0xfe,0x48,0xc0,0x23,0xe7,0x2a,0xe3,0x6e,0x17,0xdd,0xb8,0xe3,0xc0,0xc5,
0x8c,0xbe,0x63,0x30,0x68,0xcc,0x62,0xc9,0x33,0x0e,0x93,0x44,0xf7,0xf6,0x28,0x3c,
0xb2,0xed,0xf8,0x90,0x9f,0xdc,0x2f,0x6a,0x4b,0xbc,0x00,0x7d,0xe9,0x6f,0x82,0xa1,
0x20,0x9e,0x2b,0xc5,0x73,0xfe,0xd5,0xad,0xf8,0x56,0x9f,0x63,0xa1,0xf4,0x39,0x4a,
0x2c,0x1a,0xdc,0x53,0xeb,0x20,0x9c,0x8c,0x5b,0xa0,0x35,0xe5,0x64,0x2f,0x55,0x9b,
0xa6,0x19,0x1f,0xfb,0xb4,0xd6,0x81,0x33,0xea,0xb4,0x82,0x6d,0x16,0x3c,0x01,0xb5,
0x27,0x1c,0x4b,0x86,0x7a,0x93,0xa9,0xd3,0x47,0x2d,0x03,0xdf,0x84,0x70,0xf8,0x48,
0x6a,0x2a,0x44,0xad,0x7e,0x55,0x3c,0x9b,0xf5,0xb0,0xb6,0xfc,0x8d,0xd3,0x9c,0x3a,
0xcd,0x78,0xc8,0x91,0x47,0xaa,0x3f,0xde,0x12,0x73,0xdf,0xb8,0x06,0xd7,0x31,0x08,
0xd6,0xe5,0xb5,0xf3,0xee,0x30,0x12,0x7f,0xff,0xd0,0x0f,0xa2,0x52,0x90,0xa2,0xe0,
0x4b,0xb4,0x46,0xc2,0x1d,0xa8,0x28,0xce,0xcc,0xfd,0xcd,0xb3,0x01,0x93,0xa1,0x80,
0x34,0x15,0x83,0x40,0xf9,0xe8,0xae,0xc4,0x76,0xab,0xe6,0xb8,0x03,0xf2,0x7c,0xdf,
0xe6,0x96,0xc8,0x17,0x1b,0x2d,0x97,0x81,0xb6,0x6a,0xd4,0x62,0x26,0x33,0x45,0x63,
0xc2,0xa3,0x44,0x5a,0xd6,0x36,0x60,0xdc,0x66,0x1d,0xa5,0x8f,0x89,0xd4,0x81,0xa9,
0xd1,0xc8,0xc8,0xb8,0xe1,0x70,0x32,0x76,0x4a,0x4c,0xf3,0xfd,0x1a,0x94,0x4d,0x62,
0x27,0x81,0x8e,0xd6,0x4b,0xd0,0xbc,0x72,0xcb,0x3a,0xa6,0x23,0x37,0x28,0x92,0x83,
0xe8,0xb9,0x3a,0x75,0x5e,0x2c,0x9c,0xd5,0x53,0xe5,0x5c,0x43,0x15,0xa0,0xe6,0x17,
0x01,0xe5,0xb0,0x0a,0x4e,0xde,0x4b,0x8c,0xb3,0x97,0x27,0x0e,0x2a,0x8c,0xf8,0x23,
0xf6,0xac,0xa6,0x32,0xaa,0x01,0xe7,0x8c,0xd0,0x11,0xdb,0x69,0xbb,0xe2,0xc8,0xc0,
0x55,0x2e,0x4b,0x3c,0xf2,0x94,0x1f,0xf4,0xf3,0xcd,0x8d,0x6d,0x56,0xc5,0x9c,0x9f,
0xe1,0xa2,0x42,0x8d,0xb1,0xb5,0x38,0xae,0x29,0x8f,0xe6,0xa3,0x57,0x67,0xca,0x6c,
0xc6,0xb6,0xa1,0xe1,0x6a,0x7a,0x8a,0x23,0xf4,0xe8,0x30,0x3a,0xca,0xea,0xce,0xc0,
0xac,0x53,0xdd,0xda,0xf1,0xfd,0xc0,0xf1,0xea,0x25,0xfb,0x23,0xe7,0xa1,0xb8,0x1f,
0x80,0xc6,0x17,0x3a,0xa0,0xc9,0x09,0x30,0xb6,0x5c,0x1c,0xa0,0xdf,0x87,0x8e,0xd0,
0x0e,0xf9,0x49,0x01,0x9b,0x0e,0x71,0x35,0x4d,0xab,0x25,0x3a,0x43,0x5a,0xfc,0x58,
0xb8,0x91,0xf4,0xce,0xda,0xe7,0xf3,0x28,0xf7,0x67,0xee,0x1c,0x2b,0xc3,0x5d,0x7b,
0x28,0xf3,0x40,0xd7,0xd6,0x32,0x6f,0x65,0xbe,0xb4,0x16,0x4b,0x1d,0xbe,0x74,0x2a,
0x47,0xa7,0xdb,0x76,0xed,0x7b,0xf1,0x32,0xad,0x2e,0x13,0x22,0x3e,0x1c,0x13,0x51,
0x23,0xec,0xcd,0xb2,0xc7,0x69,0xf5,0xd7,0xfc,0xb1,0xb4,0x6c,0x29,0x54,0xcf,0x68,
0x6c,0x51,0x67,0x6a,0x00,0xf7,0xf3,0x41,0xb7,0x80,0xc3,0x66,0x5d,0xcf,0xbf,0xc3,
0xd1,0xb4,0xb0,0xa3,0x51,0xfe,0x72,0xa6,0x6c,0x95,0x37,0xb9,0xf1,0xa0,0x34,0xf4,
0xdb,0xa4,0xf8,0x1d,0xce,0xbf,0xfd,0x28,0xb1,0xd3,0xf9,0xd3,0x01,0xce,0xce,0x3b,
0x05,0xec,0x55,0xfe,0xe1,0x47,0x50,0xb9,0x36,0xf2,0x92,0x5f,0xe6,0x69,0xfa,0xe8,
0xfa,0x91,0xf7,0x7a,0x1c,0x67,0xf8,0x84,0x64,0x02,0x81,0x6e,0x8d,0xcb,0xde,0x43,
0xe0,0x58,0x9d,0xf4,0xe1,0xb4,0xdd,0xc1,0x7a,0x6d,0xa5,0xed,0x60,0x1f,0xc6,0x86,
0x34,0x70,0xca,0xe7,0xe7,0xb2,0xd2,0x51,0xe5,0x5d,0x6d,0xbd,0x3d,0x1c,0x22,0x87,
0x08,0x11,0x4e,0x92,0x1b,0x0a,0xc4,0xc2,0x01,0xd8,0x84,0x9b,0xac,0xe0,0xf0,0x86,
0x10,0xaa,0xbc,0x58,0xa0,0x02,0xdd,0xf9,0x58,0x7a,0x13,0xdd,0x21,0x07,0xb6,0x25,
0x22,0x64,0x72,0xa0,0x07,0xa4,0x3a,0xa8,0xb6,0x70,0x1c,0xaa,0x9f,0x1f,0x5d,0xf6,
0x14,0xc3,0x91,0x0d,0xe9,0xd4,0x55,0x05,0x27,0xa8,0x59,0x87,0x59,0x9d,0x8a,0xe4,
0x4f,0x56,0x11,0xc5,0xa7,0x02,0x7c,0x54,0x75,0xa8,0x06,0x9d,0xd4,0x7c,0xb6,0xad,
0xaa,0x7c,0x19,0x95,0xe0,0x8e,0x22,0xf6,0x7c,0xc6,0xdf,0xcf,0xe7,0x1c,0xee,0x1f,
0x33,0x3a,0x65,0xcb,0x20,0x8b,0x80,0xc5,0x88,0x70,0x86,0x61,0x9c,0x87,0x3a,0xff,
0xfd,0x0c,0xe1,0x4f,0x18,0x38,0xe9,0x16,0xfb,0x0f,0x98,0xfb,0x92,0x59,0xd6,0x70,
0xde,0x65,0x38,0xe4,0xa7,0xfa,0x90,0x44,0x35,0x83,0x92,0x98,0x0a,0x0b,0xce,0x5c,
0x15,0x0c,0x87,0xe3,0x50,0xc7,0xe2,0x48,0xab,0xfb,0x30,0x69,0x40,0x11,0xde,0x53,
0x68,0x38,0x49,0xc3,0x57,0x45,0xb5,0x98,0x16,0xce,0xa0,0x84,0x60,0xb8,0x5a,0x53,
0xe5,0x2b,0x9d,0xe6,0x0f,0xfc,0xac,0x20,0x89,0x91,0x8a,0x43,0x19,0x4b,0xb4,0x2c,
0xe9,0x62,0x28,0xff,0xd0,0xa3,0x4b,0xf3,0x62,0x5c,0x91,0x6f,0x60,0xf1,0xfd,0x18,
0x1f,0xd8,0xb9,0x54,0xce,0xb3,0xd0,0x75,0x8e,0xfc,0xa4,0xc5,0x61,0x35,0x32,0x49,
0x1e,0xb7,0x38,0x26,0xc5,0x19,0xfd,0x85,0x1d,0x9a,0x00,0x96,0xa5,0x63,0x06,0xab,
0x9a,0xc4,0x5c,0x06,0xe4,0x86,0x41,0x5a,0x67,0x2b,0x91,0x3b,0xcc,0x4e,0x0c,0x50,
0x66,0x80,0xd1,0x60,0xe2,0x28,0xc1,0x85,0xc5,0x97,0x9a,0x1b,0x71,0xe3,0xf0,0xcc,
0xee,0x70,0xd6,0x94,0x58,0x32,0xd5,0x91,0x8d,0x7a,0x8a,0xee,0x30,0xfb,0x03,0x35,
0x8b,0x50,0x8b,0xde,0x22,0x7f,0x48,0x22,0x8d,0xfb,0xb3,0x2f,0x94,0x1a,0xe6,0xcb,
0xcc,0xfb,0x23,0xc0,0x00,0xb1,0xbd,0x41,0x7f,0xed,0x62,0xcc,0x74,0x00,0x00,0x00,
0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};