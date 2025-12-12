#include <system.h>
#include <io.h>


enum {
    ATA_PRIMARY_BASE = 0x1F0,
    ATA_SECONDARY_BASE = 0x170,
    ATA_PRIMARY_CTRL = 0x3F6,
    ATA_SECONDARY_CTRL = 0x376
};

#define REG_DATA        0   /* data (R/W) (16-bit) */
#define REG_ERROR       1   /* (R)  error */
#define REG_FEATURES    1   /* (W)  features */
#define REG_SECTOR_COUNT 2  /* (R/W) */
#define REG_LBA_LOW     3
#define REG_LBA_MID     4
#define REG_LBA_HIGH    5
#define REG_DRIVE_HEAD  6
#define REG_STATUS_CMD  7   /* (R) status, (W) command */

#define CONTROL_nIEN    0x02

#define STATUS_ERR  0x01
#define STATUS_DRQ  0x08
#define STATUS_SRV  0x10
#define STATUS_DF   0x20
#define STATUS_RDY  0x40
#define STATUS_BSY  0x80

#define CMD_READ_PIO    0x20
#define CMD_IDENTIFY    0xEC

static void get_ports(uint8_t drive, uint16_t* base, uint16_t* ctrl, uint8_t* is_slave) {
    if (drive == 0) {
        *base = ATA_PRIMARY_BASE;
        *ctrl = ATA_PRIMARY_CTRL;
        *is_slave = 0;
    } else {
        *base = ATA_SECONDARY_BASE;
        *ctrl = ATA_SECONDARY_CTRL;
        *is_slave = (drive == 3);
    }
}

static int ata_wait_not_busy(uint16_t base, uint16_t ctrl, unsigned int timeout_ms) {
    /* Simple busy poll — microsecond scale. We'll loop a bounded number of times. */
    /* This is a rough loop; on real hardware you may want to use a timer. */
    unsigned int i;
    for (i = 0; i < timeout_ms * 1000u; i++) {
        uint8_t st = inb(base + REG_STATUS_CMD);
        if (!(st & STATUS_BSY)) return 0;
    }
    /* final check */
    return 1;
}


/* Wait for DRQ or error; return 0 if DRQ set, 1 if timeout, 2 if ERR set */
static int ata_wait_drq_or_err(uint16_t base, unsigned int timeout_ms) {
    unsigned int i;
    for (i = 0; i < timeout_ms * 1000u; i++) {
        uint8_t st = inb(base + REG_STATUS_CMD);
        if (st & STATUS_ERR) return 2;
        if (st & STATUS_DRQ) return 0;
        if (!(st & STATUS_BSY) && !(st & STATUS_DRQ)) {
            /* Not BSY but also not DRQ yet — keep waiting */
        }
    }
    return 1;
}

static void ata_read_sector_to(uint16_t base, void *buf) {
    uint16_t *wptr = (uint16_t *)buf;
    for (int i = 0; i < 256; ++i) { /* 512 bytes / 2 = 256 words */
        wptr[i] = inw(base + REG_DATA);
    }
}

int read_sector(uint8_t drive, uint32_t lba, uint8_t count, void *buffer) {
    printf("read_sector: drive=%d lba=%u count=%u\n to %x\n", drive, lba, count, buffer);

    if (count == 0) return 3;
    if (lba >= (1u << 28)) return 3; /* LBA28 only */

    uint16_t base, ctrl;
    uint8_t is_slave;
    get_ports(drive, &base, &ctrl, &is_slave);

    /* Use 28-bit LBA. Drive/head register: 0xE0 | (is_slave<<4) | (LBA >> 24 & 0x0F) */
    uint8_t drive_head = 0xE0 | ((is_slave & 1) << 4) | (uint8_t)((lba >> 24) & 0x0F);

    /* Sector count: 0 means 256 sectors for ATA */
    uint8_t sectors = count & 0xFF;
    if (sectors == 0) sectors = 0x00; /* write 0 to mean 256 */

    /* Wait for device ready (BSY clear, DRQ clear) */
    if (ata_wait_not_busy(base, ctrl, 500)) return 1;

    /* Select drive/head */
    outb(base + REG_DRIVE_HEAD, drive_head);
    io_wait(ctrl); /* give drive time */

    /* Set features = 0 (no special features) */
    outb(base + REG_FEATURES, 0x00);

    /* Write count and LBA low/mid/high */
    outb(base + REG_SECTOR_COUNT, sectors);
    outb(base + REG_LBA_LOW,  (uint8_t)(lba & 0xFF));
    outb(base + REG_LBA_MID,  (uint8_t)((lba >> 8) & 0xFF));
    outb(base + REG_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));

    /* Send READ command */
    outb(base + REG_STATUS_CMD, CMD_READ_PIO);

    /* For each sector:
       - wait for device to set DRQ (or error)
       - read 256 words
    */
    uint8_t *bufptr = (uint8_t *)buffer;
    for (uint16_t s = 0; s < count; ++s) {
        int w = ata_wait_drq_or_err(base, 500);
        if (w == 1) return 1; /* timeout */
        if (w == 2) return 2; /* device reported error */

        /* read sector */
        ata_read_sector_to(base, bufptr);
        bufptr += 512;

        /* After reading, the drive may continue to set DRQ for next sector or finish.
           Some drives require small delay before next polling */
        io_wait(ctrl);
    }

    /* final status check for error (optional) */
    uint8_t final_st = inb(base + REG_STATUS_CMD);
    if (final_st & STATUS_ERR) return 2;

    return 0;
}

uint32_t get_partition_lba(uint8_t drive, uint8_t part_num) {
    uint8_t sec_buf[512] = {0};
    if (read_sector(drive, 0, 1, sec_buf))
        return 0;

    return *(uint32_t*)&sec_buf[446 + part_num * 16 + 8];
}