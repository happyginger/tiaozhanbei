
#include <stdio.h>
#include <string.h>


#include "w5500/w5500.h"
#include "w5500/socket.h"
#include "sha1.h"
#include "base64.h"
#include "util.h"



#define WS_SERVER_PORT  1818
uint8 wsTxBuf[2048]={0x00,};
uint8 wsRxBuf[2048]={0x00,};

uint8 handshaked=0;
//camera
#define   WS_PACKET_LEN  1024*4
extern uint8 jpg_flag;
extern uint32 JPEGCnt;
extern uint8 JPEGBuffer[];
extern const char LOGO_PNG[];
#define LOGO_PNG_LEN 2189
//end of camera

void calc_accept_key(char* s, char* r)
{
  const char* magicKey="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  char sInput[64]={0x00,};
  char tmpBuf[32]={0x00,};
  strcpy(sInput,s);
  strcpy(sInput+strlen(s),magicKey);
  printf("input: %s\r\n",sInput);
  SHA1Context sha;
  SHA1Reset(&sha);
  SHA1Input(&sha, (const unsigned char *)sInput, strlen(sInput));
  if (!SHA1Result(&sha)){
    printf("ERROR-- could not compute message digest\n");
  }else{
    for(unsigned char i = 0; i < 5 ; i++){
      tmpBuf[i*4+0]=*((char*)&sha.Message_Digest[i]+3);
      tmpBuf[i*4+1]=*((char*)&sha.Message_Digest[i]+2);
      tmpBuf[i*4+2]=*((char*)&sha.Message_Digest[i]+1);
      tmpBuf[i*4+3]=*((char*)&sha.Message_Digest[i]+0);
    }
    for(unsigned char i=0;i<20;i++)
      printf("%02X", tmpBuf[i]);
    printf("\r\n");
    base64encode(tmpBuf,r,20);
  }
}

void do_websocket_server(SOCKET s)
{
  uint16 len=0;
  switch(getSn_SR(s))
  {
    case SOCK_INIT:
      listen(s);
      break;
    case SOCK_LISTEN:

      break;
    case SOCK_ESTABLISHED:
    //case SOCK_CLOSE_WAIT:
      if(getSn_IR(s) & Sn_IR_CON)
      {
        setSn_IR(s, Sn_IR_CON);
        handshaked=0;
      }
      ////////////camera///////////
        if(handshaked)
        {
          
          uint32 jpgLen=0;
          uint32 send_len=0;
          
          uint8 firstByte=0x82;//FIN=1, opcode=0x02: binary
          uint8 secondByte=126;//no mask, extend length=2 bytes
          uint8 extend[2]={0x00};//extend header
          while(jpg_flag!=4);//wait for receiving one picture data
          if(JPEGBuffer[4]!=0xff || JPEGBuffer[5]!=0xd8)printf("4: %x, 5: %x\r\n",JPEGBuffer[4],JPEGBuffer[5]);
          jpgLen=JPEGCnt;
          extend[0]=(jpgLen-4)/256;
          extend[1]=(jpgLen-4)%256;
          JPEGBuffer[0]=firstByte;
          JPEGBuffer[1]=secondByte;
          JPEGBuffer[2]=extend[0];
          JPEGBuffer[3]=extend[1];
          while(jpgLen)
          {
            if(jpgLen>WS_PACKET_LEN)
            {
              send(s, (uint8*)(JPEGBuffer+send_len), WS_PACKET_LEN);
              send_len+=WS_PACKET_LEN;
              jpgLen-=WS_PACKET_LEN;
              if(jpg_flag==4)jpg_flag=0;
            }
            else
            {
              if(jpgLen==0) printf("jpgLen=0\r\n");
              send(s, (uint8*)(JPEGBuffer+send_len), jpgLen);
              send_len+=jpgLen;
              jpgLen-=jpgLen;
            }
          }
          
          /*
          jpgLen=LOGO_PNG_LEN;
          while(jpgLen)
          {
            if(jpgLen>WS_PACKET_LEN)
            {
              send(s, (uint8*)(LOGO_PNG+send_len), WS_PACKET_LEN);
              send_len+=WS_PACKET_LEN;
              jpgLen-=WS_PACKET_LEN;
              
            }
            else
            {
              if(jpgLen==0) printf("jpgLen=0\r\n");
              send(s, (uint8*)(LOGO_PNG+send_len), jpgLen);
              send_len+=jpgLen;
              jpgLen-=jpgLen;
            }
          }
          */
          printf(".%d\r\n",send_len);
          
        }
        ///////////////////end of camera///
      if ((len = getSn_RX_RSR(s)) > 0)		
      {
        memset(wsRxBuf,0x00,sizeof(wsRxBuf));
        len = recv(s, wsRxBuf, len); 
        if(!handshaked)
        {
          char sec_ws_key[32]={0x00,};
          char accept_key[32]={0x00,};
          
          printf("WS Rx: %s\r\n",wsRxBuf);
          //get Sec-WebSocket-Key:
          if(strstr((char const*)wsRxBuf,"Sec-WebSocket-Key: "))
          {
            mid((char*)wsRxBuf,"Sec-WebSocket-Key: ","\r\n",sec_ws_key);
            printf("Sec-WebSocket-Key: %s\r\n",sec_ws_key);
            calc_accept_key(sec_ws_key,accept_key);
            sprintf((char*)wsTxBuf,"HTTP/1.1 101 Switching Protocols\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",accept_key);
            send(s,wsTxBuf,strlen((char*)wsTxBuf));
          }
          handshaked=1;
        }else{
          if(len<2){
            printf("Dataframe header invalid!\r\n");
            return;
          }
          //the first byte
          uint8 fin = (wsRxBuf[0] & 0x80) == 0x80;
          /*
          uint8 rsv1 = (wsRxBuf[0] & 0x40) == 0x40;
          uint8 rsv2 = (wsRxBuf[0] & 0x20) == 0x20;
          uint8 rsv3 = (wsRxBuf[0] & 0x10) == 0x10;
          */
          uint8 opcode = wsRxBuf[0] & 0x0f;
          //the second byte
          uint8 hasmask = (wsRxBuf[1] & 0x80) == 0x80;
          uint8 payloadlength = wsRxBuf[1] & 0x7f;
          uint8 extendlength=0;
          uint8 extend2[2];
          uint8 extend8[8];
          uint8 maskcode[4];
          uint8 masklength=0;
          uint32 contentlength=0;
          uint8* content;
          if(payloadlength==126){
            memcpy(extend2,wsRxBuf+2,2);
            extendlength=2;
          }else if(payloadlength==127){
            memcpy(extend8,wsRxBuf+2,8);
            extendlength=8;
          }
            
          if(hasmask){
            memcpy(maskcode,wsRxBuf+extendlength+2,4);
            masklength=4;
          }
          //content
          if(extendlength==0){
            contentlength=payloadlength;
          }else if(extendlength==2){
            contentlength=extend2[0]*256+extend2[1];
            if(contentlength>1024*100) contentlength=1024*100;
          }
          else
          {
            int32 n=1;
            for (char i = 7; i >= 0; i--)
            {
              contentlength += extend8[i] * n;
              n *= 256;
            }
          }
          content=wsRxBuf+extendlength+masklength+2;
          if(hasmask){
            for(uint32 i=0;i<contentlength;i++){
              content[i]=(uint8)(content[i]^maskcode[i%4]);
            }
            printf("Rx: %s\r\n",content);
           
          }
        }
        
      }
      
      
      break;
    case SOCK_CLOSE_WAIT:   
      if ((len = getSn_RX_RSR(s)) > 0)
      {
        
      }
      disconnect(s);
      break;
    case SOCK_CLOSED:                   
      socket(s, Sn_MR_TCP, WS_SERVER_PORT, 0x20);    /* reinitialize the socket */
      break;
    default:
    break;
  }// end of switch
}