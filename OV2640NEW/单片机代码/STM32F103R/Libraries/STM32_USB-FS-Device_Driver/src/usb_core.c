/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_core.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Standard protocol processing (USB v2.0)
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
////////////������Ϣ�������/////////////
//�������ö�ٲ���
#define USB_DEBUG0			1
//�����ϸͨ������
#define USB_DEBUG1			0
//��������ַ����������������
#define USB_DEBUG_STR_DESC	0
/////////////////////////////////////////

#define ValBit(VAR,Place)    (VAR & (1 << Place))
#define SetBit(VAR,Place)    (VAR |= (1 << Place))
#define ClrBit(VAR,Place)    (VAR &= ((1 << Place) ^ 255))

#ifdef STM32F10X_CL
 #define Send0LengthData()  {PCD_EP_Write (0, 0, 0) ; vSetEPTxStatus(EP_TX_VALID);}
#else
#define Send0LengthData() { _SetEPTxCount(ENDP0, 0); \
    vSetEPTxStatus(EP_TX_VALID); \
  }
#endif /* STM32F10X_CL */

#define vSetEPRxStatus(st) (SaveRState = st)
#define vSetEPTxStatus(st) (SaveTState = st)

#define USB_StatusIn() Send0LengthData()
#define USB_StatusOut() vSetEPRxStatus(EP_RX_VALID)

#define StatusInfo0 StatusInfo.bw.bb1 /* Reverse bb0 & bb1 */
#define StatusInfo1 StatusInfo.bw.bb0

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t_uint8_t StatusInfo;

bool Data_Mul_MaxPacketSize = FALSE;
/* Private function prototypes -----------------------------------------------*/
static void DataStageOut(void);
static void DataStageIn(void);
static void NoData_Setup0(void);
static void Data_Setup0(void);
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Standard_GetConfiguration.
* Description    : Return the current configuration variable address.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 1 , if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
uint8_t *Standard_GetConfiguration(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength =
      sizeof(pInformation->Current_Configuration);
    return 0;
  }
#if USB_DEBUG0		> 0
	Prints("-----------�����û��ص�����------------\r\n");
#endif
  pUser_Standard_Requests->User_GetConfiguration();
  return (uint8_t *)&pInformation->Current_Configuration;
}

/*******************************************************************************
* Function Name  : Standard_SetConfiguration.
* Description    : This routine is called to set the configuration value
*                  Then each class should configure device themself.
* Input          : None.
* Output         : None.
* Return         : Return USB_SUCCESS, if the request is performed.
*                  Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetConfiguration(void)
{

  if ((pInformation->USBwValue0 <=
      Device_Table.Total_Configuration) && (pInformation->USBwValue1 == 0)
      && (pInformation->USBwIndex == 0)) /*call Back usb spec 2.0*/
  {
    pInformation->Current_Configuration = pInformation->USBwValue0;
	#if USB_DEBUG0		> 0
		Prints("-------�����û��ص�����,ͨ����ֵbDeviceState = CONFIGURED--------\r\n");
	#endif
    pUser_Standard_Requests->User_SetConfiguration();
    return USB_SUCCESS;
  }
  else
  {
    return USB_UNSUPPORT;
  }
}

/*******************************************************************************
* Function Name  : Standard_GetInterface.
* Description    : Return the Alternate Setting of the current interface.
* Input          : Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is invalid when "Length" is 0.
*                  Return "Buffer" if the "Length" is not 0.
*******************************************************************************/
uint8_t *Standard_GetInterface(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength =
      sizeof(pInformation->Current_AlternateSetting);
    return 0;
  }
#if USB_DEBUG0		> 0
	Prints("-----------�����û��ص�����------------\r\n");
#endif
  pUser_Standard_Requests->User_GetInterface();
  return (uint8_t *)&pInformation->Current_AlternateSetting;
}

/*******************************************************************************
* Function Name  : Standard_SetInterface.
* Description    : This routine is called to set the interface.
*                  Then each class should configure the interface them self.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetInterface(void)
{
  RESULT Re;
  /*Test if the specified Interface and Alternate Setting are supported by
    the application Firmware*/
  Re = (*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, pInformation->USBwValue0);

  if (pInformation->Current_Configuration != 0)
  {
    if ((Re != USB_SUCCESS) || (pInformation->USBwIndex1 != 0)
        || (pInformation->USBwValue1 != 0))
    {
      return  USB_UNSUPPORT;
    }
    else if (Re == USB_SUCCESS)
    {
		#if USB_DEBUG0		> 0
			Prints("-----------�����û��ص�����------------\r\n");
		#endif
      pUser_Standard_Requests->User_SetInterface();
      pInformation->Current_Interface = pInformation->USBwIndex0;
      pInformation->Current_AlternateSetting = pInformation->USBwValue0;
      return USB_SUCCESS;
    }

  }

  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : Standard_GetStatus.
* Description    : Copy the device request data to "StatusInfo buffer".
* Input          : - Length - How many bytes are needed.
* Output         : None.
* Return         : Return 0, if the request is at end of data block,
*                  or is invalid when "Length" is 0.
*******************************************************************************/
uint8_t *Standard_GetStatus(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 2;
    return 0;
  }

  /* Reset Status Information */
  StatusInfo.w = 0;

  if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
  {
    /*Get Device Status */
    uint8_t Feature = pInformation->Current_Feature;

    /* Remote Wakeup enabled */
    if (ValBit(Feature, 5))
    {
      SetBit(StatusInfo0, 1);
    }
    else
    {
      ClrBit(StatusInfo0, 1);
    }      

    /* Bus-powered */
    if (ValBit(Feature, 6))
    {
      SetBit(StatusInfo0, 0);
    }
    else /* Self-powered */
    {
      ClrBit(StatusInfo0, 0);
    }
  }
  /*Interface Status*/
  else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
  {
    return (uint8_t *)&StatusInfo;
  }
  /*Get EndPoint Status*/
  else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
  {
    uint8_t Related_Endpoint;
    uint8_t wIndex0 = pInformation->USBwIndex0;

    Related_Endpoint = (wIndex0 & 0x0f);
    if (ValBit(wIndex0, 7))
    {
      /* IN endpoint */
      if (_GetTxStallStatus(Related_Endpoint))
      {
        SetBit(StatusInfo0, 0); /* IN Endpoint stalled */
      }
    }
    else
    {
      /* OUT endpoint */
      if (_GetRxStallStatus(Related_Endpoint))
      {
        SetBit(StatusInfo0, 0); /* OUT Endpoint stalled */
      }
    }

  }
  else
  {
    return NULL;
  }
#if USB_DEBUG0		> 0
	Prints("-----------�����û��ص�����------------\r\n");
#endif
  pUser_Standard_Requests->User_GetStatus();
  return (uint8_t *)&StatusInfo;
}

/*******************************************************************************
* Function Name  : Standard_ClearFeature.
* Description    : Clear or disable a specific feature.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_ClearFeature(void)
{
  uint32_t     Type_Rec = Type_Recipient;
  uint32_t     Status;


  if (Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT))
  {/*Device Clear Feature*/
    ClrBit(pInformation->Current_Feature, 5);
    return USB_SUCCESS;
  }
  else if (Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
  {/*EndPoint Clear Feature*/
    DEVICE* pDev;
    uint32_t Related_Endpoint;
    uint32_t wIndex0;
    uint32_t rEP;

    if ((pInformation->USBwValue != ENDPOINT_STALL)
        || (pInformation->USBwIndex1 != 0))
    {
      return USB_UNSUPPORT;
    }

    pDev = &Device_Table;
    wIndex0 = pInformation->USBwIndex0;
    rEP = wIndex0 & ~0x80;
    Related_Endpoint = ENDP0 + rEP;

    if (ValBit(pInformation->USBwIndex0, 7))
    {
      /*Get Status of endpoint & stall the request if the related_ENdpoint
      is Disabled*/
      Status = _GetEPTxStatus(Related_Endpoint);
    }
    else
    {
      Status = _GetEPRxStatus(Related_Endpoint);
    }

    if ((rEP >= pDev->Total_Endpoint) || (Status == 0)
        || (pInformation->Current_Configuration == 0))
    {
      return USB_UNSUPPORT;
    }


    if (wIndex0 & 0x80)
    {
      /* IN endpoint */
      if (_GetTxStallStatus(Related_Endpoint ))
      {
      #ifndef STM32F10X_CL
        ClearDTOG_TX(Related_Endpoint);
      #endif /* STM32F10X_CL */
        SetEPTxStatus(Related_Endpoint, EP_TX_VALID);
      }
    }
    else
    {
      /* OUT endpoint */
      if (_GetRxStallStatus(Related_Endpoint))
      {
        if (Related_Endpoint == ENDP0)
        {
          /* After clear the STALL, enable the default endpoint receiver */
          SetEPRxCount(Related_Endpoint, Device_Property.MaxPacketSize);
          _SetEPRxStatus(Related_Endpoint, EP_RX_VALID);
        }
        else
        {
        #ifndef STM32F10X_CL
          ClearDTOG_RX(Related_Endpoint);
        #endif /* STM32F10X_CL */
          _SetEPRxStatus(Related_Endpoint, EP_RX_VALID);
        }
      }
    }
	#if USB_DEBUG0		> 0
		Prints("-----------�����û��ص�����------------\r\n");
	#endif
    pUser_Standard_Requests->User_ClearFeature();
    return USB_SUCCESS;
  }

  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : Standard_SetEndPointFeature
* Description    : Set or enable a specific feature of EndPoint
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetEndPointFeature(void)
{
  uint32_t    wIndex0;
  uint32_t    Related_Endpoint;
  uint32_t    rEP;
  uint32_t    Status;

  wIndex0 = pInformation->USBwIndex0;
  rEP = wIndex0 & ~0x80;
  Related_Endpoint = ENDP0 + rEP;

  if (ValBit(pInformation->USBwIndex0, 7))
  {
    /* get Status of endpoint & stall the request if the related_ENdpoint
    is Disabled*/
    Status = _GetEPTxStatus(Related_Endpoint);
  }
  else
  {
    Status = _GetEPRxStatus(Related_Endpoint);
  }

  if (Related_Endpoint >= Device_Table.Total_Endpoint
      || pInformation->USBwValue != 0 || Status == 0
      || pInformation->Current_Configuration == 0)
  {
    return USB_UNSUPPORT;
  }
  else
  {
    if (wIndex0 & 0x80)
    {
      /* IN endpoint */
      _SetEPTxStatus(Related_Endpoint, EP_TX_STALL);
    }

    else
    {
      /* OUT endpoint */
      _SetEPRxStatus(Related_Endpoint, EP_RX_STALL);
    }
  }
#if USB_DEBUG0		> 0
	Prints("-----------�����û��ص�����------------\r\n");
#endif
  pUser_Standard_Requests->User_SetEndPointFeature();
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Standard_SetDeviceFeature.
* Description    : Set or enable a specific feature of Device.
* Input          : None.
* Output         : None.
* Return         : - Return USB_SUCCESS, if the request is performed.
*                  - Return USB_UNSUPPORT, if the request is invalid.
*******************************************************************************/
RESULT Standard_SetDeviceFeature(void)
{
  SetBit(pInformation->Current_Feature, 5);
#if USB_DEBUG0		> 0
	Prints("-----------�����û��ص�����------------\r\n");
#endif
  pUser_Standard_Requests->User_SetDeviceFeature();
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Standard_GetDescriptorData.
* Description    : Standard_GetDescriptorData is used for descriptors transfer.
*                : This routine is used for the descriptors resident in Flash
*                  or RAM
*                  pDesc can be in either Flash or RAM
*                  The purpose of this routine is to have a versatile way to
*                  response descriptors request. It allows user to generate
*                  certain descriptors with software or read descriptors from
*                  external storage part by part.
* Input          : - Length - Length of the data in this transfer.
*                  - pDesc - A pointer points to descriptor struct.
*                  The structure gives the initial address of the descriptor and
*                  its original size.
* Output         : None.
* Return         : Address of a part of the descriptor pointed by the Usb_
*                  wOffset The buffer pointed by this address contains at least
*                  Length bytes.
*******************************************************************************/
uint8_t *Standard_GetDescriptorData(uint16_t Length, ONE_DESCRIPTOR *pDesc)
{
  uint32_t  wOffset;

  wOffset = pInformation->Ctrl_Info.Usb_wOffset;
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = pDesc->Descriptor_Size - wOffset;
    return 0;
  }

  return pDesc->Descriptor + wOffset;
}

/*******************************************************************************
* Function Name  : DataStageOut.
* Description    : Data stage of a Control Write Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DataStageOut(void)
{
  ENDPOINT_INFO *pEPinfo = &pInformation->Ctrl_Info;
  uint32_t save_rLength;

  save_rLength = pEPinfo->Usb_rLength;	//�����յ����ݳ���

  	if (pEPinfo->CopyData && save_rLength)
  	{										//��������ݴ�Ҫ����
    	uint8_t *Buffer;
    	uint32_t Length;
	#if USB_DEBUG0 	> 0
		Prints("�豸����: ");
		PrintShortIntHex(save_rLength);
		Prints("������\r\n");
	#endif
    	Length = pEPinfo->PacketSize;		//�˵������
    	if (Length > save_rLength)			
    	{									//�˵�����ȴ��ڴ��������ݳ���
      		Length = save_rLength;			//���Խ�������,������Ϊʵ�ʴ����ճ���
    	}
											//���򱾴ν��յ�����һ������������
    	Buffer = (*pEPinfo->CopyData)(Length);	//���ؽ�����������ʼ��ַ
    	pEPinfo->Usb_rLength -= Length;		//ȫ����Ϣ�޸ģ������ճ��ȼ�ȥ�ѽ��ճ���
    	pEPinfo->Usb_rOffset += Length;		//ȫ����Ϣ�޸ģ�ƫ�Ƽ����ѽ��ճ���

  	#ifdef STM32F10X_CL  
    	PCD_EP_Read(ENDP0, Buffer, Length); 
  	#else  								//�����ɶ˵㻺�����������û�������
    	PMAToUserBufferCopy(Buffer, GetEPRxAddr(ENDP0), Length);
  	#endif  /* STM32F10X_CL */
	
	#if USB_DEBUG1	> 0
	{
		u16 i = 0;
		u8* temp_buf = Buffer;
		for(i = 0; i < Length; i ++)
		{
			PrintHex(*temp_buf ++);
			if(!((i+1)&0x0f))
			{								//һ�������ʾ16������
				Prints("\r\n");				
			}
		}
		if((i)&0x0f)
		{									//�ܸ�������16��������,����
			Prints("\r\n");					//�����Ѿ���forѭ���л���
		}
	}
	#endif
  	}

  	if (pEPinfo->Usb_rLength != 0)
  	{										//��������ճ��Ȳ�����0��ʹ�ܽ��ն˵��������
   	 	vSetEPRxStatus(EP_RX_VALID);/* re-enable for next data reception */
   	 	SetEPTxCount(ENDP0, 0);				//��һ�����̿��ܱ��IN״̬����(������û�д��������������ı����ݷ���)
    	vSetEPTxStatus(EP_TX_VALID);/* Expect the host to abort the data OUT stage */
  	}
  	/* Set the next State*/
  	if (pEPinfo->Usb_rLength >= pEPinfo->PacketSize)
  	{										//��������ճ��ȴ��ڵ��ڶ˵������,��������OUT���ݹ���
   	 #if USB_DEBUG0	> 0
		Prints("����������...\r\n");
		#endif
		pInformation->ControlState = OUT_DATA;
  	}
  	else
  	{										//��������ճ���С�ڶ˵������
    	if (pEPinfo->Usb_rLength > 0)
    	{									//��������Ҫ������������һ��OUT���ݹ���
      		pInformation->ControlState = LAST_OUT_DATA;
    	}
    	else if (pEPinfo->Usb_rLength == 0)
    	{									//����ȴ�����IN״̬����
		#if USB_DEBUG0	> 0
			Prints("���һ��Out���ݹ���,��������IN״̬����\r\n");
		#endif
      		pInformation->ControlState = WAIT_STATUS_IN;
      		USB_StatusIn();					//�ڶ˵㻺����׼��0�ֽڵ�����,
    	}									//����IN���ư�����ʱ��ȡ�����ݲ�ACK
  	}
}

/*******************************************************************************
* Function Name  : DataStageIn.
* Description    : Data stage of a Control Read Transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DataStageIn(void)
{
  ENDPOINT_INFO *pEPinfo = &pInformation->Ctrl_Info;
  uint32_t save_wLength = pEPinfo->Usb_wLength;		//���������ݳ���
  uint32_t ControlState = pInformation->ControlState;

  uint8_t *DataBuffer;
  uint32_t Length;
	
  	if ((save_wLength == 0) && (ControlState == LAST_IN_DATA))
  	{										//��������ͳ���Ϊ0��Ϊ���һ�η���(���һ�������豸������-18�ֽ�
    	if(Data_Mul_MaxPacketSize == TRUE)	//��ֻ�ᴥ��һ��IN���ݹ����жϣ�������OUT״̬����
    	{									//������������ݵĳ��ȸպ��ǰ����ȵ�������
      	/* No more data to send and empty packet */
		#if USB_DEBUG0	> 0
			Prints("Ϊ�˵�������������,����0��������(���δ������һ�η���)\r\n");
		#endif
			
      		Send0LengthData();				//����0��������
      		ControlState = LAST_IN_DATA;	//�޸�״̬Ϊ���һ�η���
      		Data_Mul_MaxPacketSize = FALSE;	//�ָ���־λFALSE
    	}
    	else 
   	 	{
  		/* No more data to send so STALL the TX Status*/
		#if USB_DEBUG0	> 0
			Prints("IN���ݹ������,��������Out״̬����\r\n");
		#endif
      		ControlState = WAIT_STATUS_OUT;	//�����Ѿ���������һ�η���
											//������OUT״̬����
    	#ifdef STM32F10X_CL      
      		PCD_EP_Read (ENDP0, 0, 0);
    	#endif  /* STM32F10X_CL */ 
    
    	#ifndef STM32F10X_CL 
      		vSetEPTxStatus(EP_TX_STALL);	//�ı䷢��״̬ΪSTALL
    	#endif  /* STM32F10X_CL */ 
    	}
	
    	goto Expect_Status_Out;				//��ת���������
  	}

  	Length = pEPinfo->PacketSize;   		//�˵�0������
  	ControlState = (save_wLength <= Length) ? LAST_IN_DATA : IN_DATA; //��������ȴ��ڻ���ڴ����ͳ��ȣ���Ϊ���һ�η���

  	if (Length > save_wLength)        		//��������ȴ��ڴ����͵ĳ��ȣ��򷵻�ʵ�ʴ����͵ĳ���
  	{
    	Length = save_wLength;
  	}
	
  	DataBuffer = (*pEPinfo->CopyData)(Length);	//��ʱLength��Ϊ0���������û������ͻ������ġ��ס���ַ
	
#if USB_DEBUG0	> 0
	Prints("�豸���ڷ�������->DataStageIn()...\r\n");
	Prints("���δ��䷵��: ");
	PrintShortIntHex(Length);
	Prints("������\r\n");
#endif
	
#if USB_DEBUG1	> 0
{
	u16 i = 0;
	u8* temp_buf = DataBuffer;
	for(i = 0; i < Length; i ++)
	{
		PrintHex(*temp_buf ++);
		if(!((i+1)&0x0f))
		{									//һ�������ʾ16������
			Prints("\r\n");				
		}
	}
	if((i)&0x0f)
	{										//�ܸ�������16��������,����
			Prints("\r\n");					//�����Ѿ���forѭ���л���
	}
		
}
#endif
											//�������û�������׼�����͵ĵ�һ��Ԫ�ص�ַ,��һ��
#ifdef STM32F10X_CL							//���û��������׵�ַ
  	PCD_EP_Write (ENDP0, DataBuffer, Length);
#else                                   	//���û����ݿ�����USB������
  	UserToPMABufferCopy(DataBuffer, GetEPTxAddr(ENDP0), Length);
#endif /* STM32F10X_CL */ 

	SetEPTxCount(ENDP0, Length);         	//���ö˵�0�ķ������ݴ�С
	
	pEPinfo->Usb_wLength -= Length;     	//ȫ����Ϣ�޸ģ������ͳ��ȼ�ȥ�Ѿ������˵ĳ���
	pEPinfo->Usb_wOffset += Length;			//ȫ����Ϣ�޸ģ�����ƫ�Ƽ����ѷ������˳���
	vSetEPTxStatus(EP_TX_VALID);			//����״̬ΪVALID
										
	USB_StatusOut();/* Expect the host to abort the data IN stage */

Expect_Status_Out:							//ȫ����Ϣ�޸ģ��ı�ö�ٽ׶�״̬������������״̬��
  	pInformation->ControlState = ControlState;//1.�����������ݹ��̣�2.OUT״̬����
}

/*******************************************************************************
* Function Name  : NoData_Setup0.
* Description    : Proceed the processing of setup request without data stage.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void NoData_Setup0(void)
{
	RESULT Result = USB_UNSUPPORT;
	uint32_t RequestNo = pInformation->USBbRequest;
	uint32_t ControlState;

  	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
  	{
    #if USB_DEBUG0	> 0
		Prints("��׼����->�豸");		
	#endif
		/* Device Request*/
    	/* SET_CONFIGURATION*/
    	if (RequestNo == SET_CONFIGURATION)
    	{												//ֻ�������������Ժ�������÷���˵�
		#if USB_DEBUG0	> 0
			Prints("->��������\r\n");		
		#endif
      		Result = Standard_SetConfiguration();			//ִ�����������bDeviceState = CONFIGURED;
    	}

    	/*SET ADDRESS*/
    	else if (RequestNo == SET_ADDRESS)
    	{
      		if ((pInformation->USBwValue0 > 127) || (pInformation->USBwValue1 != 0)
          	|| (pInformation->USBwIndex != 0)
          	|| (pInformation->Current_Configuration != 0))
       		/* Device Address should be 127 or less*/
      		{
        		ControlState = STALLED;
			#if USB_DEBUG0	> 0
				Prints("��ַ��Χ����,STALLED->��������\r\n");		
			#endif
        		goto exit_NoData_Setup0;
      		}
      		else
      		{
			#if USB_DEBUG0	> 0
				Prints("�������õ�ַ\r\n");		
			#endif
				Result = USB_SUCCESS;
	
	      	#ifdef STM32F10X_CL
	         	SetDeviceAddress(pInformation->USBwValue0);
	      	#endif  /* STM32F10X_CL */
      		}
		}
    	/*SET FEATURE for Device*/
    	else if (RequestNo == SET_FEATURE)
    	{
      		if ((pInformation->USBwValue0 == DEVICE_REMOTE_WAKEUP)
          		&& (pInformation->USBwIndex == 0)
          		&& (ValBit(pInformation->Current_Feature, 5)))
      		{
      		#if USB_DEBUG0	> 0
				Prints("����Feature\r\n");		
		  	#endif
				Result = Standard_SetDeviceFeature();
      		}
      		else
      		{
        		Result = USB_UNSUPPORT;
      		}
    	}
    	/*Clear FEATURE for Device */
    	else if (RequestNo == CLEAR_FEATURE)
    	{
      		if (pInformation->USBwValue0 == DEVICE_REMOTE_WAKEUP
          		&& pInformation->USBwIndex == 0
          		&& ValBit(pInformation->Current_Feature, 5))
      		{
      		#if USB_DEBUG0	> 0
				Prints("���Feature\r\n");		
		  	#endif
				Result = Standard_ClearFeature();
      		}
      		else
      		{
      		#if USB_DEBUG0	> 0
				Prints("->��֧�ֵ��豸��������\r\n");		
			#endif
				Result = USB_UNSUPPORT;
      		}
    	}

  	}

  	/* Interface Request*/
  	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
  	{
  	#if USB_DEBUG0	> 0
		Prints("��׼����->Interface");		
	#endif
		/*SET INTERFACE*/
    	if (RequestNo == SET_INTERFACE)
    	{
		#if USB_DEBUG0	> 0
			Prints("->����Interface\r\n");		
		#endif
			Result = Standard_SetInterface();
    	}
  	}

  	/* EndPoint Request*/
  	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
  	{
	#if USB_DEBUG0	> 0
		Prints("��׼����->EndPoint");		
	#endif
		/*CLEAR FEATURE for EndPoint*/
    	if (RequestNo == CLEAR_FEATURE)
    	{
    	#if USB_DEBUG0	> 0
			Prints("->���Feature\r\n");		
		#endif
			Result = Standard_ClearFeature();
    	}
    	/* SET FEATURE for EndPoint*/
    	else if (RequestNo == SET_FEATURE)
    	{
    	#if USB_DEBUG0	> 0
			Prints("->����EndPointFeature\r\n");		
		#endif
			Result = Standard_SetEndPointFeature();
    	}
  	}
  	else
  	{
  	#if USB_DEBUG0	> 0
		Prints("->��֧�ֵ�����\r\n");		
	#endif
		Result = USB_UNSUPPORT;
  	}


  	if (Result != USB_SUCCESS)
  	{										//����Ǳ�׼������������󣬵����û��ṩ�Ļص�����
	#if USB_DEBUG0	> 0
		Prints("��׼�������������, ------�����û��ṩ�Ļص�����------\r\n");		
	#endif    
		Result = (*pProperty->Class_NoData_Setup)(RequestNo);
    	if (Result == USB_NOT_READY)
    	{
			ControlState = PAUSE;
		#if USB_DEBUG0	> 0
			Prints("USB_NOT_READY,PAUSE->��������\r\n");		
		#endif
		
      	goto exit_NoData_Setup0;
   	 	}
  	}

  	if (Result != USB_SUCCESS)
  	{
		ControlState = STALLED;
	#if USB_DEBUG0	> 0
		Prints("��֧�ֵĿ��ƴ�������,STALLED->��������\r\n");		
	#endif
    goto exit_NoData_Setup0;
  	}
										//���������,�������ݹ��̵Ŀ��ƴ�����,SETUP�Ժ���IN״̬����
  	ControlState = WAIT_STATUS_IN;/* After no data stage SETUP */
	
  	USB_StatusIn();						//׼����0�ֽ������ڶ˵㻺����,��������һ��IN���ư�ʱ����,������ACK

exit_NoData_Setup0:
  	pInformation->ControlState = ControlState;
  	return;
}

/*******************************************************************************
* Function Name  : Data_Setup0.
* Description    : Proceed the processing of setup request with data stage.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Data_Setup0(void)
{
	uint8_t *(*CopyRoutine)(uint16_t);
	RESULT Result;
	uint32_t Request_No = pInformation->USBbRequest;
	
	uint32_t Related_Endpoint, Reserved;
	uint32_t wOffset, Status;
	
	
	
	CopyRoutine = NULL;
	wOffset = 0;

  	/*GET DESCRIPTOR*/
  	if (Request_No == GET_DESCRIPTOR)
  	{							//�������Ϊ:0x06 
	#if USB_DEBUG0	> 0
		Prints("��ȡ������(0x06)(1�豸;2����;3�ַ�;4�ӿ�;5�˵�):\r\n");		
	#endif
    	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
    	{						//��׼����(0x80)
			uint8_t wValue1 = pInformation->USBwValue1;
			
			#if USB_DEBUG0	> 0
				Prints("->�豸");
			#endif
      		if (wValue1 == DEVICE_DESCRIPTOR)
      		{						//��׼����->�豸������18���ֽ�
			#if USB_DEBUG0	> 0
				Prints("->�豸������(18�ֽ�)\r\n");
			#endif
			
        		CopyRoutine = pProperty->GetDeviceDescriptor;
      		}
      		else if (wValue1 == CONFIG_DESCRIPTOR)
      		{						//��׼����->����������9�ֽ�(�ӿ�������(9)�Ͷ˵�������(7)����������������һ�𷵻�)
      		#if USB_DEBUG0	> 0
				Prints("->����������(9�ֽ�+(�ӿ�)9*x +(�˵�)7*x)\r\n");
			#endif
			
				CopyRoutine = pProperty->GetConfigDescriptor;//(�����HID�豸,HID������Ҫ���ڽӿ�����������)
      		}
      		else if (wValue1 == STRING_DESCRIPTOR)
      		{						//��׼����->�ַ���������   (wValue0: 
      		#if USB_DEBUG0	> 0
				Prints("->�ַ���������\r\n");
			#endif
			#if USB_DEBUG_STR_DESC	> 0
				Prints("->0����ID,1�����ַ���,2��Ʒ�ַ���,3��Ʒ���к�--> ");
				PrintHex(pInformation->USBwValue0);
				Prints("\r\n");
			#endif
				CopyRoutine = pProperty->GetStringDescriptor;//0����ID,1�����ַ���,2��Ʒ�ַ���,3��Ʒ���к�)
			}  /* End of GET_DESCRIPTOR */
    	}
  	}

  	/*GET STATUS*/
  	else if ((Request_No == GET_STATUS) && (pInformation->USBwValue == 0)
           && (pInformation->USBwLength == 0x0002)
           && (pInformation->USBwIndex1 == 0))
  	{
    	/* GET STATUS for Device*/
    	if ((Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
       		&& (pInformation->USBwIndex == 0))
    	{
    	#if USB_DEBUG0	> 0
			Prints("��ȡ�豸(Device)Status\r\n");		
		#endif
			CopyRoutine = Standard_GetStatus;
    	}

    	/* GET STATUS for Interface*/
    	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
    	{												//�ص�����
     	 	if (((*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, 0) == USB_SUCCESS)
          	&& (pInformation->Current_Configuration != 0))
      		{
      		#if USB_DEBUG0	> 0
				Prints("��ȡ�ӿ�(Interface)Status\r\n");		
			#endif
				CopyRoutine = Standard_GetStatus;
      		}
   	 	}

    	/* GET STATUS for EndPoint*/
    	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
    	{
      		Related_Endpoint = (pInformation->USBwIndex0 & 0x0f);
      		Reserved = pInformation->USBwIndex0 & 0x70;
		#if USB_DEBUG0	> 0
				Prints("��ȡ�˵�(EndPoint)Status\r\n");		
		#endif
      		if (ValBit(pInformation->USBwIndex0, 7))
      		{
        		/*Get Status of endpoint & stall the request if the related_ENdpoint
        		is Disabled*/
        		Status = _GetEPTxStatus(Related_Endpoint);
      		}
      		else
      		{
        		Status = _GetEPRxStatus(Related_Endpoint);
      		}

      		if ((Related_Endpoint < Device_Table.Total_Endpoint) && (Reserved == 0)
          		&& (Status != 0))
      		{
        		CopyRoutine = Standard_GetStatus;
      		}
    	}
  	}

  	/*GET CONFIGURATION*/
	else if (Request_No == GET_CONFIGURATION)
  	{
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
    	{
		#if USB_DEBUG0	> 0
				Prints("��ȡ�豸����(Configuration)\r\n");		
		#endif
			CopyRoutine = Standard_GetConfiguration;
    	}
  	}
  	/*GET INTERFACE*/
  	else if (Request_No == GET_INTERFACE)
  	{
    	if ((Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
        	&& (pInformation->Current_Configuration != 0) && (pInformation->USBwValue == 0)
        	&& (pInformation->USBwIndex1 == 0) && (pInformation->USBwLength == 0x0001)
        	&& ((*pProperty->Class_Get_Interface_Setting)(pInformation->USBwIndex0, 0) == USB_SUCCESS))
    	{
    	#if USB_DEBUG0	> 0
				Prints("��ȡ�ӿ�(Interface)\r\n");		
		#endif
			CopyRoutine = Standard_GetInterface;
    	}

  	}
  
	if (CopyRoutine)
	{									//����Ǳ�׼����(�����ݹ���5���г���SYNCH_FRAMEδʵ��)
		pInformation->Ctrl_Info.Usb_wOffset = wOffset;
		pInformation->Ctrl_Info.CopyData = CopyRoutine;
		/* sb in the original the cast to word was directly */
		/* now the cast is made step by step */
		(*CopyRoutine)(0);				//����length  = 0,->��ȡ���������ݵĳ���,������Ctrl_Info.Usb_wLength��
		Result = USB_SUCCESS;			//����data_setp0�ɹ�
	}
  	else
  	{									//���Ϊ������������������û��ṩ�ص�����ʵ��
    #if USB_DEBUG0	> 0
		Prints("->�Ǳ�׼����\r\n");		
		Prints("-----���û��ṩ�ص�����ʵ��-----(�������������,�����籨������,HID������)\r\n");		
	#endif
		Result = (*pProperty->Class_Data_Setup)(pInformation->USBbRequest);
    	if (Result == USB_NOT_READY)
    	{								//���ݴ�����̻�δ��ɣ��˵㽫����NAK��Ӧ�Ժ���������
			pInformation->ControlState = PAUSE;
		#if USB_DEBUG0	> 0
			Prints("USB_NOT_READY,PAUSE->��������\r\n");
		#endif
      		return;
    	}
  	}	

  	if (pInformation->Ctrl_Info.Usb_wLength == 0xFFFF)
  	{									
	    /* Data is not ready, wait it */
	    pInformation->ControlState = PAUSE;
	#if USB_DEBUG0	> 0
		Prints("Data is not ready,PAUSE->��������\r\n");
	#endif
    	return;
  	}
  	if ((Result == USB_UNSUPPORT) || (pInformation->Ctrl_Info.Usb_wLength == 0))
  	{									//����ǲ�֧�ֵ���������ṩ���ݸ���������
    	/* Unsupported request */		//��ΪSTALLED
    	pInformation->ControlState = STALLED;
	#if USB_DEBUG0	> 0
		Prints("Unsupported request,STALLED->��������\r\n");
	#endif
    	return;
  	}


  	if (ValBit(pInformation->USBbmRequestType, 7))
  	{									//����������豸������,D7 = 1,�����ݵ�setup����
    	/* Device ==> Host */			//ͨ��Ӧ��Ϊ�������
    	__IO uint32_t wLength = pInformation->USBwLength;	//�������ݳ���
     
    	/* Restrict the data length to be the one host asks for */
    	if (pInformation->Ctrl_Info.Usb_wLength > wLength)	
    	{								//��������ͳ��ȴ������󳤶ȣ����Ϊʵ������ĳ���
      		pInformation->Ctrl_Info.Usb_wLength = wLength;
    	}
    
    	else if (pInformation->Ctrl_Info.Usb_wLength < pInformation->USBwLength)
    	{								//��������ͳ���С�����󳤶�
      		if (pInformation->Ctrl_Info.Usb_wLength < pProperty->MaxPacketSize)
      		{							//��������ͳ���С���������ȣ�������ͨ������0������������־������
        		Data_Mul_MaxPacketSize = FALSE;
      		}
      		else if ((pInformation->Ctrl_Info.Usb_wLength % pProperty->MaxPacketSize) == 0)
      		{							//��������ͳ����ܹ�������������������Ҫ����0������������־������
        		Data_Mul_MaxPacketSize = TRUE;
      		}
    	}   
										//������������︳ֵ�Ժ��û���ٸ��Ĺ�����������ֵ�����Ķ���������
    	pInformation->Ctrl_Info.PacketSize = pProperty->MaxPacketSize;
	#if USB_DEBUG0	> 0
		Prints("�豸����: ");
		PrintShortIntHex(pInformation->Ctrl_Info.Usb_wLength);
		Prints("������\r\n");
	#endif
    	DataStageIn();					//SETUP -> IN(����IN����)������������˵㻺�������ȴ��¸�IN���ư���
  	}									//��ʱ��ȡ������
  	else
  	{									//������ݷ���Ϊ�������豸,��ı���Ӧ��ͨ��״̬�Ͷ˿�״̬
	    pInformation->ControlState = OUT_DATA;
	    vSetEPRxStatus(EP_RX_VALID); /* enable for next data reception */
  	}
  	return;
}

/*******************************************************************************
* Function Name  : Setup0_Process
* Description    : Get the device request data and dispatch to individual process.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t Setup0_Process(void)
{

  	union
  	{
   	 	uint8_t* b;
   	 	uint16_t* w;
  	} pBuf;

#ifdef STM32F10X_CL
  	USB_OTG_EP *ep;
  	uint16_t offset = 0;
 
  	ep = PCD_GetOutEP(ENDP0);
  	pBuf.b = ep->xfer_buff;
#else  
  	uint16_t offset = 1;
  
  	pBuf.b = PMAAddr + (uint8_t *)(_GetEPRxAddr(ENDP0) * 2); /* *2 for 32 bits addr */
#endif /* STM32F10X_CL */

#if USB_DEBUG0		> 0
	Prints("\r\nSetup0 �ж�-->��������\r\n");
#endif		
							
  	if (pInformation->ControlState != PAUSE)
  	{									//��ȡ��׼����ĸ����ֶ�(8�ֽ�)
  	#if USB_DEBUG0		> 0
		Prints("�豸���Խ����µ�����\r\n");
	#endif
		pInformation->USBbmRequestType = *pBuf.b++; /* bmRequestType */
	    pInformation->USBbRequest = *pBuf.b++; /* bRequest */
	    pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
	    pInformation->USBwValue = ByteSwap(*pBuf.w++); /* wValue */
	    pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
	    pInformation->USBwIndex  = ByteSwap(*pBuf.w++); /* wIndex */
	    pBuf.w += offset;  /* word not accessed because of 32 bits addressing */
	    pInformation->USBwLength = *pBuf.w; /* wLength */
  	}

#if USB_DEBUG1		> 0
	Prints("�豸������������: \r\n");
	PrintHex(pInformation->USBbmRequestType);
	PrintHex(pInformation->USBbRequest);
	
	PrintHex(pInformation->USBwValue0);		//ʵ�ʵ��ֽ�(�����ߵ��ֽڶԵ�)
	PrintHex(pInformation->USBwValue1);		//ʵ�ʸ��ֽ�(�����ߵ��ֽڶԵ�)
	
	PrintHex(pInformation->USBwIndex0);		//ʵ�ʵ��ֽ�(�����ߵ��ֽڶԵ�)
	PrintHex(pInformation->USBwIndex1);		//ʵ�ʸ��ֽ�(�����ߵ��ֽڶԵ�)
	
	PrintHex(pInformation->USBwLength1);	//ʵ�ʵ��ֽ�
	PrintHex(pInformation->USBwLength0);	//ʵ�ʸ��ֽ�
	Prints("\r\n");
#endif	

  	pInformation->ControlState = SETTING_UP;
  	if (pInformation->USBwLength == 0)
  	{										//�����ݹ���(�����õ�ַ��,��һ��ΪIN״̬����)
	#if USB_DEBUG0 > 0
		Prints("�����ݵĽ�������\r\n");
	#endif
	
		/* Setup with no data stage */
		NoData_Setup0();
  	}
  	else
  	{										//�����ݹ���(��һ��Ϊ���ݹ���(OUT��IN))
	#if USB_DEBUG0 > 0
		Prints("�����ݵĽ�������: ");
		
		#if USB_DEBUG1 > 0
			PrintShortIntHex(pInformation->USBwLength);
		#endif
		Prints("\r\n");
	#endif
		/* Setup with data stage */
  		Data_Setup0();
  	}
  	return Post0_Process();
}

/*******************************************************************************
* Function Name  : In0_Process
* Description    : Process the IN token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t In0_Process(void)
{
  	uint32_t ControlState = pInformation->ControlState;
#if USB_DEBUG0 > 0
	Prints("IN0�ж�-->");
#endif
  	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA))
  	{									//������Ҫ���͵�����(�������������豸������,������д��˵㻺������,������IN���ư�����ʱ������ȡ������,��ACK��ʱ��״̬ΪLAST_IN_DATA)
	#if USB_DEBUG0 > 0
		Prints("In���ݹ���\r\n");
	#endif
		DataStageIn();					//����IN���ݹ���
    	/* ControlState may be changed outside the function */
    	ControlState = pInformation->ControlState;
  	}

  	else if (ControlState == WAIT_STATUS_IN)
  	{									//�ȴ�����IN״̬����(�������ݵĿ��ƴ�����뵽�����״̬SETUP->IN)
		#if USB_DEBUG0 > 0
			Prints("IN״̬����\r\n");
		#endif
		if ((pInformation->USBbRequest == SET_ADDRESS) &&
        (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)))
    	{								//�������յ��豸���ص�0�ֽ����ݺ�ظ�ACK(������жϷ�֧),��ʱ�豸�����µ��豸��ַ
		#if USB_DEBUG0 > 0
			Prints("�����豸��ַ: ");
			PrintHex(pInformation->USBwValue0);
			Prints("\r\n");
		#endif
			SetDeviceAddress(pInformation->USBwValue0);
      		pUser_Standard_Requests->User_SetDeviceAddress();
    	}	
		
    	(*pProperty->Process_Status_IN)();	//�����û������״̬IN�ص�����
    	ControlState = STALLED;				//����
  	}

  	else
  	{
    	ControlState = STALLED;			
  	}

  	pInformation->ControlState = ControlState;

  	return Post0_Process();
}

/*******************************************************************************
* Function Name  : Out0_Process
* Description    : Process the OUT token on all default endpoint.
* Input          : None.
* Output         : None.
* Return         : Post0_Process.
*******************************************************************************/
uint8_t Out0_Process(void)
{
	uint32_t ControlState = pInformation->ControlState;
#if USB_DEBUG0 > 0
	Prints("Out0�ж�-->");
#endif
  	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA))
	{									//������ֹ�˿��ƴ������
    	/* host aborts the transfer before finish */
    	ControlState = STALLED;
  	}
  	else if ((ControlState == OUT_DATA) || (ControlState == LAST_OUT_DATA))
  	{
	#if USB_DEBUG0 > 0
		Prints("Out���ݹ���\r\n");
	#endif
		DataStageOut();
    	ControlState = pInformation->ControlState; /* may be changed outside the function */
  	}

  	else if (ControlState == WAIT_STATUS_OUT)
  	{
	#if USB_DEBUG0 > 0
		Prints("Out״̬����\r\n");
	#endif
		(*pProperty->Process_Status_OUT)();	//�����û������״̬OUT�ص�����
  	#ifndef STM32F10X_CL
    	ControlState = STALLED;
  	#endif /* STM32F10X_CL */
  	}


  	/* Unexpect state, STALL the endpoint */
  	else
  	{
	#if USB_DEBUG0 > 0
		Prints("Unexpect state,-----��������-----\r\n");
	#endif
		ControlState = STALLED;
  	}

  	pInformation->ControlState = ControlState;

  	return Post0_Process();
}

/*******************************************************************************
* Function Name  : Post0_Process
* Description    : Stall the Endpoint 0 in case of error.
* Input          : None.
* Output         : None.
* Return         : - 0 if the control State is in PAUSE
*                  - 1 if not.
*******************************************************************************/
uint8_t Post0_Process(void)
{
#ifdef STM32F10X_CL  
  	USB_OTG_EP *ep;
#endif /* STM32F10X_CL */
      								//���ö˵�0���ջ����С
  	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);

  	if (pInformation->ControlState == STALLED)
  	{									//���״̬Ϊ�����������ö˵���Ӧ��״̬
    	vSetEPRxStatus(EP_RX_STALL);
    	vSetEPTxStatus(EP_TX_STALL);
  	}

#ifdef STM32F10X_CL
  	else if ((pInformation->ControlState == OUT_DATA) ||
      (pInformation->ControlState == WAIT_STATUS_OUT))
  	{
	    ep = PCD_GetInEP(0);
	    ep->is_in = 0;
	    OTGD_FS_EP0StartXfer(ep);
	    
	    vSetEPTxStatus(EP_TX_VALID);
  	}
  
  	else if ((pInformation->ControlState == IN_DATA) || 
      (pInformation->ControlState == WAIT_STATUS_IN))
	{
	    ep = PCD_GetInEP(0);
	    ep->is_in = 1;
	    OTGD_FS_EP0StartXfer(ep);    
  	}
#endif /* STM32F10X_CL */

  	return (pInformation->ControlState == PAUSE);
}

/*******************************************************************************
* Function Name  : SetDeviceAddress.
* Description    : Set the device and all the used Endpoints addresses.
* Input          : - Val: device adress.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetDeviceAddress(uint8_t Val)
{
#ifdef STM32F10X_CL 
  PCD_EP_SetAddress ((uint8_t)Val);
#else 
  uint32_t i;
  uint32_t nEP = Device_Table.Total_Endpoint;

  /* set address in every used endpoint */
  for (i = 0; i < nEP; i++)
  {
    _SetEPAddress((uint8_t)i, (uint8_t)i);
  } /* for */
  _SetDADDR(Val | DADDR_EF); /* set device address and enable function */
#endif  /* STM32F10X_CL */  
}

/*******************************************************************************
* Function Name  : NOP_Process
* Description    : No operation function.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void NOP_Process(void)
{
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
