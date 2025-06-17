// xilinx virtual cable demon for jlink jtag
// use connection string in Impact and another
// xilinx_xvc host=localhost:2542 disableversioncheck=true
#include "io_ch347.h"

// #pragma comment(lib, "Ws2_32.lib")
#define VECTOR_IN_SZ 2048

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

int DEFAULT_JTAG_SPEED = 30000000; // 30 Mhz

static int jtag_state;
static int verbose = 0;

/*Listen on default port 2542.*/
int iPort = 2542;
static int vlevel = 1;
unsigned long iIndex_ch347 = 0;
unsigned char szAddress[32] = "";
unsigned char serialNumber[32] = "";

dev_ctx xvcd_dev[16];

static int jtag_step(int state, int tms)
{
    static const int next_state[num_states][2] =
        {
            [test_logic_reset] = {run_test_idle, test_logic_reset},
            [run_test_idle] = {run_test_idle, select_dr_scan},

            [select_dr_scan] = {capture_dr, select_ir_scan},
            [capture_dr] = {shift_dr, exit1_dr},
            [shift_dr] = {shift_dr, exit1_dr},
            [exit1_dr] = {pause_dr, update_dr},
            [pause_dr] = {pause_dr, exit2_dr},
            [exit2_dr] = {shift_dr, update_dr},
            [update_dr] = {run_test_idle, select_dr_scan},

            [select_ir_scan] = {capture_ir, test_logic_reset},
            [capture_ir] = {shift_ir, exit1_ir},
            [shift_ir] = {shift_ir, exit1_ir},
            [exit1_ir] = {pause_ir, update_ir},
            [pause_ir] = {pause_ir, exit2_ir},
            [exit2_ir] = {shift_ir, update_ir},
            [update_ir] = {run_test_idle, select_dr_scan}};

    return next_state[state][tms];
}

void printTDO(const unsigned char *buf, int bitLen)
{
    printf(" TDO: ");
    for (int i = 0; i < bitLen; i += 8) {
        printf("%.2X", buf[(bitLen - i - 1) / 8]);
    }
    printf("\n");
}

int getInt32(unsigned char *data)
{
    // Return an int from the received byte string, data. data is
    // expected to be 4 bytes long.
    int num;

    // The int32 in data is sent little endian
    num = data[3];
    num = (num << 8) | data[2];
    num = (num << 8) | data[1];
    num = (num << 8) | data[0];

    return num;
}

void putInt32(unsigned char *data, int num)
{
    // Convert the int32_t number, num, into a 4-byte, little endian
    // string pointed to by data
    data[0] = num & 0x00ff;
    num >>= 8;
    data[1] = num & 0x00ff;
    num >>= 8;
    data[2] = num & 0x00ff;
    num >>= 8;
    data[3] = num & 0x00ff;
    num >>= 8;
}

//
//   handle_data(fd) handles JTAG shift instructions.
//   To allow multiple programs to access the JTAG chain
//   at the same time, we only allow switching between
//   different clients only when we're in run_test_idle
//   after going test_logic_reset. This ensures that one
//   client can't disrupt the other client's IR or state.
//
int handle_data(dev_ctx *ch347_ctx, socket_t fd, unsigned long frequency)
{
    int i;
    int seen_tlr = 0;
    int retVal = 0;
    const char xvcInfo[] = "xvcServer_v1.0:" TOSTRING(VECTOR_IN_SZ) "\n";
    do {
        char cmd[16];
        unsigned char buffer[2048], result[2048];
        // unsigned char buffer[4096], result[4096];

        if (sread(fd, cmd, 2) != 1)
            return 1;
        if (memcmp(cmd, "ge", 2) == 0) { // 获取信息
            if (sread(fd, cmd, 6) != 1)
                return 1;
            memcpy(result, xvcInfo, strlen(xvcInfo));

            if (swrite(fd, result, strlen(xvcInfo)) != 1) {
                printf("[%d]write %d bytes != %ld\n", __LINE__, retVal, strlen(xvcInfo));
                return 1;
            }
            if (vlevel > 0) {
                printf("%u : Received command: 'getinfo'\n", (int)time(NULL));
                printf("\t Replied with %s\n", xvcInfo);
            }
            break;
        } else if (memcmp(cmd, "se", 2) == 0) { // 速度设置
            if (sread(fd, cmd, 9) != 1)
                return 1;

            // Convert the 4-byte little endian integer after "settck:" to be an integer
            int period, actPeriod;

            // if frequency argument is non-0, use it instead of the
            // period from the settck: command
            if (frequency == 0) {
                period = getInt32((unsigned char *)cmd + 5);
            } else {
                period = 1000000000 / frequency;
            }

            printf("Get the period:%d.\n", period);

            actPeriod = io_set_period(ch347_ctx, (unsigned int)period);

            if (actPeriod < 0) {
                fprintf(stderr, "Error while setting the JTAG TCK period\n");
                actPeriod = period; /* on error, simply echo back the period value so client while not try to change it*/
            }

            putInt32(result, actPeriod);

            if (swrite(fd, result, 4) != 1) {
                return 1;
            }
            if (vlevel > 0) {
                printf("%u : Received command: 'settck'\n", (int)time(NULL));
                printf("\t Replied with '%d'\n\n", actPeriod);
            }
            break;
        } else if (memcmp(cmd, "sh", 2) == 0) {
            if (sread(fd, cmd, 4) != 1) {
                return 1;
            }
            if (vlevel > 1) {
                printf("%u : Received command: 'shift'\n", (int)time(NULL));
            }
        } else {
            fprintf(stderr, "invalid cmd '%s'\n", cmd);
            // return 1;
        }

        // 读取shift数据长度
        if (sread(fd, cmd + 6, 4) != 1) {
            fprintf(stderr, "reading length failed\n");
            return 1;
        }

        int len;
        len = getInt32((unsigned char *)cmd + 6);

        int nr_bytes = (len + 7) / 8;
        if (nr_bytes * 2 > sizeof(buffer)) {
            fprintf(stderr, "buffer size exceeded\n");
            return 1;
        }

        if (sread(fd, buffer, nr_bytes * 2) != 1) {
            fprintf(stderr, "reading data failed\n");
            return 1;
        }
        memset(result, 0, sizeof(result));

        if (vlevel > 2) {
            printf("\tNumber of Bits  : %d\n", len);
            printf("\tNumber of Bytes : %d \n", nr_bytes);
            for (i = 0; i < nr_bytes; i++)
                printf("%02x ", buffer[i]);
            printf("\n");
        }

        // Only allow exiting if the state is rti and the IR
        // has the default value (IDCODE) by going through test_logic_reset.
        // As soon as going through capture_dr or capture_ir no exit is
        // allowed as this will change DR/IR.
        seen_tlr = (seen_tlr || jtag_state == test_logic_reset) && (jtag_state != capture_dr) && (jtag_state != capture_ir);

        // Due to a weird bug(??) xilinx impacts goes through another "capture_ir"/"capture_dr" cycle after
        // reading IR/DR which unfortunately sets IR to the read-out IR value.
        // Just ignore these transactions.
        if ((jtag_state == exit1_ir && len == 5 && buffer[0] == 0x17) ||
            (jtag_state == exit1_dr && len == 4 && buffer[0] == 0x0b)) {
            if (verbose)
                printf("ignoring bogus jtag state movement in jtag_state %d\n", jtag_state);
        } else {
            for (i = 0; i < len; ++i) {
                // Do the actual cycle.
                int tms = !!(buffer[i / 8] & (1 << (i & 7)));

                // Track the state.
                jtag_state = jtag_step(jtag_state, tms);
            }

            if (io_scan(ch347_ctx, buffer, buffer + nr_bytes, result, len, jtag_state) < 0) {
                fprintf(stderr, "io_scan failed\n");
                exit(1);
            }

            if (verbose) {
                for (i = 0; i < nr_bytes; ++i)
                    printf("%02x ", result[i]);
            }
        }

        if (send(fd, (const char *)result, nr_bytes, 0) != nr_bytes) {
            printf("Send failed with error: %d\n", get_last_error());
            return 1;
        }

        if (verbose) {
            printf("jtag state %d\n", jtag_state);
        }
    } while (!(seen_tlr && jtag_state == run_test_idle));
    if (verbose) {
        puts("exit handle_data");
    }
    return 0;
}

void usage(void)
{
    const char use[] =
        "  Usage:\n"
        " -h, --help                     display this message\n"
        " -a, --address <host add xxx>   specify host address, default is 127.0.0.1\n"
        " -p, --port <port num>          specify socket port, default is 2542\n"
        " -i, --index <index num>        specify CH347 index, default is 0\n"
        " -s, --speed <ch347 speed>      specify CH347 JTAG speed, default is 30MHz\n"
        " -S, --serialnumber <ch347 usb serialnumber> Specify the CH347 serial number to open\n"
        "\n\n";
    printf(use);
    ;
    exit(0);
}

int main(int argc, char **argv)
{
    int ret = 0;
    int i, c, iResult;
    socket_t s;
    struct sockaddr_in address;
    unsigned short port = iPort;
    unsigned short jtagSpeed = DEFAULT_JTAG_SPEED;
    unsigned int coreId = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage();
        } else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--address") && i + 1 < argc) {
            strcpy(szAddress, argv[i + 1]);
            ++i;
        } else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port") && i + 1 < argc) {
            iPort = atoi(argv[i + 1]);
            printf("port: %d\n", iPort);
            ++i;
        } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--index") && i + 1 < argc) {
            iIndex_ch347 = atoi(argv[i + 1]);
            ++i;
        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--speed") && i + 1 < argc) {
            DEFAULT_JTAG_SPEED = atoi(argv[i + 1]);
            ++i;
        } else if (!strcmp(argv[i], "-S") || !strcmp(argv[i], "--serialnum") && i + 1 < argc) {
            strcpy(serialNumber, argv[i + 1]);
            ++i;
        } else {
            usage();
        }
    }
#if PLATFORM_WINDOWS
    xvcd_dev[iIndex_ch347].usb_index = iIndex_ch347;
#else
    libusb_context *ctx = NULL;
    libusb_device **devs;
    libusb_device_handle *dev_handle = NULL;
    uint8_t serial_idx;  // 序列号索引
    uint8_t product_idx; // 产品描述符索引
    uint8_t manu_idx;    // 厂商描述符索引
    uint8_t dev_index = 0;
    uint8_t *NA = "N/A";
    uint32_t r = 0;

    ret = libusb_init(&ctx); // 初始化上下文
    if (ret != LIBUSB_SUCCESS) {
        fprintf(stderr, "初始化失败: %s\n", libusb_error_name(ret));
        return -1;
    }

    ssize_t cnt = libusb_get_device_list(ctx, &devs); // 获取设备列表
    if (cnt < 0) {
        fprintf(stderr, "设备枚举失败\n");
        libusb_exit(ctx);
        return -1;
    }

    // 遍历设备，按VID/PID筛选目标设备
    for (int i = 0; i < cnt; i++) {
        libusb_device *dev = devs[i];
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);
        if (desc.idVendor == CH347_VENDOR_ID && (desc.idProduct == CH347T_PRODUCT_ID || desc.idProduct == CH347F_PRODUCT_ID)) {
            serial_idx = desc.iSerialNumber;
            product_idx = desc.iProduct;
            manu_idx = desc.iManufacturer;
            xvcd_dev[dev_index].dev_handle = dev;
            r = libusb_open(xvcd_dev[dev_index].dev_handle, &dev_handle);
            if (r < 0) {
                fprintf(stderr, "打开设备失败: %s\n", libusb_error_name(r));
                libusb_free_device_list(devs, 1); // 释放设备列表
                libusb_exit(ctx);                 // 退出libusb
                break;
            }
            printf("Current index: %d\n", dev_index);
            r = libusb_get_string_descriptor_ascii(dev_handle, manu_idx, (unsigned char *)xvcd_dev[dev_index].usb_manu, sizeof(xvcd_dev[dev_index].usb_manu));
            printf("厂商: %s\n", r > 0 ? xvcd_dev[dev_index].usb_manu : NA);
            r = libusb_get_string_descriptor_ascii(dev_handle, product_idx, (unsigned char *)xvcd_dev[dev_index].usb_product, sizeof(xvcd_dev[dev_index].usb_product));
            printf("产品: %s\n", r > 0 ? xvcd_dev[dev_index].usb_product : NA);
            r = libusb_get_string_descriptor_ascii(dev_handle, serial_idx, (unsigned char *)xvcd_dev[dev_index].usb_serial, sizeof(xvcd_dev[dev_index].usb_serial));
            printf("序列号: %s\n", r > 0 ? xvcd_dev[dev_index].usb_serial : NA);

            xvcd_dev[dev_index].usb_vid = desc.idVendor;
            xvcd_dev[dev_index].usb_pid = desc.idProduct;
            xvcd_dev[dev_index].usb_bcd = desc.bcdDevice;

            if (!strcmp(serialNumber, xvcd_dev[dev_index].usb_serial)) {
                iIndex_ch347 = dev_index;
            }

            dev_index++;
            xvcd_dev[dev_index].count = dev_index;
            libusb_close(dev_handle); // 关闭设备
        }
    }
#endif

    if (io_init(&xvcd_dev[iIndex_ch347])) {
        fprintf(stderr, "io_init failed\n");
        return 1;
    }

    iResult = network_init();
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (s == INVALID_SOCKET_VAL) {
        printf("Error at socket(): %d\n", get_last_error());
        network_cleanup();
        return 1;
    }

    i = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof i);

    memset(&address, 0, sizeof(address));
    address.sin_addr.s_addr = strcmp(szAddress, "") ? inet_addr(szAddress) : INADDR_ANY;
    address.sin_port = htons(iPort);
    address.sin_family = AF_INET;

    iResult = bind(s, (struct sockaddr *)&address, sizeof(address));
    if (iResult == __SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", get_last_error());
        close_socket(s);
        network_cleanup();
        return 1;
    }

    if (listen(s, 1024) == __SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", get_last_error());
        close_socket(s);
        network_cleanup();
        return 1;
    }

    fd_set conn;
    int maxfd = 0;

    FD_ZERO(&conn);
    FD_SET(s, &conn);

    maxfd = s;
    if (verbose) {
        puts("Started");
    }

    while (1) {
        fd_set read = conn, except = conn;
        socket_t fd;
        // Look for work to do.
        if (select(maxfd + 1, &read, 0, &except, 0) == __SOCKET_ERROR) {
            printf("Select failed with error: %d\n", get_last_error());
            break;
        }

        for (fd = 0; fd <= maxfd; ++fd) {
            if (FD_ISSET(fd, &read)) {
                // Readable listen socket? Accept connection.
                if (fd == s) {
                    socket_t newfd;
                    int nsize = sizeof(address);

                    newfd = accept(s, (struct sockaddr *)&address, &nsize);
                    if (verbose) {
                        printf("connection accepted - fd %d\n", newfd);
                    }
                    if (newfd == INVALID_SOCKET_VAL) {
                        printf("accept failed with error: %d\n", get_last_error());
                    } else {
                        puts("setting TCP_NODELAY to 1");
                        int flag = 1;
                        int optResult = setsockopt(newfd,
                                                   IPPROTO_TCP,
                                                   TCP_NODELAY,
                                                   (char *)&flag,
                                                   sizeof(flag));
                        if (optResult < 0) {
                            perror("TCP_NODELAY error");
                        }
                        if (newfd > maxfd) {
                            maxfd = newfd;
                        }
                        FD_SET(newfd, &conn);
                    }
                }
                // Otherwise, do work.
                else {
                    int r;
                    if (verbose) {
                        puts("start handle_data");
                    }
                    r = handle_data(&xvcd_dev[iIndex_ch347], fd, DEFAULT_JTAG_SPEED);
                    if (verbose) {
                        puts("stop handle_data");
                    }
                    if (r) {
                        // Close connection when required.
                        if (verbose)
                            printf("connection closed - fd %d\n", fd);
                        close_socket(fd);
                        FD_CLR(fd, &conn);
                    }
                }
            }
            // Abort connection?
            else if (FD_ISSET(fd, &except)) {
                if (verbose) {
                    printf("connection aborted - fd %d\n", fd);
                }
                close_socket(fd);
                FD_CLR(fd, &conn);
                if (fd == s)
                    break;
            }
        }
    }

    close_socket(s);
    network_cleanup();
    // Un-map IOs.
    io_close(&xvcd_dev[iIndex_ch347]);

    return 0;
}
