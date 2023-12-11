extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

size_t ramdisk_read(void *buf, size_t offset, size_t len);

size_t ramdisk_write(const void *buf, size_t offset, size_t len);

void init_ramdisk();

size_t get_ramdisk_size();
