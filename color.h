#define COLOR_COUNT 8

#define ESC "\x1b"

static char *colors[COLOR_COUNT] =
  { "\xFF",
    "0",
    "1",
    "2",
    "[",
    ESC "[1m1" ESC "[22m",
    ESC "[1m2" ESC "[22m",
    ESC "[1m[" ESC "[22m" };
