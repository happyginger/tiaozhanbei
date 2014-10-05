
#include "httputil.h"
#include "webpage.h"

#include "device.h"

#include "ov2640.h"
#include "weibo.h"

//#define DEBUG_HTTP
#define   PACKET_LEN  1024*4

extern CONFIG_MSG  ConfigMsg;
extern char tx_buf[MAX_URI_SIZE];
extern char rx_buf[MAX_URI_SIZE];

extern uint8 reboot_flag;//defined in main

extern uint8 jpg_requested;
extern uint8 jpg_flag;
extern uint8 JPEGBuffer[];
extern uint32 JPEGCnt;

uint8 boundary[64];
//uint8 tmp_buf[1460]={0xff,};

static void make_basic_config_setting_json_callback(int8* buf, CONFIG_MSG config_msg)
{
  sprintf(buf,"settingsCallback({\"ver\":\"%d.%d\",\
                \"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\
                \"ip\":\"%d.%d.%d.%d\",\
                \"gw\":\"%d.%d.%d.%d\",\
                \"sub\":\"%d.%d.%d.%d\",\
                });",config_msg.sw_ver[0],config_msg.sw_ver[1],
                config_msg.mac[0],config_msg.mac[1],config_msg.mac[2],config_msg.mac[3],config_msg.mac[4],config_msg.mac[5],
                config_msg.lip[0],config_msg.lip[1],config_msg.lip[2],config_msg.lip[3],
                config_msg.gw[0],config_msg.gw[1],config_msg.gw[2],config_msg.gw[3],
                config_msg.sub[0],config_msg.sub[1],config_msg.sub[2],config_msg.sub[3]
                );
}
static void makeUpdateCamCallback(int8* buf)
{
  sprintf(buf,"updateCamCallback(%s);","'camera.jpg'");
}
static void makeDoUpdateCamCallback(int8* buf,uint8 val)
{
  sprintf(buf, "doUpdateCallback({\"camera\":\"%d\"});",val);
}
static void makePostWeiboCallback(int8* buf,uint8 val)
{
  sprintf(buf, "postWeiboCallback({\"post\":\"%d\"});",val);
}

//processing http protocol , and excuting the followed fuction.
void do_http(void)
{
  uint8 ch=SOCK_HTTP;
  uint16 len;
  st_http_request *http_request;
  memset(rx_buf,0x00,MAX_URI_SIZE);
  http_request = (st_http_request*)rx_buf;		// struct of http request
  
  /* http service start */
  switch(getSn_SR(ch))
  {
    case SOCK_INIT:
      listen(ch);
      break;
    case SOCK_LISTEN:

      break;
    case SOCK_ESTABLISHED:
    //case SOCK_CLOSE_WAIT:
      if(getSn_IR(ch) & Sn_IR_CON)
      {
        setSn_IR(ch, Sn_IR_CON);
      }
      if ((len = getSn_RX_RSR(ch)) > 0)		
      {
        len = recv(ch, (uint8*)http_request, len); 
        *(((uint8*)http_request)+len) = 0;
        proc_http(ch, (uint8*)http_request); // request is processed
        disconnect(ch);
      }
      break;
    case SOCK_CLOSE_WAIT:   
      if ((len = getSn_RX_RSR(ch)) > 0)
      {
        //printf("close wait: %d\r\n",len);
        len = recv(ch, (uint8*)http_request, len);       
        *(((uint8*)http_request)+len) = 0;
        proc_http(ch, (uint8*)http_request); // request is processed
      }
      disconnect(ch);
      break;
    case SOCK_CLOSED:                   
      socket(ch, Sn_MR_TCP, 80, 0x20);    /* reinitialize the socket */
      break;
    default:
    break;
  }// end of switch
}

void proc_http(SOCKET s, uint8 * buf)
{

  int8* name; //get method request file name
  int8 req_name[32]={0x00,}; //post method request file name
  unsigned long file_len=0;
  uint16 send_len=0;
  uint8* http_response;
  st_http_request *http_request;
  memset(tx_buf,0x00,MAX_URI_SIZE);
  http_response = (uint8*)rx_buf;
  http_request = (st_http_request*)tx_buf;
  parse_http_request(http_request, buf);    // After analyze request, convert into http_request
  //method Analyze
  switch (http_request->METHOD)		
  {
    case METHOD_ERR :
      if(strlen((int8 const*)boundary)>0)
      {
        printf("Error=%s\r\n",http_request->URI);
      }
      else
      {
        memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
        send(s, (uint8 *)http_response, strlen((int8 const*)http_response));
      }
      break;
    case METHOD_HEAD:
    case METHOD_GET:
      //get file name from uri
      name = http_request->URI;

      if(strcmp(name,"/index.htm")==0 || strcmp(name,"/")==0 || (strcmp(name,"/index.html")==0))
      {
        file_len = strlen(INDEX_HTML);
        make_http_response_head((uint8*)http_response, PTYPE_HTML,file_len);
        send(s,http_response,strlen((char const*)http_response));
        send_len=0;
        while(file_len)
        {
          if(file_len>1024)
          {
            if(getSn_SR(s)!=SOCK_ESTABLISHED)
            {
              return;
            }
            send(s, (uint8 *)INDEX_HTML+send_len, 1024);
            send_len+=1024;
            file_len-=1024;
          }
          else
          {
            send(s, (uint8 *)INDEX_HTML+send_len, file_len);
            send_len+=file_len;
            file_len-=file_len;
          } 
        }
      }
      else if(strcmp(name,"/w5500.js")==0)
      {
        memset(tx_buf,0,MAX_URI_SIZE);
        make_basic_config_setting_json_callback(tx_buf,ConfigMsg);
        sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
        send(s, (u_char *)http_response, strlen((char const*)http_response));
      }
      /*camera */
      else if((strcmp(name,"/camera.jpg")==0) || (strcmp(name,"/test.jpg")==0))
      {
        
        //start camera capture: enable camera interrupt
        
        while(jpg_flag!=4);
        file_len=JPEGCnt;
        make_http_response_head((unsigned char*)http_response, PTYPE_JPEG, (uint32)file_len);
        send(SOCK_HTTP, (uint8 const *)http_response, strlen((char const*)http_response));      
        send_len=0;
        while(file_len)
        { 
          if(file_len>PACKET_LEN)
          {
            send(SOCK_HTTP, (uint8*)(JPEGBuffer+send_len), PACKET_LEN);
            send_len+=PACKET_LEN;
            file_len-=PACKET_LEN;
          }
          else
          {
            send(SOCK_HTTP, (uint8*)(JPEGBuffer+send_len), file_len);
            send_len+=file_len;
            file_len-=file_len;
          }
        }
        //if(strcmp(name,"/test.jpg")==0)ov2640_interrupt_disable();
        
        jpg_flag=0;
        
        printf(".\r\n");
        
        
        
      }
      else if(strcmp(name,"/camera.pl")==0)
      {
        memset(tx_buf,0,MAX_URI_SIZE);
        makeUpdateCamCallback(tx_buf);
        sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
        send(s, (u_char *)http_response, strlen((char const*)http_response));
      }
      break;
      /*POST method*/
    case METHOD_POST:

      //get file name from uri
      mid(http_request->URI, "/", " ", req_name);

      if(strcmp(req_name,"config.cgi")==0)
      {
        cgi_ipconfig(http_request);
        make_cgi_response(5,(int8*)ConfigMsg.lip,tx_buf);         
        sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
        send(s, (u_char *)http_response, strlen((char *)http_response));		
        disconnect(s);

        reboot_flag=1;
        return;
      }
      /*camera*/
      else if(strcmp(req_name,"camera.cgi")==0)
      {
        uint8* param;
        uint8 val=0;
        if(param=get_http_param_value(http_request->URI, "camera"))
        {
          val=(uint8)ATOI((int8*)param,10);
          //1: camera run//0: camera stop
        }
        if(val==0)//camera stop, then disable camera interrupt to save current picture
        {
          while(jpg_flag!=4);//wait to receive the last picture
          //ov2640_interrupt_disable();
          //jpg_flag=0;
        }else{
          jpg_flag=0;
          //ov2640_interrupt_enable();
        }
        memset(tx_buf,0,MAX_URI_SIZE);//response
        makeDoUpdateCamCallback(tx_buf,val);
        sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
        send(s, (u_char *)http_response, strlen((char const*)http_response));
      }
      else if(strcmp(req_name,"postweibo.cgi")==0)
      {
        uint8* param;
        uint8 weiboId[32]={0x00,};
        uint8 weiboPwd[32]={0x00,};
        uint8 weiboContent[512]={0x00,};
        uint8 postWeiboRet=0;
        if(param=get_http_param_value(http_request->URI, "id")){strncpy((int8*)weiboId,(int8*)param,strlen((int8*)param)); printf("id:%s\r\n",weiboId);}
        if(param=get_http_param_value(http_request->URI, "pwd")){strncpy((int8*)weiboPwd,(int8*)param,strlen((int8*)param)); printf("pwd:%s\r\n",weiboPwd);}
        if(param=get_http_param_value(http_request->URI, "weibo")){strncpy((int8*)weiboContent,(int8*)param,strlen((int8*)param)); printf("weibo:%s\r\n",weiboContent);}
        if(strlen((int8*)weiboId)==0){strncpy((int8*) weiboId,"kotra010@gmail.com",18);strncpy((int8*)weiboPwd,"wiznet2010",10);}
        
        //normalSocketBuf();
        postWeiboRet=post_weibo_upload((char*)weiboId,(char*)weiboPwd,(char*)weiboContent,(char*)JPEGBuffer,JPEGCnt);
        //maxHttpTxBuf16();
        memset(tx_buf,0,MAX_URI_SIZE);//response
        makePostWeiboCallback(tx_buf,postWeiboRet);
        sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
        send(s, (u_char *)http_response, strlen((char const*)http_response));
      }
      //end of camera//
      else if(strcmp(req_name,"firmware.cgi")==0)
      {
#ifdef DEBUG_HTTP
        printf("firmware upload\r\n");
#endif
        mid((int8*)http_request->URI,"boundary=", "\r\n", (int8*)boundary);//boundary=
#ifdef DEBUG_HTTP
        printf("boundary=%s\r\n",boundary);
#endif
        uint16 tmp_len=0;
        uint32 content_len=0;
        uint32 rx_len=0;
        uint32 upload_file_len=0; //the file length
        int8 sub[10];
        //get Content-Length  
        mid((char*)http_request->URI,"Content-Length: ","\r\n",sub);
        content_len=ATOI32(sub,10);
#ifdef DEBUG_HTTP
        printf("content_len=%d\r\n",content_len);
#endif
        //flash operation vars
        uint32 flash_dest = AppBackAddress;
     
        uint16 fw_offset = 0; //the first packet fw data offset
        uint16 wr_len = 0; //the real write length of recevied packet
        
        uint8 remain_buf[3]={0xff,};
        uint8 remain_len = 0; //remain bytes length
        
        while(rx_len!=content_len)
        {

          tmp_len=getSn_RX_RSR(s);
            
          if(tmp_len>0)
          {
            if(tmp_len>1460) tmp_len=1460;
            tmp_len=recv(s, (uint8*)rx_buf, tmp_len);
            //--boundary\r\n
            //header\r\n\r\n
            //data......
            //\r\n--boundary--\r\n
            
            //rx_len==0 means it is the first packet after http header, 
            //the header of http file upload is included, we can get the real file size from this packet
            
            if(rx_len==0)//the first packet with header
            {
#ifdef DEBUG_HTTP
              printf("http request=%d\r\n",strlen((int8*)rx_buf));
#endif
              int8* pos1=strstr((int8*)rx_buf,(int8*)boundary);
              //pos1+=strlen((int8*)boundary)+2;
#ifdef DEBUG_HTTP
              printf("pos1=%d, %s\r\n", strlen(pos1),pos1);
#endif
              int8* pos2=strstr((int8*)rx_buf,"\r\n\r\n");
              uint16 hdr_len=strlen((int8*)boundary)+6+2+(pos2-pos1)+4+2;
              upload_file_len=content_len-hdr_len;
#ifdef DEBUG_HTTP
              printf("pos2=%d,boundary len=%d, upload file len=%d\r\n",pos2-pos1,strlen((int8*)boundary),upload_file_len);
#endif
              if(ConfigMsg.debug) printf("\r\n>");
              //write fw file to applicaiton backup flash
              fw_offset = pos2-pos1+4+2; //fw data start position in the first packet
              //unlock flash
              FLASH_Unlock();
              FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
              //erase required page
              //uint32 n_pages = FLASH_PagesMask(upload_file_len);
              for(uint32 n_erased = 0; n_erased < 23; n_erased++)
              {
                FLASH_ErasePage(AppBackAddress + 0x400 * n_erased);
              }
              
              wr_len = tmp_len-fw_offset;
              //make a buf with lengh of multiful 4 and save remain bytes
              remain_len = wr_len % 4;
              memcpy(/*tmp_buf*/BUFPUB,&rx_buf[fw_offset],wr_len-remain_len);
              wr_len = wr_len - remain_len;//real length
              if(remain_len!=0)
              {
                memcpy(remain_buf,&rx_buf[tmp_len-remain_len],remain_len);
              }
              
            }else{
              if(rx_len+tmp_len==content_len)//the last packet that includes boundary
              {
                wr_len = tmp_len - strlen((int8*)boundary) - 8;
              }else{
                wr_len = tmp_len;
              }
              fw_offset=0;
              //make buffer
              if(remain_len)
              {
                memcpy(/*tmp_buf*/BUFPUB,remain_buf,remain_len);
              }
              if(wr_len+remain_len>1460)
              {
                memcpy(&BUFPUB[remain_len],rx_buf,wr_len-remain_len);
                //save remain bytes
                memcpy(remain_buf,&rx_buf[tmp_len-remain_len],remain_len);
              }else{
                memcpy(&BUFPUB[remain_len],rx_buf, wr_len);  
                memset(&BUFPUB[wr_len+remain_len],0xff,3);
                wr_len=wr_len+remain_len;
              }
            }
            
            //program
            for(uint16 i = 0; i<wr_len; i+=4)
            {
              FLASH_ProgramWord(flash_dest, *(uint32*)((uint32)(BUFPUB + i)));
              flash_dest += 4;
            }
            
            rx_len+=tmp_len;
#ifdef DEBUG_HTTP
            printf("len=%d, rx len=%d, wr len=%d, @%08x\r\n",tmp_len, rx_len, wr_len, flash_dest);
#endif
            if(ConfigMsg.debug) printf(".");
            //program over
            if(rx_len==content_len)
            {
              //lock flash again
              FLASH_Lock();
              ConfigMsg.state = 1;//NEW_APP_IN_BACK;
              ConfigMsg.fw_len = upload_file_len;

              ////write_config_to_eeprom();
#ifdef DEBUG_HTTP
              printf("fw uploaded via http\r\n");
#endif
              if(ConfigMsg.debug) printf("\r\nThe firmware is updated. Rebooting...\r\n");
              reboot_flag = 1;
              make_cgi_response(6,(int8*)ConfigMsg.lip,tx_buf);         
              sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
              send(s, (uint8 *)http_response, strlen((char *)http_response));
            }
            
          }
        }
      }
      break;
    default :
      break;
  }
}


void cgi_ipconfig(st_http_request *http_request)
{
  
  uint8 * param;
  //Device setting
  if((param = get_http_param_value(http_request->URI,"ip")))
  {
    inet_addr_((uint8*)param, ConfigMsg.lip);	
  }
  if((param = get_http_param_value(http_request->URI,"gw")))
  {
    inet_addr_((uint8*)param, ConfigMsg.gw);	
  }
  if((param = get_http_param_value(http_request->URI,"sub")))
  {
    inet_addr_((uint8*)param, ConfigMsg.sub);		
  }
  /* Program the network parameters received into eeprom */
  ////write_config_to_eeprom();
}

void trimp(uint8* src, uint8* dst, uint16 len)
{
  uint16 i;
  for(i=0;i<len;i++)
  {
    if(*(src+i)!=0x00) 
      *(dst+i)=*(src+i);
    else 
      *(dst+i)=0x20;
  }
}
uint16 make_msg_response(uint8* buf,int8* msg)
{
  uint16 len=strlen(msg);
  sprintf((char *)buf,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",len,msg);
  return len;
}
void make_cgi_response(uint16 delay, int8* url,int8* cgi_response_buf)
{
  sprintf(cgi_response_buf,"<html><head><title>iWeb - Configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>please wait for a while, the module will boot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,url[0],url[1],url[2],url[3]);
  return;
}

void make_pwd_response(int8 isRight,uint16 delay,int8* cgi_response_content, int8 isTimeout)
{
  if (isRight==1)
    sprintf(cgi_response_content,"<html><head><title>Password - iWeb</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='config.html';}</script></head><body onload='func()'>Please wait for a while, the configuration page will be loaded automatically in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
  else
  {
    if (isTimeout==0)
      sprintf(cgi_response_content,"<html><head><title>Password - iWeb</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='index.html';}</script></head><body onload='func()'>The password you input is incorrect, please check it again. </br> This page will go back in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
  }
  if(isTimeout==1)
    sprintf(cgi_response_content,"<html><head><title>Password - iWeb</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j+' ';j--;setTimeout('func()',1000);if(j==0)location.href='index.html';}</script></head><body onload='func()'>You have NOT do any operation more than 5 minutes, please input your password again. </br> This page will go back in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay);
}