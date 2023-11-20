int io_init(unsigned int index);
int io_set_period(unsigned int index, unsigned int period);
int io_scan(const unsigned char *TMS, const unsigned char *TDI, unsigned char *TDO, int bits);
void io_close(void);