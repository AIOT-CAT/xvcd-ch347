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

int io_init(unsigned int index);
int io_set_period(unsigned int index, unsigned int period);
int io_scan(const unsigned char* tms, const unsigned char* tdi, unsigned char* tdo, int len, int state);
void io_close(void);