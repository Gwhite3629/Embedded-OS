#include "../../include/stdlib.h"
#include "../../include/drivers/sd.h"

unsigned long sd_scr[2];
unsigned long sd_ocr;
unsigned long sd_rca;
unsigned long sd_err;
unsigned long sd_hv;

err_t sd_init(void)
{
    long r = 0;
    long cnt = 0;
    long ccs = 0;

    // GPIO_CD

    r = (*GPFSEL4);
    r &= ~(7 << (7 * 3));
    (*GPFSEL4) = r;

    (*GPPUD) = 2;
    wait_cycles(150);
    (*GPPUDCLK1) = (1 << 15);
    wait_cycles(150);
    (*GPPUD) = 0;
    (*GPPUDCLK1) = 0;

    r = (*GPHEN1);
    r |= 1 << 15;
    (*GPHEN1) = r;

    // GPIO_CLK, GPIO_CMD

    r = (*GPFSEL4);
    r |= (7 << (8 * 3)) | (7 << (9 * 3));
    (*GPFSEL4) = r;

    (*GPPUD) = 2;
    wait_cycles(150);
    (*GPPUDCLK1) = (1 << 16) | (1 << 17);
    wait_cycles(150);
    (*GPPUD) = 0;
    (*GPPUDCLK1) = 0;

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = (*GPFSEL5);
    r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
    (*GPFSEL5) = r;

    (*GPPUD) = 2;
    wait_cycles(150);
    (*GPPUDCLK1) = (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    wait_cycles(150);
    (*GPPUD) = 0;
    (*GPPUDCLK1) = 0;

    sd_hv = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    // Reset card
    (*EMMC_CONTROL0) = 0;
    (*EMMC_CONTROL1) |= C1_SRST_HC;
    cnt = 10000;
    do {
        wait_msec(10);
    } while (((*EMMC_CONTROL1) & C1_SRST_HC) && cnt--);
    if (cnt <= 0) {
        return E_TIMEOUT;
    }
    (*EMMC_CONTROL1) |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    wait_msec(10);

    // Set Frequency
    if ((r = sd_clk(400000)) != E_NOERR) return r;
    (*EMMC_INT_EN) = 0xffffffff;
    (*EMMC_INT_MASK) = 0xffffffff;
    sd_scr[0] = 0;
    sd_scr[1] = 0;
    sd_rca = 0;
    sd_err = 0;
    sd_cmd(CMD_GO_IDLE, 0);
    if (sd_err != E_NOERR) return sd_err;

    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
    if (sd_err) return sd_err;
    cnt = 6;
    r = 0;
    while (!(r & ACMD41_CMD_COMPLETE) && cnt--) {
        wait_cycles(400);
        r = sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        if (sd_err != E_TIMEOUT && sd_err != E_NOERR) {
            return sd_err;
        }
    }
    if (!(r & ACMD41_CMD_COMPLETE) || !cnt) return E_TIMEOUT;
    if (!(r & ACMD41_VOLTAGE)) return E_NOT_READY;
    if (r & ACMD41_CMD_CCS) ccs = SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID, 0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR, 0);
    if (sd_err != E_NOERR) return sd_err;

    if ((r = sd_clk(25000000)) != E_NOERR) return r;

    sd_cmd(CMD_CARD_SELECT, sd_rca);
    if (sd_err) return sd_err;
    if (sd_status(SR_DAT_INHIBIT) != E_NOERR) return E_TIMEOUT;
    (*EMMC_BLKSIZECNT) = (1 << 16) | 8;
    sd_cmd(CMD_SEND_SCR, 0);
    if (sd_err != E_NOERR) return sd_err;
    if (sd_int(INT_READ_RDY) != E_NOERR) return E_TIMEOUT;

    r = 0;
    cnt = 100000;
    while (r < 2 && cnt) {
        if ((*EMMC_STATUS) & SR_READ_AVAILABLE)
            sd_scr[r++] = (*EMMC_DATA);
        else 
            wait_msec(1)
    }

    if (r != 2) return E_TIMEOUT;
    if (sd_src[0] & SCR_SD_BUS_WIDTH_4) {
        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca | 2);
        if (sd_err != E_NOERR) return sd_err;
        (*EMMC_CONTROL0) |= C0_HCTL_DWIDTH;
    }

    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;

    return E_NOERR;
}

static err_t sd_clk(unsigned int f)
{
    unsigned int d = 41666666/f;
    unsigned int c = 41666666/f;
    unsigned int x = 32;
    unsigned int s = 32;
    unsigned int h = 0;

    int cnt = 100000;

    while (((*EMMC_STATUS) & (SR_CMD_INHIBIT | SR_DATA_INHIBIT)) && cnt--) {
        wait_msec(1);
    }
    
    if (cnt <= 0) {
        return SD_TIMEOUT;
    }

    (*EMMC_CONTROL1) &= ~C1_CLK_EN;
    wait_msec(10);
    x = c - 1;
    if (!x) 
        s = 0;
    else {
        if (!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if (!(x & 0xff000000u)) { x <<= 8; s -= 8; }
        if (!(x & 0xf0000000u)) { x <<= 4; x -= 4; }
        if (!(x & 0xC0000000u)) { x <<= 2; x -= 2; }
        if (!(x & 0x80000000u)) { x <<= 1; x -= 1; }
        if (s > 0) s--;
        if (s > 7) s = 7;
    }
    if (sd_hv > HOST_SPEC_V2)
        d = c;
    else 
        d = (1 << s);
    if (d <= 2) {
        d = 2;
        s = 0;
    }
    if (sd_hv > HOST_SPEC_V2)
        h = (d & 0x300) >> 2;
    d = (((d & 0x0FF) << 8) | h);
    (*EMMC_CONTROL1) = (*EMMC_CONTROL1 & 0xFFFF003f) | d;
    wait_msec(10);
    (*EMMC_CONTROL1) |= C1_CLK_EN;
    wait_msec(10);
    cnt = 10000;
    while (!(*EMMC_CONTROL1 & C1_CLK_STABLE) && cnt--) {
        wait_msec(10);
    }
    if (cnt <= 0) {
        return E_NOT_READY;
    }

    return E_NOERR;
}

err_t sd_status(unsigned int mask)
{
    int cnt = 1000000;
    while (((*EMMC_STATUS) & mask) && !((*EMMC_INTERRUPT) & INT_ERROR_MASK) && cnt--) {
        wait_msec(1);
    }
    return (cnt <= 0 || ((*EMMC_INTERRUPT) * INT_ERROR_MASK)) ? E_NOT_READY : E_NOERR;
}

static err_t sd_int(unsigned int mask)
{
    unsigned int r = mask | INT_ERROR_MASK;
    unsigned int m = mask | INT_ERROR_MASK;

    int cnt = 1000000;
    while (!((*EMMC_INTERRUPT) & m) && cnt--) {
        wait_msec(1);
    }
    r = (*EMMC_INTERRUPT);
    if (cnt <= 0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT)) {
        (*EMMC_INTERRUPT) = r;
        return E_TIMEOUT;
    } else if (r & INT_ERROR_MASK) {
        (*EMMC_INTERRUPT) = r;
        return E_NOT_READY;
    }
    (*EMMC_INTERRUPT) = mask;
    return E_NOERR;
}

err_t sd_read(char *buff, uint16_t sector, size_t n)
{

}

err_t sd_write(const char *buff, uint16_t sector, size_t n)
{

}

static err_t sd_cmd(unsigned int cmd, unsigned int arg) {
    return sd_ioctl(cmd, arg);
}

err_t sd_ioctl(unsigned int cmd, unsigned int arg)
{
    int r = 0;
    sd_err = E_NOERR;
    if (cmd & CMD_NEED_APP) {
        r = sd_cmd(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48:0), sd_rca);
        if (sd_rca && !r) {
            sd_err = E_NOT_READY;
            return E_NOERR;
        }
        cmd &= ~CMD_NEED_APP;
    }
    if (sd_status(SR_CMD_INHIBIT)) {
        sd_err = E_TIMEOUT;
        return 0;
    }
    (*EMMC_ARG) = arg;
    (*EMMC_CMDTM) = cmd;
    
    if (cmd == CMD_SEND_OP_COND) wait_msec(1000);
    else if (cmd == CMD_SEND_IF_COND || cmd == CMD_APP_CMD) wait_msec(100);

    if ((r = sd_int(INT_CMD_DONE))) {
        sd_err = r;
        return E_NOERR;
    }
    r = (*EMMC_RESP0);
    
    if (cmd == CMD_GO_IDLE || cmd == CMD_APP_CMD) return E_NOERR;
    else if (cmd == (CMD_APP_CMD | CMD_RSPNS_48)) return r & SR_APP_CMD;
    else if (cmd == CMD_SEND_OP_COND) return r;
    else if (cmd == CMD_SEND_IF_COND) return r == arg ? E_NOERR : E_NOT_READY;
    else if (cmd == CMD_ALL_SEND_CID) {
        r |= (*EMMC_RESP3);
        r |= (*EMMC_RESP2);
        r |= (*EMMC_RESP1);
        return r;
    } else if (cmd == CMD_SEND_REL_ADDR) {
        sd_arr = (((r&0x1FFF))|((r&0x2000)<<6)|((r&0x4000)<<8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    return r & CMD_ERRORS_MASK;
}
