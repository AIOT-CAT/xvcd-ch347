#include "ch347_xvcd_api.h"

/* socket 相关函数 */
int network_init()
{
#ifdef PLATFORM_WINDOWS
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    return 0; // UNIX无需初始化
#endif
}

void network_cleanup()
{
#ifdef PLATFORM_WINDOWS
    WSACleanup();
#endif
}

void close_socket(socket_t socket)
{
#ifdef PLATFORM_WINDOWS
    closesocket(socket);
#else
    close(socket);
#endif
}

int get_last_error()
{
#ifdef PLATFORM_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}

int sread(socket_t fd, void *target, int len)
{
    char *t = target;
    while (len) {
        int r = recv(fd, t, len, 0);
        if (r == __SOCKET_ERROR || r == 0) {
            int err = get_last_error();
            printf("Recv failed with error: %d\n", err);
            if (r == 0) {
                puts("Seems like connection was gracefully closed. Exiting");
            }
            return r;
        }
        t += r;
        len -= r;
    }
    return 1;
}

int swrite(socket_t fd, void *target, int len)
{
    char *t = target;
    while (len) {
        int r = send(fd, t, len, 0);
        if (r == __SOCKET_ERROR || r == 0) {
            int err = get_last_error();
            printf("Send failed with error: %d\n", err);
            if (r == 0) {
                puts("Seems like connection was gracefully closed. Exiting");
            }
            return r;
        }
        t += r;
        len -= r;
    }
    return 1;
}

/* ch347操作相关函数 */
int ch347_open(dev_ctx *ch347_ctx)
{
    int ret = 0;
#if PLATFORM_WINDOWS
    if (CH347OpenDevice(ch347_ctx->usb_index) != INVALID_HANDLE_VALUE) {
        printf("Open CH347 Succes.\n");
    } else {
        printf("Open CH347 fail.\n");
        return -1;
    }

    CH347SetTimeout(ch347_ctx->usb_index, 1000, 1000);
#else
    if (ch347_ctx->dev_handle == NULL)
        return -1;

    ret = libusb_init(NULL);
    if (ret < 0) {
        printf("libusb_init failed: %s\n", libusb_error_name(ret));
        return ret;
    }

    ret = libusb_open(ch347_ctx->dev_handle, &ch347_ctx->op_handle);
    if (ret < 0) {
        printf("libusb_open failed: %s\n", libusb_error_name(ret));
        return ret;
    }

    // claim interface x
    int interface_num = 4;
    ret = libusb_claim_interface(ch347_ctx->op_handle, interface_num);
    if (ret < 0) {
        printf("libusb_claim_interface failed: %s\n", libusb_error_name(ret));
        return ret;
    }
#endif
    return 0;
}

int ch347_close(dev_ctx *ch347_ctx)
{
#if PLATFORM_WINDOWS
    CH347CloseDevice(ch347_ctx->usb_index);
#else
    libusb_close(ch347_ctx->op_handle);
    libusb_exit(NULL);
#endif
    return 0;
}

int ch347_jtag_init(dev_ctx *ch347_ctx, uint8_t clock)
{
    int ret = 0;
#if PLATFORM_WINDOWS
    ret = CH347Jtag_INIT(ch347_ctx->usb_index, clock);
    if (!ret) {
        printf("CH347Jtag_INIT failed: %d\n", ret);
        return -1;
    }
#else
    unsigned long int i = 0, j;
    bool retVal;
    uint8_t cmdBuf[32] = "";
    cmdBuf[i++] = CH347_CMD_JTAG_INIT;
    cmdBuf[i++] = 6;
    cmdBuf[i++] = 0;

    cmdBuf[i++] = 0;
    cmdBuf[i++] = clock;

    for (j = 0; j < 4; j++)
        cmdBuf[i++] = TCK_L | TMS_H | TDI_L | TRST_H;

    uint32_t mLength = i;
    if (!ch347_write(ch347_ctx, cmdBuf, &mLength) || (mLength != i))
        return -1;

    mLength = 4;
    memset(cmdBuf, 0, sizeof(cmdBuf));

    if (!ch347_read(ch347_ctx, cmdBuf, &mLength) || (mLength != 4))
        return -1;

    retVal = ((cmdBuf[0] == CH347_CMD_JTAG_INIT) &&
              (cmdBuf[CH347_CMD_HEADER] == 0));
#endif
    return 0;
}

int ch347_write(dev_ctx *ch347_ctx, uint8_t *data, uint32_t *len)
{
    int ret = 0;
#if PLATFORM_WINDOWS
    ULONG length = *len;
    ret = CH347WriteData(ch347_ctx->usb_index, data, &length);
    if (!ret) {
        printf("CH347WriteData failed: %d\n", ret);
        return false;
    }
    *len = length;
#else
    int tmp_length = 0;
    ret = libusb_bulk_transfer(ch347_ctx->op_handle,
                               CH347_EP_OUT,
                               (char *)data,
                               *len,
                               &tmp_length, 100);
    *len = tmp_length;

    if (ret < 0) {
        printf("libusb_bulk_transfer write failed: %d\n", ret);
        return false;
    }
#endif
    return true;
}

int ch347_read(dev_ctx *ch347_ctx, uint8_t *data, uint32_t *len)
{
    int ret = 0;
#if PLATFORM_WINDOWS
    ULONG length = *len;
    ret = CH347ReadData(ch347_ctx->usb_index, data, &length);
    if (!ret) {
        printf("CH347ReadData failed: %d\n", ret);
        return false;
    }
    *len = length;
#else
    int tmp_length = 0;
    int size = *len;
    ret = libusb_bulk_transfer(ch347_ctx->op_handle,
                               CH347_EP_IN,
                               (char *)data,
                               size,
                               &tmp_length, 100);

    *len = tmp_length;
    if (ret < 0) {
        printf("libusb_bulk_transfer read failed: %d\n", ret);
    }
#endif
    return true;
}