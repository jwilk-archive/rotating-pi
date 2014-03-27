#define ESC "\x1b"

static const char *colors[] =
  { "\xFF",
    ESC "[30;1m0" ESC "[0m",
    ESC "[30;1m1" ESC "[0m",
    ESC "[30;1m[" ESC "[0m",
    "1",
    "[",
    ESC "[1m1" ESC "[0m",
    ESC "[1m[" ESC "[0m",
  };

static const size_t color_count = sizeof colors / sizeof colors[0];

/* vim:set ts=2 sw=2 et: */
