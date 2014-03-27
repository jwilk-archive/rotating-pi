#define COLOR_COUNT 8

#define ESC "\x1b"

static char *colors[COLOR_COUNT] =
  { "\xFF",
    ESC "[30;1m0" ESC "[0m",
    ESC "[30;1m1" ESC "[0m",
    ESC "[30;1m[" ESC "[0m",
    "1",
    "[",
    ESC "[1m1" ESC "[0m",
    ESC "[1m[" ESC "[0m",
  };

/* vim:set ts=2 sw=2 et: */
