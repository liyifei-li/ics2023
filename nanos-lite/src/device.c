#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  const char *p = buf;
  for (int i = 0; i < len; i++) {
    putch(*(p + i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T input_keybrd = io_read(AM_INPUT_KEYBRD);
  bool keydown = input_keybrd.keydown;
  int keycode = input_keybrd.keycode;
  if (keycode != AM_KEY_NONE) {
    return snprintf((char *)buf, len, "%s %s\n", keydown ? "kd" : "ku", keyname[keycode]);
  }
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
//  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
