/*****************************************************************************
**                      Copyright  (C)  WCH  2001-2025                      **
**                      Web:  http://wch.cn                                 **
******************************************************************************/
// USB���߽ӿ�оƬCH341/7����Ӧ�ò�ӿڿ�,CH347/9����480Mbps����USB������չUART/SPI/I2C/JTAG/SWD
// CH346����480Mbps����USB������չUART/SPI SLAVE,UART/PARALLEL SLAVE
// CH347-DLL  V1.4
// ���л���: Windows 98/ME, Windows 2000/XP, WIN7/8/10/11,and later.
// support USB chip: CH341, CH341A,CH347,CH339W
// USB => Parallel, I2C, SPI, JTAG, SWD, PARALLEL,UART ...
//Notes:
//Copyright (C) 2025 Nanjing Qinheng Microelectronics Co., Ltd.



#ifndef		_CH347_DLL_H
#define		_CH347_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN64
#define		mOFFSET( s, m )			( (ULONG_PTR) & ( ( ( s * ) 0 ) -> m ) )	// �����ȡ�ṹ��Ա���ƫ�Ƶ�ַ�ĺ�
#else
#define		mOFFSET( s, m )			( (ULONG) & ( ( ( s * ) 0 ) -> m ) )	// �����ȡ�ṹ��Ա���ƫ�Ƶ�ַ�ĺ�
#endif

#ifndef		max
#define		max( a, b )				( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )	// �ϴ�ֵ
#endif

#ifndef		min
#define		min( a, b )				( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )	// ��Сֵ
#endif

#ifdef		ExAllocatePool
#undef		ExAllocatePool						// ɾ����TAG���ڴ����
#endif

#ifndef		NTSTATUS
typedef		LONG	NTSTATUS;					// ����״̬
#endif

//��CH31DLL����CH341WDM����
#ifndef _CH341_DLL_H
typedef	struct	_USB_SETUP_PKT {				// USB���ƴ���Ľ����׶ε�����������ṹ
	UCHAR			mUspReqType;				// 00H ��������
	UCHAR			mUspRequest;				// 01H �������
	union	{
		struct	{
			UCHAR	mUspValueLow;				// 02H ֵ�������ֽ�
			UCHAR	mUspValueHigh;				// 03H ֵ�������ֽ�
		};
		USHORT		mUspValue;					// 02H-03H ֵ����
	};
	union	{
		struct	{
			UCHAR	mUspIndexLow;				// 04H �����������ֽ�
			UCHAR	mUspIndexHigh;				// 05H �����������ֽ�
		};
		USHORT		mUspIndex;					// 04H-05H ��������
	};
	USHORT			mLength;					// 06H-07H ���ݽ׶ε����ݳ���
} mUSB_SETUP_PKT, *mPUSB_SETUP_PKT;


typedef	struct	_WIN32_COMMAND {				// ����WIN32����ӿڽṹ
	union	{
		ULONG		mFunction;					// ����ʱָ�����ܴ�����߹ܵ���
		NTSTATUS	mStatus;					// ���ʱ���ز���״̬
	};
	ULONG			mLength;					// ��ȡ����,���غ������ݵĳ���
	union	{
		mUSB_SETUP_PKT	mSetupPkt;				// USB���ƴ���Ľ����׶ε���������
		UCHAR			mBuffer[ 512];      	// ���ݻ�����,����Ϊ0��255B
	};
} mWIN32_COMMAND, *mPWIN32_COMMAND;
// WIN32Ӧ�ò�ӿ�����
#define		IOCTL_CH341_COMMAND		( FILE_DEVICE_UNKNOWN << 16 | FILE_ANY_ACCESS << 14 | 0x0f34 << 2 | METHOD_BUFFERED )	// ר�ýӿ�

#define		mWIN32_COMMAND_HEAD		mOFFSET( mWIN32_COMMAND, mBuffer )	// WIN32����ӿڵ�ͷ����

#define		mCH341_MAX_NUMBER		32			// ���ͬʱ���ӵ�CH341/7�豸����

#define		mMAX_BUFFER_LENGTH		0x1000		// ���ݻ�������󳤶�4096

#define		mMAX_COMMAND_LENGTH		( mWIN32_COMMAND_HEAD + mMAX_BUFFER_LENGTH )	// ������ݳ��ȼ�������ṹͷ�ĳ���

#define		mDEFAULT_BUFFER_LEN		0x0400		// ���ݻ�����Ĭ�ϳ���1024

#define		mDEFAULT_COMMAND_LEN	( mWIN32_COMMAND_HEAD + mDEFAULT_BUFFER_LEN )	// Ĭ�����ݳ��ȼ�������ṹͷ�ĳ���

// CH341�˵��ַ
#define		mCH347_ENDP_DATA_UP		0x86		// CH347�����ݿ��ϴ��˵�ĵ�ַ
#define		mCH347_ENDP_DATA_DOWN	0x06		// CH347�����ݿ��´��˵�ĵ�ַ

// �豸��ӿ��ṩ�Ĺܵ���������
#define		mPipeDeviceCtrl			0x00000004	// CH347���ۺϿ��ƹܵ�
#define		mPipeDataUp				0x00000006	// CH347�����ݿ��ϴ��ܵ�
#define		mPipeDataDown			0x00000007	// CH347�����ݿ��´��ܵ�

// Ӧ�ò�ӿڵĹ��ܴ���
#define		mFuncNoOperation		0x00000000	// �޲���
#define		mFuncGetVersion			0x00000001	// ��ȡ��������汾��
#define		mFuncGetConfig			0x00000002	// ��ȡUSB�豸����������
#define		mFuncSetTimeout			0x00000009	// ����USBͨѶ��ʱ
#define		mFuncSetExclusive		0x0000000b	// ���ö�ռʹ��
#define		mFuncResetDevice		0x0000000c	// ��λUSB�豸
#define		mFuncResetPipe			0x0000000d	// ��λUSB�ܵ�
#define		mFuncAbortPipe			0x0000000e	// ȡ��USB�ܵ�����������
#define		mFuncBufferMode			0x00000020	// �趨�����ϴ�ģʽ����ѯ�������е����ݳ���
#define		mFuncBufferModeDn		0x00000021	// �趨�����´�ģʽ����ѯ�������е����ݳ���
#define		mFuncGetVersionEx		0x00000022	// ��ȡ��������汾�ż�оƬ�ͺ�
// USB�豸��׼�������
#define		mUSB_CLR_FEATURE		0x01
#define		mUSB_SET_FEATURE		0x03
#define		mUSB_GET_STATUS			0x00
#define		mUSB_SET_ADDRESS		0x05
#define		mUSB_GET_DESCR			0x06
#define		mUSB_SET_DESCR			0x07
#define		mUSB_GET_CONFIG			0x08
#define		mUSB_SET_CONFIG			0x09
#define		mUSB_GET_INTERF			0x0a
#define		mUSB_SET_INTERF			0x0b
#define		mUSB_SYNC_FRAME			0x0c

// CH341���ƴ���ĳ���ר����������
#define		mCH341_VENDOR_READ		0xC0		// ͨ�����ƴ���ʵ�ֵ�CH341����ר�ö�����
#define		mCH341_VENDOR_WRITE		0x40		// ͨ�����ƴ���ʵ�ֵ�CH341����ר��д����

#define		mCH341A_CMD_I2C_STREAM	0xAA		// I2C�ӿڵ������,�Ӵ��ֽڿ�ʼΪI2C������
#define		mCH341A_CMD_UIO_STREAM	0xAB		// UIO�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������
#define		mCH341A_CMD_PIO_STREAM	0xAE		// PIO�ӿڵ������,�Ӵ��ֽڿ�ʼΪ������
// CH341A���ƴ���ĳ���ר���������
#define		mCH341A_BUF_CLEAR		0xB2		// ���δ��ɵ�����
#define		mCH341A_I2C_CMD_X		0x54		// ����I2C�ӿڵ�����,����ִ��
#define		mCH341A_DELAY_MS		0x5E		// ������Ϊ��λ��ʱָ��ʱ��
#define		mCH341A_GET_VER			0x5F		// ��ȡоƬ�汾

#define		mCH341A_CMD_I2C_STM_STA	0x74		// I2C�ӿڵ�������:������ʼλ
#define		mCH341A_CMD_I2C_STM_STO	0x75		// I2C�ӿڵ�������:����ֹͣλ
#define		mCH341A_CMD_I2C_STM_OUT	0x80		// I2C�ӿڵ�������:�������,λ5-λ0Ϊ����,�����ֽ�Ϊ����,0������ֻ����һ���ֽڲ�����Ӧ��
#define		mCH341A_CMD_I2C_STM_IN	0xC0		// I2C�ӿڵ�������:��������,λ5-λ0Ϊ����,0������ֻ����һ���ֽڲ�������Ӧ��
#define		mCH341A_CMD_I2C_STM_MAX	( min( 0x3F, mCH341_PACKET_LENGTH ) )	// I2C�ӿڵ�������������������������ݵ���󳤶�
#define		mCH341A_CMD_I2C_STM_SET	0x60		// I2C�ӿڵ�������:���ò���,λ2=SPI��I/O��(0=���뵥��,1=˫��˫��),λ1λ0=I2C�ٶ�(00=����,01=��׼,10=����,11=����)
#define		mCH341A_CMD_I2C_STM_US	0x40		// I2C�ӿڵ�������:��΢��Ϊ��λ��ʱ,λ3-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_I2C_STM_MS	0x50		// I2C�ӿڵ�������:������Ϊ��λ��ʱ,λ3-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_I2C_STM_DLY	0x0F		// I2C�ӿڵ�����������������ʱ�����ֵ
#define		mCH341A_CMD_I2C_STM_END	0x00		// I2C�ӿڵ�������:�������ǰ����

#define		mCH341A_CMD_UIO_STM_IN	0x00		// UIO�ӿڵ�������:��������D7-D0
#define		mCH341A_CMD_UIO_STM_DIR	0x40		// UIO�ӿڵ�������:�趨I/O����D5-D0,λ5-λ0Ϊ��������
#define		mCH341A_CMD_UIO_STM_OUT	0x80		// UIO�ӿڵ�������:�������D5-D0,λ5-λ0Ϊ����
#define		mCH341A_CMD_UIO_STM_US	0xC0		// UIO�ӿڵ�������:��΢��Ϊ��λ��ʱ,λ5-λ0Ϊ��ʱֵ
#define		mCH341A_CMD_UIO_STM_END	0x20		// UIO�ӿڵ�������:�������ǰ����

#define		MAX_DEVICE_PATH_SIZE	128			// �豸���Ƶ�����ַ���
#define		MAX_DEVICE_ID_SIZE		64			// �豸ID������ַ���
#endif 

// �����ӿ�
#define CH347_USB_VENDOR   0
#define CH347_USB_HID      2
#define CH347_USB_VCP      3

// CH347_USB_VENDOR֧��CH341/CH347T/CH347F/CH339W
#define CHIP_TYPE_CH341  0
#define CHIP_TYPE_CH347  1
#define CHIP_TYPE_CH347F 2
#define CHIP_TYPE_CH339W 3
#define CHIP_TYPE_CH347T CHIP_TYPE_CH347

// оƬ���ܽӿ�����
#define CH347_FUNC_UART          0
#define CH347_FUNC_SPI_IIC       1
#define CH347_FUNC_JTAG_IIC      2
#define CH347_FUNC_JTAG_IIC_SPI  3		// CH347Fͬʱ֧��SPI\I2C\JTAG�ӿ�

#define DEFAULT_READ_TIMEOUT	500		// Ĭ�϶���ʱ������
#define DEFAULT_WRITE_TIMEOUT	500		// Ĭ��д��ʱ������

#define	mCH347_PACKET_LENGTH	512		// CH347֧�ֵ����ݰ��ĳ���

#pragma pack(1) 

//SPI����������
typedef struct _SPI_CONFIG{
	UCHAR           iMode;                 // 0-3:SPI Mode0/1/2/3
	UCHAR           iClock;                // 0=60MHz, 1=30MHz, 2=15MHz, 3=7.5MHz, 4=3.75MHz, 5=1.875MHz, 6=937.5KHz��7=468.75KHz
	UCHAR			iByteOrder;            // 0=��λ��ǰ(LSB), 1=��λ��ǰ(MSB)
	USHORT          iSpiWriteReadInterval; // SPI�ӿڳ����ȡд�����������λΪuS
	UCHAR           iSpiOutDefaultData;    // SPI������ʱĬ���������
	ULONG			iChipSelect;           // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1�������Ч: λ1λ0Ϊ00/01�ֱ�ѡ��CS1/CS2������Ϊ�͵�ƽ��ЧƬѡ
	UCHAR           CS1Polarity;           // λ0��ƬѡCS1���Կ��ƣ�0���͵�ƽ��Ч��1���ߵ�ƽ��Ч��
	UCHAR           CS2Polarity;           // λ0��ƬѡCS2���Կ��ƣ�0���͵�ƽ��Ч��1���ߵ�ƽ��Ч��
	USHORT          iIsAutoDeativeCS;      // ������ɺ��Ƿ��Զ�����Ƭѡ
	USHORT          iActiveDelay;          // ����Ƭѡ��ִ�ж�д��������ʱʱ��,��λus
	ULONG           iDelayDeactive;        // ����Ƭѡ��ִ�ж�д��������ʱʱ��,��λus
}mSpiCfgS,*mPSpiCfgS;

//�豸��Ϣ
typedef struct _DEV_INFOR{
	UCHAR    iIndex;                 // ��ǰ�����
	UCHAR    DevicePath[MAX_PATH];   // �豸������,����CreateFile
	UCHAR    UsbClass;               // ������� 0:CH347_USB_CH341, 2:CH347_USB_HID, 3:CH347_USB_VCP
	UCHAR    FuncType;               // ������� 0:CH347_FUNC_UART, 1:CH347_FUNC_SPI_I2C, 2:CH347_FUNC_JTAG_I2C, 3:CH347_FUNC_JTAG_IIC_SPI
	CHAR     DeviceID[64];           // USB\VID_xxxx&PID_xxxx
	UCHAR    ChipMode;               // оƬ����ģʽ,0:Mode0(UART0/1); 1:Mode1(Uart1+SPI+I2C); 2:Mode2(HID Uart1+SPI+I2C) 3:Mode3(Uart1+Jtag) 4:CH347F(Uart*2+Jtag/SPI/IIC)
	HANDLE   DevHandle;              // �豸���
	USHORT   BulkOutEndpMaxSize;     // �����ϴ��˵��С
	USHORT   BulkInEndpMaxSize;      // �����´��˵��С
	UCHAR    UsbSpeedType;           // USB�ٶ����ͣ�0:FS,1:HS,2:SS
	UCHAR    CH347IfNum;             // USB�ӿں�: CH347T: IF0:UART;   IF1:SPI/IIC/JTAG/GPIO
									 //            CH347F: IF0:UART0;  IF1:UART1; IF 2:SPI/IIC/JTAG/GPIO
	UCHAR    DataUpEndp;             // �����ϴ��˵��ַ
	UCHAR    DataDnEndp;             // �����´��˵��ַ
	CHAR     ProductString[64];      // USB��Ʒ�ַ���
	CHAR     ManufacturerString[64]; // USB�����ַ���
	ULONG    WriteTimeout;           // USBд��ʱ
	ULONG    ReadTimeout;            // USB����ʱ
	CHAR     FuncDescStr[64];        // �ӿڹ���������
	UCHAR    FirewareVer;            // �̼��汾,ʮ������ֵ
}mDeviceInforS,*mPDeviceInforS;

#pragma pack() 

// CH347��ģʽ���ú���,֧��CH347����ģʽ�µĴ򿪡��رա�USB����USBд������HID
//��USB�豸
HANDLE WINAPI CH347OpenDevice(ULONG DevI);	// ָ���豸���

//�ر�USB�豸
BOOL WINAPI CH347CloseDevice(ULONG iIndex);	// ָ���豸���

// ��ȡ�豸USB���к�
BOOL WINAPI CH347GetSerialNumber(ULONG iIndex,				// ָ���豸���
								 PUCHAR iSerialNumberStr);	// ָ���ȡ�����豸���к�

// ��ȡ�豸��Ϣ
BOOL WINAPI CH347GetDeviceInfor(ULONG iIndex,					// ָ���豸���
								mDeviceInforS *DevInformation);	// ָ���ȡ�����豸��Ϣ

// ��ȡCH347оƬ����:0:CHIP_TYPE_CH341��1:CHIP_TYPE_CH347/CHIP_TYPE_CH347T,2:CHIP_TYPE_CH347F��3:CHIP_TYPE_CH339W
UCHAR	WINAPI	CH347GetChipType(ULONG			iIndex );  // ָ���豸���

// ��ȡ�����汾����汾���豸�汾��оƬ����(CH341(FS)/CH347HS)
BOOL	WINAPI	CH347GetVersion(ULONG 			 iIndex, 
							    PUCHAR           iDriverVer,
								PUCHAR           iDLLVer,
							    PUCHAR           ibcdDevice,
							    PUCHAR           iChipType);  //CHIP_TYPE_CH341/7

typedef		VOID	( CALLBACK	* mPCH347_NOTIFY_ROUTINE ) (								 // �豸���֪ͨ�¼��ص�����
															ULONG			iEventStatus );  // �豸����¼��͵�ǰ״̬(�����ж���): 0=�豸�γ��¼�, 3=�豸�����¼�

#define		CH347_DEVICE_ARRIVAL		3		// �豸�����¼�,�Ѿ�����
#define		CH347_DEVICE_REMOVE_PEND	1		// �豸��Ҫ�γ�
#define		CH347_DEVICE_REMOVE			0		// �豸�γ��¼�,�Ѿ��γ�

// �趨�豸�¼�֪ͨ����
BOOL	WINAPI	CH347SetDeviceNotify(ULONG					iIndex,				// ָ���豸���,0��Ӧ��һ���豸
									 PCHAR					iDeviceID,			// ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
									 mPCH347_NOTIFY_ROUTINE	iNotifyRoutine );   // ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���

// ��ȡUSB���ݿ�
BOOL	WINAPI	CH347ReadData( ULONG			iIndex,          // ָ���豸���
							   PVOID			oBuffer,         // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
							   PULONG			ioLength );		 // ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

// дȡUSB���ݿ�
BOOL	WINAPI	CH347WriteData(ULONG			iIndex,			// ָ���豸���
							   PVOID			iBuffer,		// ָ��һ��������,����׼��д��������
							   PULONG			ioLength );		// ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���

// ����USB���ݶ�д�ĳ�ʱ
BOOL	WINAPI	CH347SetTimeout(ULONG			iIndex,         // ָ���豸���
							    ULONG			iWriteTimeout,  // ָ��USBд�����ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
								ULONG			iReadTimeout ); // ָ��USB��ȡ���ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)

/***************SPI********************/
// SPI��������ʼ��	
BOOL	WINAPI	CH347SPI_Init(ULONG iIndex,			// ָ���豸���
							  mSpiCfgS *SpiCfg);	// ָ��SPI���ýṹ��

// ����SPIʱ��Ƶ�ʣ����øýӿں������µ���CH347SPI_Init���г�ʼ��
BOOL WINAPI CH347SPI_SetFrequency(ULONG iIndex,		 // ָ���豸���
								  ULONG iSpiSpeedHz);// ����SPIʱ�ӣ���λΪHZ

// ����SPI����λ��
BOOL WINAPI CH347SPI_SetDataBits(ULONG iIndex,		// ָ���豸���
								 UCHAR iDataBits);	// 0=8bit��1=16bit

// ��ȡSPI������������Ϣ
BOOL    WINAPI  CH347SPI_GetCfg(ULONG iIndex,mSpiCfgS *SpiCfg);

// ����Ƭѡ״̬,ʹ��ǰ���ȵ���CH347SPI_Init��CS��������
BOOL	WINAPI	CH347SPI_ChangeCS(ULONG			iIndex,         // ָ���豸���	
								  UCHAR         iStatus);       // 0=����Ƭѡ,1=����Ƭѡ

// ����SPIƬѡ
BOOL	WINAPI	CH347SPI_SetChipSelect(ULONG			iIndex,            // ָ���豸���
									   USHORT           iEnableSelect,     // �Ͱ�λΪCS1���߰�λΪCS2; �ֽ�ֵΪ1=����CS,Ϊ0=���Դ�CS����
									   USHORT           iChipSelect,       // �Ͱ�λΪCS1���߰�λΪCS2;Ƭѡ���,0=����Ƭѡ,1=����Ƭѡ
									   ULONG            iIsAutoDeativeCS,  // ��16λΪCS1����16λΪCS2;������ɺ��Ƿ��Զ�����Ƭѡ
									   ULONG            iActiveDelay,      // ��16λΪCS1����16λΪCS2;����Ƭѡ��ִ�ж�д��������ʱʱ��,��λus
									   ULONG            iDelayDeactive);   // ��16λΪCS1����16λΪCS2;����Ƭѡ��ִ�ж�д��������ʱʱ��,��λus

// SPI4д����
BOOL	WINAPI	CH347SPI_Write(ULONG			iIndex,          // ָ���豸���	
							   ULONG			iChipSelect,     // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1����Ƭѡ����
							   ULONG			iLength,         // ׼������������ֽ���	
							   ULONG			iWriteStep,      // ׼����ȡ�ĵ�����ĳ���
							   PVOID			ioBuffer);       // ָ��һ��������,����׼����MOSIд��������

// SPI4������.������д���ݣ�Ч�ʽ�CH347SPI_WriteRead�ߺܶ�
BOOL	WINAPI	CH347SPI_Read(ULONG			iIndex,           // ָ���豸���	
							  ULONG			iChipSelect,      // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1����Ƭѡ����
							  ULONG         oLength,          // ׼���������ֽ���
							  PULONG		iLength,          // ׼������������ֽ���	
							  PVOID			ioBuffer);        // ָ��һ��������,����׼����DOUTд��������,���غ��Ǵ�DIN���������

// ����SPI������,4�߽ӿ�
BOOL	WINAPI	CH347SPI_WriteRead(ULONG			iIndex,       // ָ���豸���
								   ULONG			iChipSelect,  // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1�����Ƭѡ
								   ULONG			iLength,      // ׼������������ֽ���
								   PVOID			ioBuffer );   // ָ��һ��������,����׼����DOUTд��������,���غ��Ǵ�DIN���������

// ����SPI������,4�߽ӿ�
BOOL	WINAPI	CH347StreamSPI4(ULONG			iIndex,       // ָ���豸���
								ULONG			iChipSelect,  // Ƭѡ����, λ7Ϊ0�����Ƭѡ����, λ7Ϊ1�������Ч
								ULONG			iLength,      // ׼������������ֽ���
								PVOID			ioBuffer );   // ָ��һ��������,����׼����DOUTд��������,���غ��Ǵ�DIN���������

/***************JTAG********************/
// JTAG�ӿڳ�ʼ��������JTAGͨ���ٶ�
BOOL	WINAPI	CH347Jtag_INIT(ULONG iIndex,		// ָ���豸���
							   UCHAR iClockRate);   // ͨ���ٶȣ�0=468.75KHz,1=937.5KHz,2=1.875MHz,3=3.75MHz,4=7.5MHz,5=15MHz,6=30MHz,7=60MHz

// ��ȡJtag�ٶ�����
BOOL    WINAPI  CH347Jtag_GetCfg(ULONG iIndex,         // ָ���豸���
							     UCHAR *ClockRate);    // ͨ���ٶȣ���ЧֵΪ0-7��ֵԽ��ͨ���ٶ�Խ��


// �ı�TMS��ֵ������״̬�л�
BOOL	WINAPI CH347Jtag_TmsChange(ULONG  iIndex,   // �豸���
								   PUCHAR tmsValue, // �����л���TMSλֵ,���ֽ�Ϊ��λ
								   ULONG  Step,     // tmsValue�ڴ洢��TMS��Чλ��
								   ULONG  Skip);    // ��Ч��ʼλ

// ��Shift-DR/IR״̬���ж�д��ִ��������Exit DR/IR
// ״̬��:Shift-DR/IR.RW.->Exit DR/IR
BOOL   WINAPI  CH347Jtag_IoScan(ULONG   iIndex,    
							    PUCHAR  DataBits,   //��Ҫ���д��������λ
							    ULONG   DataBitsNb, //��Ҫ�������ݵ�λ��
							    BOOL    IsRead);    //�Ƿ���Ҫ��ȡ����

// ����Shift-DR/IR״̬���ж�д,ִ����ɺ�,�������һ�������л�״̬��Exit DR/IR;�������,��ͣ��Shift-DR/IR״̬
// ״̬��:Shift-DR/IR.RW..->[Exit DR/IR]
BOOL   WINAPI  CH347Jtag_IoScanT(ULONG  iIndex,       // ָ���豸���
								 PUCHAR DataBits,     // ��Ҫ���д��������λ
								 ULONG  DataBitsNb,   // ��Ҫ�������ݵ�λ��
								 BOOL   IsRead,       // �Ƿ���Ҫ��ȡ����
								 BOOL   IsLastPkt);   // �Ƿ�Ϊ���һ��

// JTAG��λTap״̬����.������������TCK��TMSΪ�߽��ɽ�״̬����ΪTest-Logic Reset״̬
ULONG	WINAPI  CH347Jtag_Reset(ULONG iIndex);	// ָ���豸���

// ����TRST���Ӳ����λ
BOOL WINAPI CH347Jtag_ResetTrst(ULONG iIndex,	// ָ���豸���
								BOOL  iLevel);	// 0=����Ϊ�ͣ�1=����Ϊ��

// λ����ʽJTAG IR/DR���ݶ�д.�������������ݵĶ�д����ָ�������״̬���л��ȿ����ഫ�䡣���������ݴ��䣬����ʹ��CH347Jtag_WriteRead_Fast
// �������4096�ֽ�Ϊ��λ������д
// ״̬��:Run-Test->Shift-IR/DR..->Exit IR/DR -> Run-Test
BOOL	WINAPI	CH347Jtag_WriteRead(ULONG			iIndex,           // ָ���豸���
									BOOL            IsDR,             // =TRUE: DR���ݶ�д,=FALSE:IR���ݶ�д
									ULONG			iWriteBitLength,  // д����,׼��д���ĳ���
									PVOID			iWriteBitBuffer,  // ָ��һ��������,����׼��д��������	
									PULONG			oReadBitLength,   // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ�ĳ���
									PVOID			oReadBitBuffer ); // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// JTAG IR/DR����������д,���ڶ��ֽ�������д����JTAG�̼����ز�������Ӳ����4K������������д��������Ȳ�����4096�ֽڡ���������С�����е���
// ״̬��:Run-Test->Shift-IR/DR..->Exit IR/DR -> Run-Test
BOOL	WINAPI	CH347Jtag_WriteRead_Fast(ULONG		iIndex,            // ָ���豸���
										 BOOL       IsDR,              // =TRUE: DR���ݶ�д,=FALSE:IR���ݶ�д
										 ULONG		iWriteBitLength,   // д����,׼��д���ĳ���
										 PVOID		iWriteBitBuffer,   // ָ��һ��������,����׼��д��������	
										 PULONG		oReadBitLength,    // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ�ĳ���
										 PVOID		oReadBitBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// λ����ʽJTAG IR/DR���ݶ�д.�������������ݵĶ�д����ָ�������״̬���л��ȿ����ഫ�䡣���������ݴ��䣬����ʹ��CH347Jtag_WriteRead_Fast
// �������4096�ֽ�Ϊ��λ������д
// ״̬��:Run-Test-> Shift-IR/DR..->Exit IR/DR -> Run-Test
BOOL	WINAPI	CH347Jtag_WriteReadEx(ULONG			iIndex,           // ָ���豸���
									  BOOL			IsInDrOrIr,	      // =TRUE: ��SHIFT-DR/IR״̬�������ݽ���  ==FALSE: Run-Test->Shift-IR/DR.�������ݽ���.->Exit IR/DR -> Run-Test
									  BOOL          IsDR,             // =TRUE: DR���ݶ�д,=FALSE:IR���ݶ�д
									  ULONG			iWriteBitLength,  // д����,׼��д���ĳ���
									  PVOID			iWriteBitBuffer,  // ָ��һ��������,����׼��д��������	
									  PULONG		oReadBitLength,   // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ�ĳ���
									  PVOID			oReadBitBuffer ); // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// JTAG IR/DR����������д,���ڶ��ֽ�������д����JTAG�̼����ز�������Ӳ����4K������������д��������Ȳ�����4096�ֽڡ���������С�����е���
// ״̬��:Run-Test->Shift-IR/DR..->Exit IR/DR -> Run-Test
BOOL	WINAPI	CH347Jtag_WriteRead_FastEx(ULONG		iIndex,            // ָ���豸���
										   BOOL		    IsInDrOrIr,	       // =TRUE: ��SHIFT-DR/IR״̬�������ݽ���  ==FALSE: Run-Test->Shift-IR/DR.�������ݽ���.->Exit IR/DR -> Run-Test								
										   BOOL         IsDR,              // =TRUE: DR���ݶ�д,=FALSE:IR���ݶ�д
										   ULONG		iWriteBitLength,   // д����,׼��д���ĳ���
										   PVOID		iWriteBitBuffer,   // ָ��һ��������,����׼��д��������	
										   PULONG		oReadBitLength,    // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ�ĳ���
										   PVOID		oReadBitBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// �����л�JTAG״̬�����谴��˳��ִ��
BOOL WINAPI CH347Jtag_SwitchTapState(UCHAR TapState);		// ָ���л�����״̬
// 0:Test-Logic Reset,1:Run-Test/Idle��2:Run-Test/Idle -> Shift-DR,3:Shift-DR -> Run-Test/Idle
// 4:Run-Test/Idle -> Shift-IR, 5:Shift-IR -> Run-Test/Idle, 6:Exit1-DR/IR -> Update-DR/IR -> Run-Test/Idle

// �����л�JTAG״̬��,��ָ�������豸iIndex
BOOL WINAPI CH347Jtag_SwitchTapStateEx(ULONG iIndex,		// ָ���豸���
									   UCHAR TapState);		// ָ���л�����״̬

// JTAG DRд,���ֽ�Ϊ��λ,���ڶ��ֽ�������д����JTAG�̼����ز�����
// ״̬��:Run-Test->Shift-DR..->Exit DR -> Run-Test
BOOL	WINAPI	CH347Jtag_ByteWriteDR(ULONG			iIndex,        // ָ���豸���									
									  ULONG			iWriteLength,  // д����,׼��д�����ֽڳ���
									  PVOID			iWriteBuffer); // ָ��һ��������,����׼��д��������	

// JTAG DR��,���ֽ�Ϊ��λ,���ֽ���������
// ״̬��:Run-Test->Shift-DR..->Exit DR -> Run-Test
BOOL	WINAPI	CH347Jtag_ByteReadDR(ULONG			iIndex,        // ָ���豸���									
									 PULONG			oReadLength,   // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									 PVOID			oReadBuffer ); // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// JTAG IRд,���ֽ�Ϊ��λ,���ֽ�����д��
// ״̬��:Run-Test->Shift-IR..->Exit IR -> Run-Test
BOOL	WINAPI	CH347Jtag_ByteWriteIR(ULONG			iIndex,        // ָ���豸���									
									  ULONG			iWriteLength,  // д����,׼��д�����ֽڳ���
									  PVOID			iWriteBuffer); // ָ��һ��������,����׼��д��������										

// JTAG IR��,���ֽ�Ϊ��λ,���ֽ�������д��
// ״̬��:Run-Test->Shift-IR..->Exit IR -> Run-Test
BOOL	WINAPI	CH347Jtag_ByteReadIR(ULONG			iIndex,         // ָ���豸���									
									 PULONG			oReadLength,    // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									 PVOID			oReadBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// λ����ʽJTAG DR����д.�������������ݵĶ�д����ָ�������״̬���л��ȿ����ഫ�䡣���������ݴ��䣬����ʹ��USB20Jtag_ByeWriteDR
// ״̬��:Run-Test->Shift-DR..->Exit DR -> Run-Test
BOOL	WINAPI	CH347Jtag_BitWriteDR(ULONG			iIndex,             // ָ���豸���									
									 ULONG    	    iWriteBitLength,    // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									 PVOID			iWriteBitBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// λ����ʽJTAG IR����д.�������������ݵĶ�д����ָ�������״̬���л��ȿ����ഫ�䡣���������ݴ��䣬����ʹ��USB20Jtag_ByteWriteIR
// ״̬��:Run-Test->Shift-IR..->Exit IR -> Run-Test
BOOL	WINAPI	CH347Jtag_BitWriteIR(ULONG			iIndex,             // ָ���豸���									
									 ULONG    	    iWriteBitLength,    // ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									 PVOID			iWriteBitBuffer );  // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// λ����ʽJTAG IR���ݶ�.�������������ݵĶ�д����ָ�������״̬���л��ȡ����������ݴ��䣬����ʹ��USB20Jtag_ByteReadIR
// ״̬��:Run-Test->Shift-IR..->Exit IR -> Run-Test
BOOL	WINAPI	CH347Jtag_BitReadIR(ULONG			iIndex,				// ָ���豸���									
									PULONG    	oReadBitLength,			// ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									PVOID			oReadBitBuffer );	// ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

// λ����ʽJTAG DR���ݶ�.�������������ݵĶ�д���������͸������ݴ��䣬����ʹ��USB20Jtag_ByteReadDR
// ״̬��:Run-Test->Shift-DR..->Exit DR -> Run-Test
BOOL	WINAPI	CH347Jtag_BitReadDR(ULONG			iIndex,				// ָ���豸���									
									PULONG    	    oReadBitLength,		// ָ�򳤶ȵ�Ԫ,���غ�Ϊʵ�ʶ�ȡ���ֽڳ���
									PVOID			oReadBitBuffer );   // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������

/***************GPIO********************/
// ��ȡCH347��GPIO��������ŵ�ƽֵ
BOOL WINAPI CH347GPIO_Get(ULONG iIndex,       // ָ���豸���
						  UCHAR *iDir,        // ���ŷ���:GPIO0-7��Ӧλ0-7,	0=���룻1=���
						  UCHAR *iData);      // GPIO0��ƽ:GPIO0-7��Ӧλ0-7,0=�͵�ƽ��1=�ߵ�ƽ)

// ����CH347��GPIO��������ŵ�ƽֵ 
BOOL WINAPI CH347GPIO_Set(ULONG iIndex,        // ָ���豸���
						  UCHAR iEnable,       // ������Ч��־:��Ӧλ0-7,��ӦGPIO0-7.
						  UCHAR iSetDirOut,    // ����I/O����,ĳλ��0���Ӧ����Ϊ����,ĳλ��1���Ӧ����Ϊ���.GPIO0-7��Ӧλ0-7.
						  UCHAR iSetDataOut);  // �������,���I/O����Ϊ���,��ôĳλ��0ʱ��Ӧ��������͵�ƽ,ĳλ��1ʱ��Ӧ��������ߵ�ƽ


typedef		VOID	( CALLBACK	* mPCH347_INT_ROUTINE ) (  // �жϷ������
	                                                     PUCHAR			iStatus );  // �ж�״̬����,�ο������λ˵��
// 8���ֽ�GPIO0-7����״̬.ÿ�ֽ�λ��������:
// λ7����ǰ��GPIO0����0�����룻1�������
// λ6����ǰ��GPIO0��ƽ��0���͵�ƽ��1���ߵ�ƽ��
// λ5����ǰ��GPIO0�Ƿ�����Ϊ�жϣ�0����ѯģʽ��1���ж�ģʽ��
// λ4-3������GPIO0���ж�ģʽ��00���½��ش�����01�������ش���;10��˫���ش�����11: ����;
// λ2-0��������

// �趨GPIO�жϷ������
BOOL	WINAPI	CH347SetIntRoutine(ULONG			iIndex,           // ָ���豸���
	                               UCHAR           Int0PinN,          // �ж�0 GPIO���ź�,����7:�����ô��ж�Դ; Ϊ0-7��Ӧgpio0-7
	                               UCHAR           Int0TripMode,      // �ж�0����: 00:�½��ش���; 01:�����ش���; 02:˫���ش���; 03:����;
	                               UCHAR           Int1PinN,          // �ж�1 GPIO���ź�,����7�����ô��ж�Դ,Ϊ0-7��Ӧgpio0-7
	                               UCHAR           Int1TripMode,      // �ж�1����: 00:�½��ش���; 01:�����ش���; 02:˫���ش���; 03:����;
	                               mPCH347_INT_ROUTINE	iIntRoutine );// ָ���жϷ������,ΪNULL��ȡ���жϷ���,�������ж�ʱ���øó���

// ��ȡ�ж�����
BOOL	WINAPI	CH347ReadInter(ULONG			iIndex,      // ָ���豸���
	                           PUCHAR			iStatus );   // ָ��8�ֽڵ�Ԫ,�ֱ�ΪGPIO0-7����״̬,ÿ���ֽ�λ˵���ο��жϷ������iStatusλ˵�� 

// �����ж����ݶ�����
BOOL	WINAPI	CH347AbortInter(ULONG			iIndex );  // ָ���豸���

// ����IAP�̼�����ģʽ
BOOL	WINAPI	CH347StartIapFwUpate(ULONG    iIndex,
									 ULONG	  FwSize); // �̼�����

/**************HID/VCP����**********************/
// �򿪴���
HANDLE WINAPI CH347Uart_Open(ULONG iIndex);		// ָ���豸���

// �رմ���
BOOL WINAPI CH347Uart_Close(ULONG iIndex);		// ָ���豸���

BOOL	WINAPI	CH347Uart_SetDeviceNotify(												// �趨�豸�¼�֪ͨ����
										  ULONG					iIndex,					// ָ���豸���,0��Ӧ��һ���豸
										  PCHAR					iDeviceID,				// ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
										  mPCH347_NOTIFY_ROUTINE	iNotifyRoutine );   // ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���

// ��ȡUARTӲ������
BOOL    WINAPI  CH347Uart_GetCfg(ULONG   iIndex,       // ָ���豸���
								 PULONG  BaudRate,     // ������
								 PUCHAR  ByteSize,     // ����λ��(5,6,7,8,16)
								 PUCHAR  Parity,       // У��λ(0��None; 1��Odd; 2��Even; 3��Mark; 4��Space)
								 PUCHAR  StopBits,     // ֹͣλ��(0��1ֹͣλ; 1��1.5ֹͣλ; 2��2ֹͣλ)��
								 PUCHAR  ByteTimeout); // �ֽڳ�ʱ

// ����UART����								   
BOOL  WINAPI   CH347Uart_Init(ULONG  iIndex,      // ָ���豸���
							  DWORD  BaudRate,    // ������
							  UCHAR  ByteSize,    // ����λ��(5,6,7,8,16)
							  UCHAR  Parity,      // У��λ(0��None; 1��Odd; 2��Even; 3��Mark; 4��Space)
							  UCHAR  StopBits,    // ֹͣλ��(0��1ֹͣλ; 1��1.5ֹͣλ; 2��2ֹͣλ)��
							  UCHAR  ByteTimeout);// �ֽڳ�ʱʱ��,��λ100uS

// ����USB���ݶ�д�ĳ�ʱ
BOOL	WINAPI	CH347Uart_SetTimeout(ULONG			iIndex,         // ָ���豸���
									 ULONG			iWriteTimeout,  // ָ��USBд�����ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
									 ULONG			iReadTimeout ); // ָ��USB��ȡ���ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)

// ���ڶ�
BOOL  WINAPI   CH347Uart_Read(ULONG			iIndex,    // ָ���豸���
							  PVOID			oBuffer,   // ָ��һ���㹻��Ļ�����,���ڱ����ȡ������
							  PULONG	    ioLength );// ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���
// ����д
BOOL  WINAPI   CH347Uart_Write(ULONG		iIndex,    // ָ���豸���
							   PVOID		iBuffer,   // ָ��һ��������,����׼��д��������
							   PULONG		ioLength );// ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���

// ��ѯ���������ж����ֽ�δȡ��������HIDģʽ���ڣ�
BOOL WINAPI CH347Uart_QueryBufUpload(ULONG			iIndex,        // ָ���豸���
									 LONGLONG       *RemainBytes); // ��������δȡ�ֽ���

// ��ȡ�豸��Ϣ
BOOL WINAPI CH347Uart_GetDeviceInfor(ULONG iIndex,mDeviceInforS *DevInformation);

/********I2C***********/
// I2C����
BOOL	WINAPI	CH347I2C_Set(ULONG			iIndex,   // ָ���豸���
							 ULONG			iMode );  // ָ��ģʽ,������
// λ0-λ2: I2C�ӿ��ٶ�/SCLƵ��, 000=����/20KHz,001=��׼/100KHz(Ĭ��ֵ),010=����/400KHz,011=����/750KHz,100=����/50KHz,101=��׼/200KHz��110=����/1MHz
// ��������,����Ϊ0

// ����I2Cʱ����չ����
BOOL	WINAPI	CH347I2C_SetStretch(ULONG			iIndex,   // ָ���豸���
									BOOL			iEnable); // 0=�ر�ʱ����չ���ܣ�Ĭ�Ϲرգ���1=����ʱ����չ����

// ����Ӳ���첽��ʱ,���ú�ܿ췵��,������һ��������֮ǰ��ʱָ��������
BOOL	WINAPI	CH347I2C_SetDelaymS(ULONG			iIndex,       // ָ���豸���
									ULONG			iDelay ) ;    // ָ����ʱ�ĺ�����

// ����I2C��������ģʽ
BOOL	WINAPI CH347I2C_SetDriverMode(ULONG			iIndex,		  // ָ���豸���
									  UCHAR			iMode);		  // 0=��©ģʽ��1=����ģʽ

// ����I2C������,2�߽ӿ�,ʱ����ΪSCL����,������ΪSDA����
BOOL	WINAPI	CH347StreamI2C( ULONG			iIndex,        // ָ���豸���
							    ULONG			iWriteLength,  // ׼��д���������ֽ���
							    PVOID			iWriteBuffer,  // ָ��һ��������,����׼��д��������,���ֽ�ͨ����I2C�豸��ַ����д����λ
							    ULONG			iReadLength,   // ׼����ȡ�������ֽ���
							    PVOID			oReadBuffer ); // ָ��һ��������,���غ��Ƕ��������

// ����I2C������,2�߽ӿ�,ʱ����ΪSCL����,������ΪSDA����
BOOL	WINAPI	CH347StreamI2C_RetACK(  // ����I2C������,2�߽ӿ�,ʱ����ΪSCL����,������ΪSDA����(׼˫��I/O),�ٶ�Լ56K�ֽ�,�����������˻�ȡ����ACK����
									  ULONG			iIndex,        // ָ���豸���
									  ULONG			iWriteLength,  // ׼��д���������ֽ���
									  PVOID			iWriteBuffer,  // ָ��һ��������,����׼��д��������,���ֽ�ͨ����I2C�豸��ַ����д����λ
									  ULONG			iReadLength,   // ׼����ȡ�������ֽ���
									  PVOID			oReadBuffer,   // ָ��һ��������,���غ��Ƕ��������
									  PULONG		rAckCount);    // ָ���д���ص�ACKֵ

#ifndef _CH341_DLL_H
typedef	enum	_EEPROM_TYPE {// EEPROM�ͺ�
	ID_24C01,
	ID_24C02,
	ID_24C04,
	ID_24C08,
	ID_24C16,
	ID_24C32,
	ID_24C64,
	ID_24C128,
	ID_24C256,
	ID_24C512,
	ID_24C1024,
	ID_24C2048,
	ID_24C4096
} EEPROM_TYPE;
#endif

// ��EEPROM�ж�ȡ���ݿ�
BOOL	WINAPI	CH347ReadEEPROM(ULONG			iIndex,     // ָ���豸���
								EEPROM_TYPE		iEepromID,  // ָ��EEPROM�ͺ�
								ULONG			iAddr,      // ָ�����ݵ�Ԫ�ĵ�ַ
								ULONG			iLength,    // ׼����ȡ�������ֽ���
								PUCHAR			oBuffer );  // ָ��һ��������,���غ��Ƕ��������
// ��EEPROM��д�����ݿ�
BOOL	WINAPI	CH347WriteEEPROM(ULONG			iIndex,     // ָ���豸���
								 EEPROM_TYPE	iEepromID,  // ָ��EEPROM�ͺ�
								 ULONG			iAddr,      // ָ�����ݵ�Ԫ�ĵ�ַ
								 ULONG			iLength,    // ׼��д���������ֽ���
								 PUCHAR			iBuffer );  // ָ��һ��������,����׼��д��������

//���õ�8λʱ�ӵ�������ʱʱ�䣬��������CH347T
BOOL WINAPI CH347I2C_SetAckClk_DelayuS(ULONG iIndex,	// ָ���豸���
									   ULONG iDelay);	// ָ����ʱ��΢����

// �ú������ڲ�ѯָ��������CH339WоƬ��ǰ�����ӿڹ��ܵ�����״̬��
// ����ֵλ���壺
//    λ7 (0x80): USBתJTAGʹ��״̬��1=ʹ�ܣ�0=���ã�
//    λ6 (0x40): USBתSPIʹ��״̬��1=ʹ�ܣ�0=���ã�
//    λ5 (0x20): USBתUARTʹ�ܣ��������أ���1=ʹ�ܣ�0=���ã�
//    λ4 (0x10): USBתUART����ʹ�ܣ�1=ʹ�ܣ�0=���ã�
//    λ3 (0x08): USBתI2Cʹ��״̬��1=ʹ�ܣ�0=���ã�
//    λ2-λ0: ����λ
UCHAR	WINAPI	CH339GetChipFuncState( ULONG iIndex );	// ָ���豸���

#ifdef __cplusplus
}
#endif

#endif // _CH347_DLL_H
