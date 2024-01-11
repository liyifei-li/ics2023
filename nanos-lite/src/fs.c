#include <fs.h>
#include <ramdisk.h>
#include <device.h>

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_FB, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/*This is the information about all files in disk. */
Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]    = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT]   = {"stdout", 0, 0, 0, invalid_read, serial_write},
  [FD_STDERR]   = {"stderr", 0, 0, 0, invalid_read, serial_write},
  [FD_EVENTS]   = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_FB]       = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = gpu_config.vmemsz;
  return;
}

int fs_open(const char *pathname, int flags, int mode) {
  size_t file_table_size = sizeof(file_table) / sizeof(file_table[0]);
  for (int i = 0; i < file_table_size; i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  size_t file_size = file_table[fd].size;
  size_t file_disk_offset = file_table[fd].disk_offset;
  size_t file_open_offset = file_table[fd].open_offset;
  size_t ret;
  if (file_table[fd].read != NULL) {
    ret = file_table[fd].read(buf, file_open_offset, len);
  }
  else {
    ret = len < file_size - file_open_offset ? len : file_size - file_open_offset;
    ramdisk_read(buf, file_disk_offset + file_open_offset, ret);
  }
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  size_t file_size = file_table[fd].size;
  size_t file_disk_offset = file_table[fd].disk_offset;
  size_t file_open_offset = file_table[fd].open_offset;
  size_t ret;
  if (file_table[fd].write != NULL) {
    ret = file_table[fd].write(buf, file_open_offset, len);
  }
  else {
    ret = len < file_size - file_open_offset ? len : file_size - file_open_offset;
    ramdisk_write(buf, file_disk_offset + file_open_offset, ret);
  }
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  size_t ret;
  switch (whence) {
    case SEEK_SET: ret = file_table[fd].open_offset = offset; break;
    case SEEK_CUR: ret = file_table[fd].open_offset += offset; break;
    case SEEK_END: ret = file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: ret = 0; break;
  }
  return ret;
}

int fs_close(int fd) {
  return 0;
}
