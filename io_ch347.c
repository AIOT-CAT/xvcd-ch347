#include <stdio.h>
#include <string.h>
#include <stdint.h>
//#include <unistd.h>
#include <io.h>
#include <process.h>
#include <windows.h>
#include "io_ch347.h"
#include "CH347/CH347DLL.H"
#define SINGLEDATA 1
#define PACKDATA   0

#define MAX_BUFFER  512
typedef uint8_t bool;
#define true 1
#define false 0
int usb_xfer(unsigned wlen, unsigned rlen, unsigned* ract, bool defer);
int flush() { return usb_xfer(0, 0, 0, false); }
/*********/

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
#define UCMDPKT_DATA_MAX_BITS_USBHS  248 // 248

#define KHZ(n) ((n) * (unsigned long)(1000))
#define MHZ(n) ((n) * (unsigned long)(1000000))
#define GHZ(n) ((n) * (unsigned long)(1000000000))

unsigned int iIndex = 0;

uint8_t ibuf[MAX_BUFFER];
uint8_t _obuf[MAX_BUFFER];
uint8_t* obuf = _obuf;
int get_buffer_size()  { return get_obuf_length(); }
int get_obuf_length() { return MAX_BUFFER - (obuf - _obuf); }
bool isFull() { return get_obuf_length() == 0; }
int io_init(unsigned int index)
{
    int RetVal;
    mDeviceInforS DevInfor = {0};

    // Open the CH347
    if (CH347OpenDevice(index) != INVALID_HANDLE_VALUE) {
        printf("Open CH347 Succes.\n");
	} else {
		printf("Open CH347 fail.\n");
		return -1;
	}

    // Set the time for USB timeout return
    CH347SetTimeout(index, 1000, 1000);

    iIndex = index;

    // Init the CH347 default clock rate : 3.75MHz
    RetVal = CH347Jtag_INIT(iIndex, 3);
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
	long clock_rate = 1000000000 / period;
	int speed_clock[] = { KHZ(468.75), KHZ(937.5), MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60) };
	// int speed_clock[] = {MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60)};

	for (i = 0; i < sizeof(speed_clock) / sizeof(int); i++) {
		if (clock_rate < speed_clock[i]) {
			clockIndex = i;
			break;
		}
	}
	if (clockIndex > 7 || clock_rate > MHZ(60)) {
		clockIndex = 7;
	}
	RetVal = CH347Jtag_INIT(iIndex, clockIndex);
	if (!RetVal) {
		printf("CH347 Set Clock failed\n");
		return -1;
	}
	printf("CH347 Set Clock : %d.\n", speed_clock[clockIndex]);
	period = MHZ(1000) / speed_clock[i];
	if (period > 10)
		period = period - (period % 10);
	return period;
}

int writeTDI(const uint8_t* tx, uint8_t* rx, uint32_t len, bool end)
{
	if (len == 0) {
		return 0;
	}
	unsigned bytes = (len - (end ? 1 : 0)) / 8;
	unsigned bits = len - bytes * 8;
	uint8_t* rptr = rx;
	const uint8_t* tptr = tx;
	const uint8_t* txend = tx + bytes;
	uint8_t cmd = (rx != NULL) ? CH347_CMD_JTAG_DATA_SHIFT_RD : CH347_CMD_JTAG_DATA_SHIFT;
	while (tptr < txend) {
		if (get_obuf_length() < 4) {
			flush();
			if (!isFull() || obuf != _obuf) {
				printf("flush fail\n");
			}
		}
		if (obuf != _obuf) printf("have data\n");
		int avail = get_obuf_length() - 3;
		int chunk = (txend - tptr < avail) ? txend - tptr : avail;
		if (tx) {
			memcpy(&obuf[3], tptr, chunk);
		}
		else {
			memset(&obuf[3], 0, chunk);
		}
		tptr += chunk;
		// write header
		obuf[0] = cmd;
		obuf[1] = (chunk >> 0) & 0xff;
		obuf[2] = (chunk >> 8) & 0xff;
		unsigned actual_length = 0;
		int ret = usb_xfer(chunk + 3, (rx) ? chunk + 3 : 0, &actual_length, rx == 0 && get_obuf_length());
		if (!rx)
			continue;
		if (ibuf[0] != CH347_CMD_JTAG_DATA_SHIFT_RD) {
			printf("ibuf[0] != CH347_CMD_JTAG_DATA_SHIFT_RD\n");
		}
		unsigned size = ibuf[1] + ibuf[2] * 0x100;
		if (ibuf[0] != CH347_CMD_JTAG_DATA_SHIFT_RD || actual_length - 3 != size) {
			printf("writeTDI: invalid read data\n");
		}
		memcpy(rptr, &ibuf[3], size);
		rptr += size;
	}
	unsigned actual_length;
	if (bits == 0)
		return EXIT_SUCCESS;
	cmd = (rx) ? CH347_CMD_JTAG_BIT_OP_RD : CH347_CMD_JTAG_BIT_OP;
	if (get_obuf_length() < (int)(4 + bits * 2)) {
		flush();
	}
	uint8_t* ptr = &obuf[3];
	uint8_t x = 0;
	const uint8_t* bptr = tx + bytes;
	for (unsigned i = 0; i < bits; ++i) {
		uint8_t txb = (tx) ? bptr[i >> 3] : 0;
		uint8_t _tdi = (txb & (1 << (i & 7))) ? TDI_H : 0;
		x = _tdi;
		if (i == bits - 1) {
			x |= TMS_H;
		}
		*ptr++ = x;
		*ptr++ = x | TCK_H;
	}
	*ptr++ = x & ~TCK_H;
	unsigned wlen = ptr - obuf;
	obuf[0] = cmd;
	obuf[1] = wlen - 3;
	obuf[2] = (wlen - 3) >> 8;
	int ret = usb_xfer(wlen, (rx) ? (bits + 3) : 0, &actual_length, rx == NULL);

	if (ret < 0) {
		printf("writeTDI: usb bulk read failed\n");
	}
	if (!rx)
		return EXIT_SUCCESS;

	unsigned size = ibuf[1] + ibuf[2] * 0x100;
	if (ibuf[0] != CH347_CMD_JTAG_BIT_OP_RD || actual_length - 3 != size) {
		printf("writeTDI: invalid read data\n");
	}
	for (unsigned i = 0; i < size; ++i) {
		if (ibuf[3 + i] == 0x01) {
			*rptr |= (0x01 << i);
		}
		else {
			*rptr &= ~(0x01 << i);
		}
	}
	return EXIT_SUCCESS;
}

bool is_all_zero(void* ptr, size_t size) {
	unsigned char* byte_ptr = (unsigned char*)ptr;

	for (size_t i = 0; i < size; i++) {
		if (byte_ptr[i] != 0) {
			return false;
		}
	}
	return true; 
}

int io_scan(const unsigned char *TMS, const unsigned char *TDI, unsigned char *TDO, int bits, int state)
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
	// TODO：状态为shift_dr或shift_ir，但tms仍有可能不全为0
	if ((state == shift_dr || state == shift_ir) && is_all_zero(TMS, (bits+7)/8)) {
		// set tck to low
		v = TCK_L | TMS_L | TDI_L;
		unsigned char settck[] = { 0xD2,0x01,0x00,0x00 };
		int len = 4;
		if (!CH347WriteData(iIndex, settck, &len) || len != 4) {
			printf("%d : CH347WriteData failed.\n", __LINE__);
			return -1;
		}
		len = 10;
		if (!CH347ReadData(iIndex, settck, &len)) {
			printf("%d : CH347ReadData failed.\n", __LINE__);
			return -1;
		}
		return writeTDI(TDI, TDO, bits, false);
	}
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
            if (TMS[nb8 + (i / 8)] & (1 << (i & 7))) {
                v |= TMS_H;
            }
            if (TDI[nb8 + (i / 8)] & (1 << (i & 7))) {
                v |= TDI_H;
            }
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
        RetVal = CH347ReadData(iIndex, rbuffer, &
        Txlen);
        if (!RetVal) {
            printf("CH347 read data failed.\n");
            return -1;
        }
        memcpy(&buffer[DI], &rbuffer[CH347_CMD_HEADER], Txlen);

        if (Txlen != (length + 3)) {
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

int usb_xfer(unsigned wlen, unsigned rlen, unsigned* ract, bool defer) {
	unsigned long actual_length = 0;

	if (defer && !rlen && obuf - _obuf > (wlen + 12)) {
		obuf += wlen;
		return 0;
	}

	if (obuf - _obuf > MAX_BUFFER) {
		printf("buffer overflow\n");
	}

	wlen += obuf - _obuf;
	if (wlen > MAX_BUFFER) {
		printf("buffer overflow\n");
	}
	obuf = _obuf;

	if (wlen == 0) {
		return 0;
	}

	int r = 0;
	if (wlen) {
		actual_length = wlen;
		if (!CH347WriteData(iIndex, obuf, &actual_length) || actual_length != wlen) {
			printf("write fail.\n");
			return -1;
		}
	}
	obuf = _obuf;
	int rlen_total = 0;
	uint8_t* pibuf = ibuf;
	if (rlen) {
		while (rlen) {
			actual_length = rlen;
			if (!CH347ReadData(iIndex, pibuf, &actual_length)) {
				printf("read fail.\n");
				return -1;
			}
			rlen -= actual_length;
			pibuf += actual_length;
			rlen_total += actual_length;
		}
		*ract = rlen_total;
	}
	return 0;
}
