#include <fs.h>
#include <ramdisk.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/*This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode) {
  size_t file_table_size = sizeof(file_table) / sizeof(file_table[0]);
  for (int i = 0; i < file_table_size; i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      return i;
    }
  }
  assert(0);
}

size_t fs_read(int fd, void *buf, size_t len) {
  size_t file_size = file_table[fd].size;
  size_t file_disk_offset = file_table[fd].disk_offset;
  size_t file_open_offset = file_table[fd].open_offset;
  printf("fs_read: %d %d %d\n", file_size, file_disk_offset, file_open_offset);
  assert(file_open_offset + len <= file_size);
  ramdisk_read(buf, file_disk_offset + file_open_offset, len);
  file_table[fd].open_offset += len;
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  size_t file_size = file_table[fd].size;
  size_t file_disk_offset = file_table[fd].disk_offset;
  size_t file_open_offset = file_table[fd].open_offset;
  printf("fs_write: %d %d %d\n", file_size, file_disk_offset, file_open_offset);
  assert(file_open_offset + len <= file_size + 10);
  ramdisk_write(buf, file_disk_offset + file_open_offset, len);
  file_table[fd].open_offset += len;
  return len;
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
