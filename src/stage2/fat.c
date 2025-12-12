#define SECTOR_SIZE 512

#include <system.h>
#include <io.h>

#define FAT_EOC32 0x0FFFFFF8u

FAT_info fat;
static uint8_t sector_buf[SECTOR_SIZE];
static uint8_t fat_buf[SECTOR_SIZE];
static uint8_t fat_buf2[SECTOR_SIZE];

static int read_lba(uint32_t lba) {
    return read_sector(fat.drive, lba, 1, sector_buf);
}

int fat_init(uint8_t drive, uint32_t partition_lba) {
    fat.drive = drive;

    if (read_sector(drive, partition_lba, 1, sector_buf))
        return -1;

    fat.bytes_per_sector     = *(uint16_t*)&sector_buf[11];
    fat.sectors_per_cluster  = sector_buf[13];
    fat.reserved             = *(uint16_t*)&sector_buf[14];
    fat.fats                 = sector_buf[16];
    fat.root_entries         = *(uint16_t*)&sector_buf[17];
    fat.sectors_per_fat      = *(uint16_t*)&sector_buf[22];

    // FAT type detection
    fat.fat_type = 16;

    // Layout
    fat.fat_start  = partition_lba + fat.reserved;
    fat.root_start = fat.fat_start + fat.fats * fat.sectors_per_fat;
    fat.root_sectors = ((fat.root_entries * 32) + 511) / 512;
    fat.data_start = fat.root_start + fat.root_sectors;

    return 0;
}

static void make_83(const char *in, char out[11])
{
    memset(out, ' ', 11);
    int i = 0, j = 0;
    while (in[i] && j < 11) {
        if (in[i] == '.') {
            j = 8;
            i++;
            continue;
        }
        out[j++] = (in[i] >= 'a' && in[i] <= 'z') ? in[i] - 32 : in[i];
        i++;
    }
}

static uint32_t fat_next_cluster(uint32_t cluster) {
    if (cluster < 2) return 0xFFFFFFFF;
    uint32_t fat_offset_bytes;
    uint32_t need_bytes;

    
    if (fat.fat_type == 12) {
        /* FAT12: 12 bits per entry. offset = cluster + (cluster / 2) */
        fat_offset_bytes = cluster + (cluster / 2);
        need_bytes = 2;
    } else if (fat.fat_type == 16) {
        /* FAT16: 16 bits per entry. offset = cluster * 2 */
        fat_offset_bytes = cluster * 2;
        need_bytes = 2;
    } else {
        /* FAT32: 32 bits per entry. offset = cluster * 4 */
        fat_offset_bytes = cluster * 4;
        need_bytes = 4;
    }
    
    uint32_t bps = fat.bytes_per_sector ? fat.bytes_per_sector : SECTOR_SIZE;
    uint32_t sector_index = fat.fat_start + (fat_offset_bytes / bps);
    uint32_t offset_in_sector = fat_offset_bytes % bps;
    
    if (read_sector(fat.drive, sector_index, 1, fat_buf))
    return 0xFFFFFFFF;
    
    if (offset_in_sector + need_bytes > bps) {
        if (read_sector(fat.drive, sector_index + 1, 1, fat_buf2))
        return 0xFFFFFFFF;
        
        uint8_t bytes[4] = {0};
        uint32_t first_part = bps - offset_in_sector;
        memcpy(bytes, &fat_buf[offset_in_sector], first_part);
        memcpy(bytes + first_part, &fat_buf2[0], need_bytes - first_part);
        
        uint32_t raw = 0;
        for (uint32_t i = 0; i < need_bytes; i++) {
            raw |= (uint32_t)bytes[i] << (i * 8);
        }
        
        if (fat.fat_type == 12) {
            uint32_t value;
            if (cluster & 1) {
                /* odd cluster: high 12 bits of the 16-bit little-endian raw */
                value = raw >> 4;
            } else  {
                value = raw & 0x0FFF;
            }
            if (value >= 0xFF8) return 0xFFFFFFFF;
            return value & 0x0FFF;
        } else if (fat.fat_type == 16) {
            uint32_t value = raw & 0xFFFF;
            if (value >= 0xFFF8) return 0xFFFFFFFF;
            return value;
        } else {
            uint32_t value = raw & 0x0FFFFFFF;
            if (value >= FAT_EOC32) return 0xFFFFFFFF;
            return value;
        }
    } else {
        if (fat.fat_type == 12) {
            /* read 2 bytes */
            uint16_t raw = *(uint16_t*)&fat_buf[offset_in_sector];
            uint32_t value = (cluster & 1) ? (raw >> 4) : (raw & 0x0FFF);
            if (value >= 0xFF8) return 0xFFFFFFFF;
            return value & 0x0FFF;
        } else if (fat.fat_type == 16) {
            uint16_t raw = *(uint16_t*)&fat_buf[offset_in_sector];
            printf("Raw: %x\n", raw);
            if (raw >= 0xFFF8) return 0xFFFFFFFF;
            return (uint32_t)raw;
        } else {
            /* FAT32: read 4 bytes and mask top 4 bits */
            uint32_t raw = *(uint32_t*)&fat_buf[offset_in_sector];
            uint32_t value = raw & 0x0FFFFFFF;
            if (value >= FAT_EOC32) return 0xFFFFFFFF;
            return value;
        }
    }
}

int fat_find_file(const char *name, FAT_file *file)
{
    char fatname[11];
    make_83(name, fatname);

    uint32_t entries = fat.root_entries;
    uint32_t sector  = fat.root_start;
    uint32_t offset  = 0;

    while (entries) {
        if (read_sector(fat.drive, sector, 1, sector_buf))
            return -1;

        for (int i = 0; i < 512; i += 32) {
            if (sector_buf[i] == 0x00)
                return -1; // end

            if (!(sector_buf[i+11] & 0x08)) { // not volume label
                if (!memcmp(&sector_buf[i], fatname, 11)) {
                    memcpy(file->name, fatname, 11);
                    file->attr = sector_buf[i+11];
                    file->first_cluster = *(uint16_t*)&sector_buf[i+26];
                    file->size = *(uint32_t*)&sector_buf[i+28];
                    return 0;
                }
            }
        }

        sector++;
        entries -= (512 / 32);
    }
    return -1;
}


int fat_read_file(const FAT_file *file, void *buffer, uint32_t bufsize)
{
    uint32_t cluster = file->first_cluster;
    uint32_t remaining = file->size;
    uint8_t *buf = (uint8_t*)buffer;

    while (cluster != 0xFFFFFFFF && remaining) {
        uint32_t first_sector = fat.data_start + (cluster - 2) * fat.sectors_per_cluster;

        for (int i = 0; i < fat.sectors_per_cluster && remaining; i++) {
            printf("Reading sector %u of cluster %u\n", i, cluster);
            if (read_sector(fat.drive, first_sector + i, 1, buf))
                return -1;

            buf += 512;
            if (remaining >= 512)
                remaining -= 512;
            else
                remaining = 0;

            printf("Remaining %u\n", remaining);
        }

        cluster = fat_next_cluster(cluster);
        printf("Next cluster: 0x%x\n", cluster);
    }

    return 0;
}

static void print_name(uint8_t *entry)
{
    char name[13];
    int i, j=0;

    // 8.3 format
    for (i=0;i<8;i++) {
        if (entry[i]==' ') break;
        name[j++] = entry[i];
    }

    if (entry[8] != ' ') {
        name[j++] = '.';
        for (i=8;i<11;i++) {
            if (entry[i]==' ') break;
            name[j++] = entry[i];
        }
    }
    name[j] = 0;
    printf("%-12s %s\n", name, (entry[11]&0x10) ? "<DIR>" : "");
}


void fat_list_dir(uint32_t dir_start, uint32_t entries_count, int is_root) {
    uint32_t sectors = (entries_count * 32 + 511) / 512;
    uint32_t sector = dir_start;

    for (uint32_t s=0; s<sectors; s++) {
        if (read_sector(fat.drive, sector+s, 1, sector_buf)) return;

        for (int i=0; i<512; i+=32) {
            uint8_t *entry = &sector_buf[i];

            if (entry[0]==0x00) return; // no more entries
            if (entry[0]==0xE5) continue; // deleted
            if (entry[11]==0x0F) continue; // LFN
            if ((entry[11]&0x08)) continue; // volume label

            print_name(entry);
        }
    }
}