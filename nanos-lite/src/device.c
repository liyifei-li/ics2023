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
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  char event_text[16]; //Maximum length of keyname is 11
  if (kbd.keycode) {
    if (kbd.keydown) {
      strcpy(event_text, "kd "); 
    }
    else {
      strcpy(event_text, "ku "); 
    }
    strcat(event_text, keyname[kbd.keycode]);
    strcat(event_text, "\n");
    strncpy((char *)buf, event_text, len);
    return len < strlen(event_text) ? len : strlen(event_text);
  }
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
