__root const char INDEX_HTML[] ="<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<meta charset='gb2312' />"\
"<title>Wiznet 2014</title>"\
"<style>"\
"#graph {"\
"border: 1px solid #03F;"\
"margin:0 40px 0 80px;"\
"}"\
"#graph2 {"\
"border: 1px solid #03F;"\
"margin:0 0 0 0;"\
"}"\
"h1 {margin: 0 auto;text-align:center;}"\
"</style>"\
"<script>"\
"var arryTH = new Array(2);"\
"var preTPosX,preTPosY,preHPosX,preHPosY;"\
"var timeCounter = 0;"\
"var canvas;"\
"var context;"\
"var canvas2;"\
"var context2;"\
"var chartMargin;"\
"var chartAxisSpace;"\
"var chartWidth;"\
"var chartHeight;"\
"var maxValue = 0;"\
"var numYLabels;"\
"var numBars = 0;"\
"function initGraph() {"\
"canvas = document.getElementById('graph');"\
"context = canvas.getContext('2d');"\
"canvas2 = document.getElementById('graph2');"\
"context2 = canvas2.getContext('2d');"\
"initSettings();"\
"drawAxis();"\
"};"\
"function initSettings() {"\
"chartMargin = 20;"\
"chartAxisSpace = 50;"\
"chartHeight = canvas.height-chartAxisSpace-2*chartMargin;"\
"chartWidth = canvas.width-chartAxisSpace-2*chartMargin;"\
"numYLabels = 10;"\
"numBars = 15;"\
"maxValue = 100;"\
"};"\
"function drawAxis() {"\
"context.lineWidth = 2;"\
"context.strokeStyle = '#999';"\
"context.moveTo(chartMargin+chartAxisSpace, chartHeight+chartMargin);"\
"context.lineTo(chartMargin+chartAxisSpace, chartMargin);"\
"context.stroke();"\
"context.moveTo(chartMargin+chartAxisSpace, chartHeight+chartMargin);"\
"context.lineTo(chartMargin+chartAxisSpace+chartWidth, chartMargin+chartHeight);"\
"context.stroke();"\
"context.lineWidth = 1;"\
"var markerAmount = parseInt(maxValue/numYLabels);"\
"var markerLabel;"\
"var markerXPos;"\
"var markerYPos;"\
"context.textAlign = 'right';"\
"context.fillStyle = '#000';"\
"for(var i=0; i<=numYLabels; i++){"\
"markerLabel = i*markerAmount;"\
"markerXPos = chartMargin+chartAxisSpace - 5;"\
"markerYPos = chartMargin+(chartHeight-((i*markerAmount*chartHeight)/maxValue));"\
"context.fillText(markerLabel, markerXPos, markerYPos, chartAxisSpace);"\
"}"\
"context.textAlign = 'center';"\
"for(var i=0; i<=numBars; i++)"\
"{"\
"markerLabel = i;"\
"markerXPos = chartMargin+chartAxisSpace + i*30;"\
"markerYPos = chartMargin+chartHeight + 10;"\
"context.fillText(markerLabel, markerXPos, markerYPos, 40);"\
"}"\
"context.save();"\
"context.translate(chartMargin+10, chartHeight/2);"\
"context.rotate(Math.PI*-90/180);"\
"context.fillText('温度(/ ℃)',0,0);"\
"context.restore();"\
"context2.lineWidth = 2;"\
"context2.strokeStyle = '#999';"\
"context2.moveTo(chartMargin+chartAxisSpace, chartHeight+chartMargin);"\
"context2.lineTo(chartMargin+chartAxisSpace, chartMargin);"\
"context2.stroke();"\
"context2.moveTo(chartMargin+chartAxisSpace, chartHeight+chartMargin);"\
"context2.lineTo(chartMargin+chartAxisSpace+chartWidth, chartMargin+chartHeight);"\
"context2.stroke();"\
"context2.lineWidth = 1;"\
"context2.textAlign = 'right';"\
"context2.fillStyle = '#000';"\
"for(var i=0; i<=numYLabels; i++){"\
"markerLabel = i*markerAmount;"\
"markerXPos = chartMargin+chartAxisSpace - 5;"\
"markerYPos = chartMargin+(chartHeight-((i*markerAmount*chartHeight)/maxValue));"\
"context2.fillText(markerLabel, markerXPos, markerYPos, chartAxisSpace);"\
"}"\
"context2.textAlign = 'center';"\
"for(var i=0; i<=numBars; i++)"\
"{"\
"markerLabel = i;"\
"markerXPos = chartMargin+chartAxisSpace + i*30;"\
"markerYPos = chartMargin+chartHeight + 10;"\
"context2.fillText(markerLabel, markerXPos, markerYPos, 40);"\
"}"\
"context2.save();"\
"context2.translate(chartMargin+10, chartHeight/2);"\
"context2.rotate(Math.PI*-90/180);"\
"context2.fillText('湿度(/ %)',0,0);"\
"context2.restore();"\
"};"\
"var sensorWebSocket = null;"\
"var wsUri = 'ws://192.168.1.111:1818';"\
"function initWebSocket() {"\
"sensorWebSocket = new WebSocket(wsUri);"\
"sensorWebSocket.onopen = function(evt) {onOpen(evt)};"\
"sensorWebSocket.onclose = function(evt) {onClose(evt)};"\
"sensorWebSocket.onmessage = function(evt) {onMessage(evt)};"\
"sensorWebSocket.onerror = function(evt) {onError(evt)};"\
"};"\
"function onOpen(evt) {"\
"updateStatus('connected');}"\
"function onMessage(evt) {"\
"arrayTH = evt.data.split('.');"\
"var curTPosX,curTPosY,curHPosX,curHPosY;"\
"curTPosX = chartMargin+chartAxisSpace+(timeCounter+1)*30;"\
"curTPosY = (maxValue-arrayTH[0])*chartHeight/maxValue + chartMargin;"\
"curHPosX = chartMargin+chartAxisSpace+(timeCounter+1)*30;"\
"curHPosY = (maxValue-arrayTH[1])*chartHeight/maxValue + chartMargin;"\
"context.fillStyle='#000';"\
"context.beginPath();"\
"context.arc(curTPosX,curTPosY,2,0,Math.PI*2,true);"\
"context.closePath();"\
"context.fill();"\
"context.lineWidth = 1;"\
"context.strokeStyle = '#999';"\
"context.fillText('('+arrayTH[0]+')', curTPosX, curTPosY-5, chartAxisSpace);"\
"if(timeCounter!=0)"\
"{"\
"context.moveTo(curTPosX,curTPosY);"\
"context.lineTo(preTPosX,preTPosY);"\
"context.stroke();"\
"}"\
"context2.fillStyle='#000';"\
"context2.beginPath();"\
"context2.arc(curHPosX,curHPosY,2,0,Math.PI*2,true);"\
"context2.closePath();"\
"context2.fill();"\
"context2.lineWidth = 1;"\
"context2.strokeStyle = '#999';"\
"context2.fillText('('+arrayTH[1]+')', curHPosX, curHPosY-5, chartAxisSpace);"\
"if(timeCounter!=0)"\
"{"\
"context2.moveTo(curHPosX,curHPosY);"\
"context2.lineTo(preHPosX,preHPosY);"\
"context2.stroke();"\
"}"\
"preTPosX = curTPosX;"\
"preTPosY = curTPosY;"\
"preHPosX = curHPosX;"\
"preHPosY = curHPosY;"\
"timeCounter++;"\
"};"\
"function onError(evt) {"\
"console.log('Error: ' + evt.data);"\
"updateStatus('error: ' + evt.data);"\
"};"\
"function onClose(evt) {"\
"console.log('Connection closed');"\
"updateStatus('connection closed');"\
"};"\
"function postMessage() {"\
"msg = document.getElementById('msg').value;"\
"console.log('sending message: ' + msg);"\
"updateStatus('sending message: ' + msg);"\
"sensorWebSocket.send(msg);"\
"};"\
"function closeWS() {"\
"console.log('disconnecting ws');"\
"updateStatus('disconnecting');"\
"sensorWebSocket.close();"\
"};"\
"function updateStatus(msg) {"\
"document.getElementById('wsState').innerHTML = msg;"\
"};"\
"window.addEventListener('load',initGraph,false);"\
"window.addEventListener('load',initWebSocket,false);"\
"</script>"\
"</head>"\
"<body>"\
"<h1>温湿度监测演示系统</h1>"\
"<canvas id='graph' width='600' height='400'>"\
"The Canvas HTML5 element is not supported by your browser."\
"Please run this page in a different browser."\
"</canvas>"\
"<canvas id='graph2' width='600' height='400'>"\
"The Canvas HTML5 element is not supported by your browser."\
"Please run this page in a different browser."\
"</canvas>"\
"<div id='wsState'></div>"\
"<div id='messages'></div>"\
"<div style='margin:5px 5px;text-align:center'>"\
"&copy;Copyright 1998-2013 by WIZnet Team"\
"</div>"\
"</body>"\
"</html>";