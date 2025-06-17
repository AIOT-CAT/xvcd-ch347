#ifndef _IO_CH347_H
#define _IO_CH347_H

#include "ch347_xvcd_api.h"

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

int io_init(dev_ctx *ch347_ctx);
int io_set_period(dev_ctx *ch347_ctx, unsigned int period);
int io_scan(dev_ctx *ch347_ctx, const unsigned char* tms, const unsigned char* tdi, unsigned char* tdo, int len, int state);
void io_close(dev_ctx *ch347_ctx);

int usb_xfer(dev_ctx *ch347_ctx, unsigned wlen, unsigned rlen, unsigned *ract, bool defer);


#endif