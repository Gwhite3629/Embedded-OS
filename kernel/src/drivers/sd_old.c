#include <stdlib.h>
#include <drivers/sd.h>
#include <drivers/io.h>

unsigned long sd_scr[2];
unsigned long sd_ocr;
unsigned long sd_rca;
unsigned long sd_err;
unsigned long sd_hv;

#define TIMEOUT_WAIT(stop_if_true, usec)        \
do {                            \
    int cnt = 0; \
    do {                     \
        cnt++; \
        if(stop_if_true)            \
            break;              \
    } while(cnt < usec);            \
} while(0);

static err_t sd_int(unsigned int mask)
{
    unsigned int r = mask | INT_ERROR_MASK;
    unsigned int m = mask | INT_ERROR_MASK;

    int cnt = 1000000;
    while (!((chip_read(EMMC_INTERRUPT)) & m) && cnt--) {
        udelay(1000);
    }
    r = chip_read(EMMC_INTERRUPT);
    if (cnt <= 0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT)) {
        chip_write(r, EMMC_INTERRUPT);
        return E_TIMEOUT;
    } else if (r & INT_ERROR_MASK) {
        chip_write(r, EMMC_INTERRUPT);
        return E_NOT_READY;
    }
    chip_write(mask, EMMC_INTERRUPT);
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

    while (((chip_read(EMMC_STATUS)) & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && cnt--) {
        udelay(1000);
    }
    
    if (cnt <= 0) {
        return E_TIMEOUT;
    }

    chip_write(chip_read(EMMC_CONTROL1) & ~C1_CLK_EN, EMMC_CONTROL1);
    udelay(10000);
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
    chip_write((chip_read(EMMC_CONTROL1) & 0xFFFF003f) | d, EMMC_CONTROL1);
    udelay(10000);
    chip_write(chip_read(EMMC_CONTROL1) | C1_CLK_EN, EMMC_CONTROL1);
    udelay(10000);
    cnt = 10000;
    while (!(chip_read(EMMC_CONTROL1) & C1_CLK_STABLE) && cnt--) {
        udelay(10000);
    }
    if (cnt <= 0) {
        return E_NOT_READY;
    }

    return E_NOERR;
}

err_t sd_ioctl(unsigned int cmd, unsigned int arg)
{
    int r = 0;
    sd_err = E_NOERR;
    if (cmd & CMD_NEED_APP) {
        r = sd_ioctl(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48:0), sd_rca);
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
    (*(unsigned int *)EMMC_ARG1) = arg;
    (*(unsigned int *)EMMC_CMDTM) = cmd;
    
    if (cmd == CMD_SEND_OP_COND) udelay(1000000);
    else if (cmd == CMD_SEND_IF_COND || cmd == CMD_APP_CMD) udelay(100000);

    if ((r = sd_int(INT_CMD_DONE))) {
        sd_err = r;
        return E_NOERR;
    }
    r = chip_read(EMMC_RESP0);
    
    if (cmd == CMD_GO_IDLE || cmd == CMD_APP_CMD) return E_NOERR;
    else if (cmd == (CMD_APP_CMD | CMD_RSPNS_48)) return r & SR_APP_CMD;
    else if (cmd == CMD_SEND_OP_COND) return r;
    else if (cmd == CMD_SEND_IF_COND) return r == arg ? E_NOERR : E_NOT_READY;
    else if (cmd == CMD_ALL_SEND_CID) {
        r |= chip_read(EMMC_RESP3);
        r |= chip_read(EMMC_RESP2);
        r |= chip_read(EMMC_RESP1);
        return r;
    } else if (cmd == CMD_SEND_REL_ADDR) {
        sd_err = (((r&0x1FFF))|((r&0x2000)<<6)|((r&0x4000)<<8)) & CMD_ERRORS_MASK;
        return r & CMD_RCA_MASK;
    }
    return r & CMD_ERRORS_MASK;
}

static err_t sd_cmd(unsigned int cmd, unsigned int arg) {
    if (cmd & CMD_NEED_APP) {
        cmd &= 0xff;
    }

    while(chip_read(EMMC_STATUS) & 0x1) {
        udelay(1000);
    }

    if ((cmd & (3 << 16)) == (3 << 16)) {
        if ((cmd & (3 << 22)) != (3 << 22)) {
            while(chip_read(EMMC_STATUS) & 0x2) {
                udelay(1000);
            }
        }
    }

    chip_write(arg, EMMC_ARG1);
    chip_write(cmd, EMMC_CMDTM);

    udelay(2000);
    TIMEOUT_WAIT(chip_read(EMMC_INTERRUPT) & 0x8001, 500000);
    chip_write(0xffff0001, EMMC_INTERRUPT);
    udelay(2000);

    return 0;
}
/*
err_t sd_init(void)
{
    long r = 0;
    long cnt = 0;
    long ccs = 0;

    // GPIO_CD

    r = chip_read(GPIO_GPFSEL4);
    r &= ~(7 << (7 * 3));
    chip_write(r, GPIO_GPFSEL4);

    chip_write(2, GPIO_GPPUD);
    udelay(150000);
    chip_write((1 << 15), GPIO_GPPUDCLK1);
    udelay(150000);
    chip_write(0, GPIO_GPPUD);
    chip_write(0, GPIO_GPPUDCLK1);

    r = chip_read(GPIO_GPHEN1);
    r |= 1 << 15;
    chip_write(r, GPIO_GPHEN1);

    // GPIO_CLK, GPIO_CMD

    r = chip_read(GPIO_GPFSEL4);
    r |= (7 << (8 * 3)) | (7 << (9 * 3));
    chip_write(r, GPIO_GPFSEL4);

    chip_write(2, GPIO_GPPUD);
    udelay(150000);
    chip_write((1 << 16) | (1 << 17), GPIO_GPPUDCLK1);
    udelay(150000);
    chip_write(0, GPIO_GPPUD);
    chip_write(0, GPIO_GPPUDCLK1);

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = chip_read(GPIO_GPFSEL5);
    r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
    chip_write(r, GPIO_GPFSEL5);

    chip_write(2, GPIO_GPPUD);
    udelay(150000);
    chip_write((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21), GPIO_GPPUDCLK1);
    udelay(150000);
    chip_write(0, GPIO_GPPUD);
    chip_write(0, GPIO_GPPUDCLK1);

    sd_hv = (chip_read(EMMC_SLOTISR_VER) & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    // Reset card
    chip_write(0, EMMC_CONTROL0);
    chip_write(chip_read(EMMC_CONTROL1) | C1_SRST_HC, EMMC_CONTROL1);
    cnt = 10000;
    do {
        udelay(10000);
    } while (((chip_read(EMMC_CONTROL1)) & C1_SRST_HC) && cnt--);
    if (cnt <= 0) {
        printk("\x1b[31mSD TIMEOUT -> emmc_ctrl1\n\x1b[0m");
        return E_TIMEOUT;
    }
    chip_write(chip_read(EMMC_CONTROL1) | C1_CLK_INTLEN | C1_TOUNIT_MAX, EMMC_CONTROL1);
    udelay(10000);

    // Set Frequency
    if ((r = sd_clk(400000)) != E_NOERR) {
        printk("\x1b[31mSD_ERR: %d -> sd_clk(low)\n\x1b[0m", r);
        return r;
    }
    chip_write(0xffffffff, EMMC_INT_EN);
    chip_write(0xffffffff, EMMC_INT_MASK);
    sd_scr[0] = 0;
    sd_scr[1] = 0;
    sd_rca = 0;
    sd_err = 0;
    sd_cmd(CMD_GO_IDLE, 0);
    if (sd_err != E_NOERR) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(idle)\n\x1b[0m", sd_err);
        return sd_err;
    }

    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(IF_COND)\n\x1b[0m", sd_err);
        return sd_err;
    }
    cnt = 6;
    r = 0;
    while (!(r & ACMD41_CMD_COMPLETE) && cnt--) {
        udelay(400000);
        r = sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        if (sd_err != E_TIMEOUT && sd_err != E_NOERR) {
            printk("\x1b[31mSD_ERR: %d -> sd_cmd(OP_COND)\n\x1b[0m", sd_err);
            return sd_err;
        }
    }
    if (!(r & ACMD41_CMD_COMPLETE) || !cnt) {
        printk("\x1b[31mSD TIMEOUT -> acmd\n\x1b[0m");
        return E_TIMEOUT;
    }
    if (!(r & ACMD41_VOLTAGE)) {
        printk("\x1b[31mSD NOT READY\n\x1b[0m");
        return E_NOT_READY;
    }
    if (r & ACMD41_CMD_CCS) ccs = SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID, 0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR, 0);
    if (sd_err != E_NOERR) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(REL_ADDR)\n\x1b[0m", sd_err);
        return sd_err;
    }

    if ((r = sd_clk(25000000)) != E_NOERR) {
        printk("\x1b[31mSD_ERR: %d -> sd_clk(high)\n\x1b[0m", r);
        return r;
    }

    sd_cmd(CMD_CARD_SELECT, sd_rca);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(select)\n\x1b[0m", sd_err);
        return sd_err;
    }
    if (sd_status(SR_DAT_INHIBIT) != E_NOERR) {
        printk("\x1b[31mSD TIMEOUT -> inhibit\n\x1b[0m");
        return E_TIMEOUT;
    }
    chip_write((1 << 16) | 8, EMMC_BLKSIZECNT);
    sd_cmd(CMD_SEND_SCR, 0);
    if (sd_err != E_NOERR) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(scr)\n\x1b[0m", sd_err);
        return sd_err;
    }
    if (sd_int(INT_READ_RDY) != E_NOERR) {
        printk("\x1b[31mSD TIMEOUT -> rd_rdy\n\x1b[0m");
        return E_TIMEOUT;
    }

    r = 0;
    cnt = 100000;
    while (r < 2 && cnt) {
        if ((chip_read(EMMC_STATUS)) & SR_READ_AVAILABLE)
            sd_scr[r++] = (*(unsigned int *)EMMC_DATA);
        else 
            udelay(1000);
    }

    if (r != 2) {
        printk("\x1b[31mSD TIMEOUT -> status\n\x1b[0m");
        return E_TIMEOUT;
    }
    if (sd_scr[0] & SCR_SD_BUS_WIDTH) {
        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca | 2);
        if (sd_err != E_NOERR) {
            printk("\x1b[31mSD_ERR: %d -> sd_cmd(bus_width)\n\x1b[0m", sd_err);
            return sd_err;
        }
        chip_write(chip_read(EMMC_CONTROL0) | C0_HCTL_DWIDTH, EMMC_CONTROL0);
    }

    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;

    return E_NOERR;
}
*/

static uint32_t sd_get_clock_divider(uint32_t base_clock, uint32_t target_rate)
{
    // TODO: implement use of preset value registers

    uint32_t targetted_divisor = 0;
    if(target_rate > base_clock)
        targetted_divisor = 1;
    else
    {
        targetted_divisor = base_clock / target_rate;
        uint32_t mod = base_clock % target_rate;
        if(mod)
            targetted_divisor--;
    }

    // Decide on the clock mode to use

    // Currently only 10-bit divided clock mode is supported

        // HCI version 3 or greater supports 10-bit divided clock mode
        // This requires a power-of-two divider

        // Find the first bit set
        int divisor = -1;
        for(int first_bit = 31; first_bit >= 0; first_bit--)
        {
            uint32_t bit_test = (1 << first_bit);
            if(targetted_divisor & bit_test)
            {
                divisor = first_bit;
                targetted_divisor &= ~bit_test;
                if(targetted_divisor)
                {
                    // The divisor is not a power-of-two, increase it
                    divisor++;
                }
                break;
            }
        }

        if(divisor == -1)
            divisor = 31;
        if(divisor >= 32)
            divisor = 31;

        if(divisor != 0)
            divisor = (1 << (divisor - 1));

        if(divisor >= 0x400)
            divisor = 0x3ff;

        uint32_t freq_select = divisor & 0xff;
        uint32_t upper_bits = (divisor >> 8) & 0x3;
        uint32_t ret = (freq_select << 8) | (upper_bits << 6) | (0 << 5);

        return ret;
}

err_t sd_init(void)
{
    uint32_t control1 = chip_read(EMMC_CONTROL1);
    control1 |= (1 << 24);
    control1 &= ~(1 << 2);
    control1 &= ~(1 << 0);

    // Reset control1
    chip_write(control1, EMMC_CONTROL1);
    TIMEOUT_WAIT((chip_read(EMMC_CONTROL1) & (0x7 << 24)) == 0, 1000000);
    if ((chip_read(EMMC_CONTROL1) & (0x7 << 24)) != 0) {
        printk("\x1b[31mSD TIMEOUT -> reset\n\x1b[0m");
        return E_TIMEOUT;
    }

    uint32_t control0 = chip_read(EMMC_CONTROL0);
    control0 |= 0x0f << 8;
    chip_write(control0, EMMC_CONTROL0);
    udelay(2000);

    // Check for card
    TIMEOUT_WAIT(chip_read(EMMC_STATUS) & (1 << 16), 500000);
    uint32_t status_reg = chip_read(EMMC_STATUS);
    if ((status_reg & (1 << 16)) == 0) {
        printk("\x1b[31mSD NO CARD -> status\n\x1b[0m");
        return E_NOCARD;
    }

    // Clear control2
    chip_write(0, EMMC_CONTROL2);
    uint32_t capabilities_0 = chip_read(EMMC_CAPABILITIES_0);
    uint32_t base_clock = ((capabilities_0 >> 8) & 0xff) * 1000000;
    if (base_clock == 0) {
        base_clock = 100000000;
    }

    // Set clock to slow
    control1 = chip_read(EMMC_CONTROL1);
    control1 |= 1;

    uint32_t f_id = sd_get_clock_divider(base_clock, 400000);
    control1 &= ~(0xf << 16);
    control1 |= f_id;
    //control1 |= (7 << 16);
    control1 &= ~(0x3ff << 6);
    control1 |= (11 << 16);

    chip_write(control1, EMMC_CONTROL1);
    TIMEOUT_WAIT(chip_read(EMMC_CONTROL1) & 0x2, 1000000);
    if ((chip_read(EMMC_CONTROL1) & 0x2) == 0) {
        printk("\x1b[31mSD TIMEOUT -> set slow clock\n\x1b[0m");
        return E_TIMEOUT;
    }

    // Enable SD clock
    udelay(2000);
    control1 = chip_read(EMMC_CONTROL1);
    control1 |= 4;
    chip_write(control1, EMMC_CONTROL1);
    udelay(2000);

    // Mask off interrupts
    chip_write(0, EMMC_INT_EN);
    chip_write(0xffffffff, EMMC_INTERRUPT);
    chip_write(0xffffffff & (~(1<<8)), EMMC_INT_MASK);

    udelay(2000);

    // Set to idle
    sd_cmd(CMD_GO_IDLE, 0);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(idle)\n\x1b[0m", sd_err);
        return sd_err;
    }

    // Send CMD8
    sd_cmd(CMD_SEND_IF_COND, 0x1aa);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(if_cond)\n\x1b[0m", sd_err);
        return sd_err;
    }

    // ACMD41 inquiry
    sd_cmd(ACMD41_CMD_CCS, 0);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(ACMD41)\n\x1b[0m", sd_err);
        return sd_err;
    }
    
    sd_cmd(ACMD41_CMD_CCS, 0x00ff8000 | ((1 << 30) | (1 << 24) | (1 << 28)));
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(SET ACMD)\n\x1b[0m", sd_err);
        return sd_err;
    }

    sd_clk(25000000);
    udelay(5000);

    // Disable SD clock
    control1 = chip_read(EMMC_CONTROL1);
    control1 &= ~(1 << 2);
    chip_write(control1, EMMC_CONTROL1);

    udelay(5000);

    // Enable SD clock
    control1 = chip_read(EMMC_CONTROL1);
    control1 |= (1 << 2);
    chip_write(control1, EMMC_CONTROL1);

    udelay(10000);

    sd_cmd(CMD_ALL_SEND_CID, 0);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(CID)\n\x1b[0m", sd_err);
        return sd_err;
    }

    sd_cmd(CMD_SEND_REL_ADDR, 0);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(rel_addr)\n\x1b[0m", sd_err);
        return sd_err;
    }
    sd_rca = (chip_read(EMMC_RESP0) >> 16) & 0xffff;

    sd_cmd(CMD_CARD_SELECT, sd_rca << 16);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(select)\n\x1b[0m", sd_err);
        return sd_err;
    }

    uint32_t controller_block_size = chip_read(EMMC_BLKSIZECNT);
    controller_block_size &= (~0xfff);
    controller_block_size |= 0x200;
    chip_write(controller_block_size, EMMC_BLKSIZECNT);

    sd_cmd(CMD_SEND_SCR, 0);
    if (sd_err) {
        printk("\x1b[31mSD_ERR: %d -> sd_cmd(scr)\n\x1b[0m", sd_err);
        return sd_err;
    }
    sd_scr[0] = chip_read(EMMC_RESP1);
    sd_scr[1] = chip_read(EMMC_RESP0);

    chip_write(0xffffffff, EMMC_INTERRUPT);

    return 0;
}

err_t sd_status(unsigned int mask)
{
    int cnt = 1000000;
    while (((chip_read(EMMC_STATUS)) & mask) && !((chip_read(EMMC_INTERRUPT)) & INT_ERROR_MASK) && cnt--) {
        udelay(1000);
    }
    return (cnt <= 0 || ((chip_read(EMMC_INTERRUPT)) & INT_ERROR_MASK)) ? E_NOT_READY : E_NOERR;
}

err_t sd_read(char *buff, uint16_t sector, size_t n)
{
    int r;
    int c = 0;
    int d;

    if(n<1)
        n=1;

    if(sd_status(SR_DAT_INHIBIT)) {
        sd_err = E_TIMEOUT;
        printk("\x1b[31mSD_ERR: %d -> sd_status(inhibit)\n\x1b[0m", sd_err);
        return 0;
    }
    unsigned int *buf = (unsigned int *) buff;
    if(sd_scr[0] & SCR_SUPP_CCS) {
        if(n > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD_SET_BLOCKCNT,n);
            if(sd_err) {
                printk("\x1b[31mSD_ERR: %d -> sd_cmd(blockcnt)\n\x1b[0m", sd_err);
                return 0;
            }
        }
        chip_write((n << 16) | 512, EMMC_BLKSIZECNT);
        sd_cmd(n == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, sector);
        if(sd_err) {
            printk("\x1b[31mSD_ERR: %d -> sd_cmd(read)\n\x1b[0m", sd_err);
            return 0;
        }
    } else {
        chip_write((1 << 16) | 512, EMMC_BLKSIZECNT);
    }
    while( c < n ) {
        if(!(sd_scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD_READ_SINGLE, (sector + c) * 512);
            if(sd_err) {
                printk("\x1b[31mSD_ERR: %d -> sd_cmd(read)\n\x1b[0m", sd_err);
                return 0;
            }
        }
        if((r = sd_int(INT_READ_RDY))) {
            printk("\x1b[31mSD TIMEOUT\n\x1b[0m");
            sd_err = r;
            return 0;
        }
        for(d=0;d<128;d++) {
            buf[d] = chip_read(EMMC_DATA);
        }
        c++;
        buf += 128;
    }
    if(n > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS))
        sd_cmd(CMD_STOP_TRANS,0);
    return sd_err != E_NOERR || c != n ? 0 : n * 512;
}

err_t sd_write(const char *buff, uint16_t sector, size_t n)
{
    int r;
    int c = 0;
    int d;
    if(n<1)
        n=1;
    
    if(sd_status(SR_DAT_INHIBIT | SR_WRITE_AVAILABLE)) {
        sd_err = E_TIMEOUT;
        printk("\x1b[31mSD_ERR: %d -> sd_status(inhibit)\n\x1b[0m", sd_err);
        return 0;
    }
    unsigned int *buf = (unsigned int *)buff;
    if(sd_scr[0] & SCR_SUPP_CCS) {
        if(n > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD_SET_BLOCKCNT, n);
            if(sd_err) {
                printk("\x1b[31mSD_ERR: %d -> sd_cmd(blockcnt)\n\x1b[0m", sd_err);
                return 0;
            }
        }
        chip_write((n << 16) | 512,EMMC_BLKSIZECNT);
        sd_cmd(n == 1 ? CMD_WRITE_SINGLE : CMD_WRITE_MULTI, sector);
        if(sd_err) {
            printk("\x1b[31mSD_ERR: %d -> sd_cmd(write)\n\x1b[0m", sd_err);
            return 0;
        }
    } else {
        chip_write((1 << 16) | 512,EMMC_BLKSIZECNT);
    }
    while(c < n) {
        if(!(sd_scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD_WRITE_SINGLE,(sector + c) * 512);
            if(sd_err) {
                printk("\x1b[31mSD_ERR: %d -> sd_cmd(write)\n\x1b[0m", sd_err);
                return 0;
            }
        }
        if((r=sd_int(INT_WRITE_RDY))){
            sd_err = r;
            printk("\x1b[31mSD_ERR: %d -> sd_int(rdy)\n\x1b[0m", sd_err);
            return 0;
        }
        for(d=0;d<128;d++) {
            chip_write(buf[d], EMMC_DATA);
        }
        c++;
        buf += 128;
    }
    if((r = sd_int(INT_DATA_DONE))){
        sd_err = r;
        printk("\x1b[31mSD_ERR: %d -> sd_int(data_done)\n\x1b[0m", sd_err);
        return 0;
    }
    if(n > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) 
        sd_cmd(CMD_STOP_TRANS, 0);
    return sd_err != E_NOERR || c != n ? 0 : n * 512;
}
