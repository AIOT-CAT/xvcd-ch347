// xilinx virtual cable demon for jlink jtag
// use connection string in Impact and another
// xilinx_xvc host=localhost:2542 disableversioncheck=true

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <time.h>

#include "io_ch347.h"

#define VECTOR_IN_SZ 2048

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

/*Listen on port 2542.*/
#define DEFAULT_PORT       2542
int DEFAULT_JTAG_SPEED = 30000000; // 30 Mhz

static int jtag_state;
static int verbose = 0;

static int vlevel = 1;
unsigned long iIndex_CH347 = 0;

/*JTAG state machine.*/
enum {
    test_logic_reset,
    run_test_idle,

    select_dr_scan,
    capture_dr,
    shift_dr,
    exit1_dr,
    pause_dr,
    exit2_dr,
    update_dr,

    select_ir_scan,
    capture_ir,
    shift_ir,
    exit1_ir,
    pause_ir,
    exit2_ir,
    update_ir,

    num_states
};

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

static int sread(SOCKET fd, void *target, int len)
{
    char *t = target;
    while (len) {
        int r = recv(fd, t, len, 0);
        if (r == SOCKET_ERROR || r == 0) {
            int err = WSAGetLastError();
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

static int swrite(SOCKET fd, void *target, int len)
{
    char *t = target;
    while (len) {
        int r = send(fd, t, len, 0);
        if (r == SOCKET_ERROR || r == 0) {
            int err = WSAGetLastError();
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
	data[0] = num & 0x00ff; num >>= 8;
	data[1] = num & 0x00ff; num >>= 8;
	data[2] = num & 0x00ff; num >>= 8;
	data[3] = num & 0x00ff; num >>= 8;

}

//
//   handle_data(fd) handles JTAG shift instructions.
//   To allow multiple programs to access the JTAG chain
//   at the same time, we only allow switching between
//   different clients only when we're in run_test_idle
//   after going test_logic_reset. This ensures that one
//   client can't disrupt the other client's IR or state.
//
int handle_data(SOCKET fd, unsigned long frequency)
{
    int i;
    int seen_tlr = 0;
    int retVal = 0;
    const char xvcInfo[] = "xvcServer_v1.0:" TOSTRING(VECTOR_IN_SZ) "\n";
    do {
        char cmd[16];
        unsigned char buffer[2048], result[2048];

        if (sread(fd, cmd, 2) != 1)
            return 1;

        if (memcmp(cmd, "ge", 2) == 0) {
            if (sread(fd, cmd, 6) != 1)
                return 1;
            memcpy(result, xvcInfo, strlen(xvcInfo));
            
            if (swrite(fd, result, strlen(xvcInfo)) != 1) {
                printf("[%d]write %d bytes != %d\n", __LINE__, retVal, strlen(xvcInfo));
                return 1;
            }
            if (vlevel > 0) {
                printf("%u : Received command: 'getinfo'\n", (int)time(NULL));
                printf("\t Replied with %s\n", xvcInfo);
            }
            break;
        } else if (memcmp(cmd, "se", 2) == 0) {
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

            actPeriod = io_set_period(iIndex_CH347, (unsigned int)period);

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
            if (sread(fd, cmd, 4) != 1)
                return 1;
            if (vlevel > 1) {
                printf("%u : Received command: 'shift'\n", (int)time(NULL));
            }
        } else {

            fprintf(stderr, "invalid cmd '%s'\n", cmd);
            return 1;
        }

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
        memset(result, 0, nr_bytes);
        if (vlevel > 2) {
            printf("\tNumber of Bits  : %d\n", len);
            printf("\tNumber of Bytes : %d \n", nr_bytes);
            for (i=0; i < nr_bytes; i++)
                printf("%02x ", buffer[i]);
            printf("\n");
        }

        //
        // Only allow exiting if the state is rti and the IR
        // has the default value (IDCODE) by going through test_logic_reset.
        // As soon as going through capture_dr or capture_ir no exit is
        // allowed as this will change DR/IR.
        //
        seen_tlr = (seen_tlr || jtag_state == test_logic_reset) && (jtag_state != capture_dr) && (jtag_state != capture_ir);

        //
        // Due to a weird bug(??) xilinx impacts goes through another "capture_ir"/"capture_dr" cycle after
        // reading IR/DR which unfortunately sets IR to the read-out IR value.
        // Just ignore these transactions.
        //
        if ((jtag_state == exit1_ir && len == 5 && buffer[0] == 0x17) ||
            (jtag_state == exit1_dr && len == 4 && buffer[0] == 0x0b)) {
            if (verbose)
                printf("ignoring bogus jtag state movement in jtag_state %d\n", jtag_state);
        } else {
            for (i = 0; i < len; ++i) {
                //
                // Do the actual cycle.
                //
                int tms = !!(buffer[i / 8] & (1 << (i & 7)));

                //
                // Track the state.
                //
                jtag_state = jtag_step(jtag_state, tms);
            }
            
            if (io_scan(buffer, buffer + nr_bytes, result, len) < 0) {
                fprintf(stderr, "io_scan failed\n");
                exit(1);
            }

            if (verbose) {
				for (i = 0; i < nr_bytes; ++i)
					printf("%02x ", result[i]);
            }
        }

        if (send(fd, (const char *)result, nr_bytes, 0) != nr_bytes) {
            printf("Send failed with error: %d\n", WSAGetLastError());
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

int main(int argc, char **argv)
{
    int ret = 0;
    int i;
    SOCKET s;
    int c;
    struct sockaddr_in address;
    unsigned short port = DEFAULT_PORT;
    unsigned short jtagSpeed = DEFAULT_JTAG_SPEED;
    unsigned int coreId = 0;

    opterr = 0;

    if (io_init(iIndex_CH347)) {
        fprintf(stderr, "io_init failed\n");
        return 1;
    }

    WSADATA wsaData;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    i = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof i);

    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    address.sin_family = AF_INET;

    iResult = bind(s, (struct sockaddr *)&address, sizeof(address));

    if (iResult == SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
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
        SOCKET fd;

        // Look for work to do.
        if (select(maxfd + 1, &read, 0, &except, 0) == SOCKET_ERROR) {
            printf("Select failed with error: %d\n", WSAGetLastError());
            break;
        }

        for (fd = 0; fd <= maxfd; ++fd) {
            if (FD_ISSET(fd, &read)) {
                // Readable listen socket? Accept connection.
                if (fd == s) {
                    SOCKET newfd;
                    int nsize = sizeof(address);

                    newfd = accept(s, (struct sockaddr *)&address, &nsize);
                    if (verbose) {
                        printf("connection accepted - fd %d\n", newfd);
                    }
                    if (newfd == INVALID_SOCKET) {
                        printf("accept failed with error: %d\n", WSAGetLastError());
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
                    r = handle_data(fd, DEFAULT_JTAG_SPEED);
                    if (verbose) {
                        puts("stop handle_data");
                    }
                    if (r) {
                        // Close connection when required.
                        if (verbose)
                            printf("connection closed - fd %d\n", fd);
                        closesocket(fd);
                        FD_CLR(fd, &conn);
                    }
                }
            }
            // Abort connection?
            else if (FD_ISSET(fd, &except)) {
                if (verbose) {
                    printf("connection aborted - fd %d\n", fd);
                }
                closesocket(fd);
                FD_CLR(fd, &conn);
                if (fd == s)
                    break;
            }
        }
    }
    closesocket(s);
    WSACleanup();
    // Un-map IOs.
    io_close();

    return 0;
}
