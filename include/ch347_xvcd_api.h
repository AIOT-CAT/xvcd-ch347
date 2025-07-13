#ifndef _CH347_XVCD_API_H_
#define _CH347_XVCD_API_H_

#if defined(_WIN32) || defined(__CYGWIN__)|| defined(__CYGWIN32__) || defined(__CYGWIN64__) || defined(__MSYS__)
    #define  PLATFORM_WINDOWS  1
    #define  _CRT_SECURE_NO_WARNINGS
    #define  _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #include <process.h>
    #pragma  comment(lib, "Ws2_32.lib")
    #include "CH347DLL.H"
#else
    #define  PLATFORM_UNIX 1
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/io.h>
    #include <libusb-1.0/libusb.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <time.h>

#define CH347_EP_OUT 0x06u
#define CH347_EP_IN  0x86u

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

#define KHZ(n) ((n)*UINT64_C(1000))
#define MHZ(n) ((n)*UINT64_C(1000000))
#define GHZ(n) ((n)*UINT64_C(1000000000))

#define HW_TDO_BUF_SIZE              4096
#define SF_PACKET_BUF_SIZE           51200 /* Command packet length */
#define UCMDPKT_DATA_MAX_BYTES_USBHS 507   /* The data length contained in each command packet during USB high-speed operation */
#define UCMDPKT_DATA_MAX_BITS_USBHS  248   /* bit mode transfer 248 byte */
#define USBC_PACKET_USBHS            512   /* Maximum data length per packet at USB high speed */
#define USBC_PACKET_USBHS_SINGLE     510   /* usb high speed max package length */
#define CH347_CMD_HEADER             3     /* Protocol header length */
#define CH347_CMD_HEADER             3     /* 协议包头长度
                                              Protocol transmission format: CMD (1 byte) + Length (2 bytes) + Data */
#define CH347_CMD_INFO_RD            0xCA  /* Parameter acquisition, used to obtain firmware version,
                                              TAG interface related parameters, etc */
#define CH347_CMD_JTAG_INIT          0xD0  /* JTAG Interface Initialization Command */
#define CH347_CMD_JTAG_BIT_OP        0xD1  /* JTAG interface pin bit control command */
#define CH347_CMD_JTAG_BIT_OP_RD     0xD2  /* JTAG interface pin bit control and read commands */
#define CH347_CMD_JTAG_DATA_SHIFT    0xD3  /* JTAG interface data shift command */
#define CH347_CMD_JTAG_DATA_SHIFT_RD 0xD4  /* JTAG interface data shift and read command */

typedef struct ch347_dev_info {
    uint16_t usb_vid;
    uint16_t usb_pid;
    uint16_t usb_bcd;
    uint8_t usb_manu[256];
    uint8_t usb_product[256];
    uint8_t usb_serial[256];
#ifdef PLATFORM_WINDOWS
    uint32_t usb_index;
#else
    libusb_device *dev_handle;
    libusb_device_handle *op_handle;
#endif 
    uint32_t count;
}dev_ctx;

typedef
#ifdef PLATFORM_WINDOWS
    SOCKET
#else
    int
#endif
    socket_t;

#define INVALID_SOCKET_VAL (socket_t)(~0)
#define __SOCKET_ERROR	(-1)

#define CH347_VENDOR_ID 0x1a86
#define CH347T_PRODUCT_ID 0x55DD
#define CH347F_PRODUCT_ID 0x55DE


int network_init();
void network_cleanup();
void close_socket(socket_t socket);
int get_last_error();
int sread(socket_t fd, void *target, int len);
int swrite(socket_t fd, void *target, int len);
int ch347_open(dev_ctx *ch347_ctx);
int ch347_close(dev_ctx *ch347_ctx);
int ch347_jtag_init(dev_ctx *ch347_ctx, uint8_t clock);
int ch347_write(dev_ctx *ch347_ctx, uint8_t *data, uint32_t *len);
int ch347_read(dev_ctx *ch347_ctx, uint8_t *data, uint32_t *len);

#endif //_CH347_XVCD_API_H_