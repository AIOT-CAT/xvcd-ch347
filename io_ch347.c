#include "CH347DLL.H"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>

#define SINGLEDATA 1
#define PACKDATA   0

#define JTAGIO_STA_OUT_TDI  (0x10)
#define JTAGIO_STA_OUT_TMS  (0x02)
#define JTAGIO_STA_OUT_TCK  (0x01)
#define JTAGIO_STA_OUT_TRST (0x20)
#define TDI_H               JTAGIO_STA_OUT_TDI
#define TDI_L               0
#define TMS_H               JTAGIO_STA_OUT_TMS
#define TMS_L               0
#define TCK_H               JTAGIO_STA_OUT_TCK
#define TCK_L               0
#define TRST_H              JTAGIO_STA_OUT_TRST
#define TRST_L              0

#define CH347_CMD_HEADER 3 // 协议包头长度
// Protocol transmission format: CMD (1 byte)+Length (2 bytes)+Data
#define CH347_CMD_INFO_RD            0xCA // Parameter acquisition, used to obtain firmware version, JTAG interface related parameters, etc
#define CH347_CMD_JTAG_INIT          0xD0 // JTAG Interface Initialization Command
#define CH347_CMD_JTAG_BIT_OP        0xD1 // JTAG interface pin bit control command
#define CH347_CMD_JTAG_BIT_OP_RD     0xD2 // JTAG interface pin bit control and read commands
#define CH347_CMD_JTAG_DATA_SHIFT    0xD3 // JTAG interface data shift command
#define CH347_CMD_JTAG_DATA_SHIFT_RD 0xD4 // JTAG interface data shift and read command

#define HW_TDO_BUF_SIZE              4096
#define UCMDPKT_DATA_MAX_BYTES_USBHS 507 // The data length contained in each command packet during USB high-speed operation
#define UCMDPKT_DATA_MAX_BITS_USBHS  248

#define KHZ(n) ((n) * (unsigned long)(1000))
#define MHZ(n) ((n) * (unsigned long)(1000000))
#define GHZ(n) ((n) * (unsigned long)(1000000000))

unsigned int iIndex = 0;

int io_init(unsigned int index)
{
    int RetVal;
    mDeviceInforS DevInfor = {0};

    // Open the CH347
    if (CH347OpenDevice(index) != INVALID_HANDLE_VALUE) {
        printf("Open CH347 Succes.\n");
    }

    // Set the time for USB timeout return
    CH347SetTimeout(index, 500, 500);

    iIndex = index;

    // Get the CH347 information
    RetVal = CH347GetDeviceInfor(index, &DevInfor);
    if (!RetVal) {
        printf("Get CH347 Info failed.\n");
        return -1;
    }

    // Judge whether CH347 operates in mode 3 (JTAG + UART Mode).
    if (DevInfor.ChipMode != 3) {
        printf("The CH347:[%d] not the JTAG mode[3], Current Mode is ==> [%d].\n ", index, DevInfor.ChipMode);
        return -1;
    }

    // Init the CH347 default clock rate : 30MHz
    RetVal = CH347Jtag_INIT(iIndex, 4);
    if (!RetVal)
        return -1;
    printf("CH347:[%d] init done.\n", index);

    return 0;
}

int io_set_period(unsigned int index, unsigned int period)
{
    int i = 0;
    int clockIndex = 0;
    int RetVal;
    int clock_rate = MHZ(10000) / period;
    printf("Clock_Rete:%d.\n", clock_rate);
    int speed_clock[] = {KHZ(468.75), KHZ(937.5), MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60)};
    // int speed_clock[] = {MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60)};

    for (i = 0; i < sizeof(speed_clock) / sizeof(int); i++) {
        if ((clock_rate >= speed_clock[i]) && (clock_rate <= speed_clock[i + 1])) {
            if (i < 5)
                clockIndex = i + 1;
            else
                clockIndex = i;
            RetVal = CH347Jtag_INIT(iIndex, clockIndex);
            if (!RetVal)
                return -1;
            printf("CH347 Set Clock : %d.\n", speed_clock[i]);
            break;
        }
    }

    period = MHZ(1000) / speed_clock[i];
    if (period > 10)
        period = period - (period % 10);
    printf("period = %d.\n", period);
    return period;
}

int io_scan(const unsigned char *TMS, const unsigned char *TDI, unsigned char *TDO, int bits)
{
    unsigned char v;
    unsigned char CmdBuffer[2 * 16384 + 512];
    unsigned char buffer[2 * 16384 + 512];
    unsigned char rbuffer[2 * 16384 + 512];
    unsigned long i, length, DI, DII, RetVal;
    unsigned long BI, Txlen;

    int count = 0;
    int nb8 = 0;

    if (bits > sizeof(CmdBuffer) / 2) {
        fprintf(stderr, "FATAL: out of buffer space for %d bits\n", bits);
        return -1;
    }

    DI = DII = BI = 0;

    while (DI < bits) {
        if ((bits - DI) > UCMDPKT_DATA_MAX_BITS_USBHS)
            length = UCMDPKT_DATA_MAX_BITS_USBHS;
        else
            length = bits - DI;

        CmdBuffer[BI++] = CH347_CMD_JTAG_BIT_OP_RD;
        CmdBuffer[BI++] = (unsigned char)(((length * 2) >> 0) & 0xFF);
        CmdBuffer[BI++] = (unsigned char)(((length * 2) >> 8) & 0xFF);

        for (i = 0; i < length; ++i) {
            v = TCK_L | TMS_L | TDI_L;
            if (TMS[nb8 + (i / 8)] & (1 << (i & 7)))
                v |= TMS_H;
            if (TDI[nb8 + (i / 8)] & (1 << (i & 7)))
                v |= TDI_H;
            CmdBuffer[BI++] = v;
            CmdBuffer[BI++] = v | TCK_H;
        }

        // 添加用于处理大包数据时组包操作参数
        RetVal = CH347WriteData(iIndex, CmdBuffer, &BI);
        if (!RetVal) {
            printf("CH347 Write data failed.\n");
            return -1;
        }

        Txlen = length + 3;
        // RetVal = CH347ReadData(iIndex, buffer + DI, &Txlen);
        RetVal = CH347ReadData(iIndex, rbuffer, &Txlen);
        if (!RetVal) {
            printf("CH347 read data failed.\n");
            return -1;
        }
        memcpy(&buffer[DI], &rbuffer[CH347_CMD_HEADER], Txlen);

        if (Txlen != (length + 3)) {
            // RetVal = CH347ReadData(iIndex, buffer + DI + Txlen, &BI);
            RetVal = CH347ReadData(iIndex, rbuffer, &BI);
            if (!RetVal) {
                printf("CH347 read data failed.\n");
                return -1;
            }
            memcpy(&buffer[DI + Txlen], rbuffer, BI);
        }
        BI = 0;
        memset(CmdBuffer, 0, sizeof(CmdBuffer));

        DI += length;
        nb8 += (length / 8);
    }
    memset(TDO, 0, (bits + 7) / 8);

    for (i = 0; i < bits; ++i) {
        if (buffer[i] & 0x01) {
            TDO[i / 8] |= 1 << (i & 7);
        }
    }

    return 0;
}

void io_close(void)
{
    CH347CloseDevice(iIndex);
}