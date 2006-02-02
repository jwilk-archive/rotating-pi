/* Copyright (c) 2004, 2006 Jakub Wilk <ubanus@users.sf.net> */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "color.h"
#include "pi.h"

typedef int bool;
#define false 0
#define true 1

struct termios termattr;

#define c_default_color "7"

#define c_str_goto00 "\x1B[H"
#define c_str_clear "\x1B[H\x1B[2J"
#define c_str_color(color) "\x1B[0;3" color "m"
#define c_str_save "\x1B""7"
#define c_str_restore "\x1B""8"
#define c_str_drawon "\x1B[12m"
#define c_str_drawoff "\x1B[10;0m"
#define c_str_cursoron "\x1B[?25h"
#define c_str_cursoroff "\x1B[?25l"

double sines[1<<8];

static inline int get_pixel(int x, int y)
{
  char c;
  if (x < 0 || y < 0)
    return 0;
  if (x >= c_vimagewidth || y >= c_vimageheight)
    return 0;
  c = vimage[c_vimagewidth * y + x];
  if (c == '#')
    return c_vimagecolorcount - 1;
  if (c >= 'A' && c <= 'Z')
    return c_vimagecolorcount - (1 + c - 'A');
  return 0;
}

static inline double get_pixel_ex(double x, double y)
{
  double rx = floor(x); double fx = x - rx; double gx = 1 - fx;
  double ry = floor(y); double fy = y - ry; double gy = 1 - fy;

  int ix = rx;
  int iy = ry;  
  
  return
    gx * gy * get_pixel(ix + 0, iy + 0) +
    fx * gy * get_pixel(ix + 1, iy + 0) +
    gx * fy * get_pixel(ix + 0, iy + 1) +
    fx * fy * get_pixel(ix + 1, iy + 1);  
}

static void restore_screen(void)
{
  fputs(c_str_cursoron c_str_drawoff c_str_drawoff, stdout);
  fflush(stdout);
  tcsetattr(STDIN_FILENO, TCSANOW, &termattr);
}

static inline bool init_screen(void)
{
  struct termios cattr;
  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
    return false;
  tcgetattr(STDIN_FILENO, &termattr);
  atexit(restore_screen);
  cattr = termattr;
  cattr.c_lflag &= ~(ICANON|ECHO);
  cattr.c_cc[VMIN] = 0;
  cattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &cattr);
  fputs(c_str_clear c_str_color(c_default_color) c_str_drawon c_str_cursoroff, stdout);
  fflush(stdout);
  return true;
}

static inline void goto_0_0(void)
{
  fputs(c_str_goto00, stdout);
}

static inline void rh_init_sines(void)
{
  int i;
#ifndef M_PI
  double M_PI_4 = atan(1);
#endif

  for (i=0; i<(1<<8); i++)
    sines[i]=sin(i*M_PI_4/(1<<5));
}

static inline bool get_term_size(int *x, int *y)
{
  struct winsize ws;

  *x = *y = 0;
  if (ioctl(0, TIOCGWINSZ, &ws) != -1)
  {
    *x=ws.ws_col;
    *y=ws.ws_row;
    return true;
  }
  return false;
}

static void signal_handler(int sn)
{
  restore_screen();
  fprintf(stderr, "Ouch! (%d)\n\n", sn);
  exit(EXIT_FAILURE);
}  
              
int main(int argc, char **argv)
{
  int c, x, y;
  uint8_t alpha;
  int sizex, sizey;
  int step = 0;

  if (argc >= 2)
    step = atoi(argv[1]);
  if (step < 1)
    step = 1;
  if (step > 10)
    step = 10;
  
  if (!get_term_size(&sizex, &sizey) || !init_screen())
  {
    fputs("Nasty screen, can't continue...\n", stderr);
    exit(EXIT_FAILURE);
  }
  signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGABRT, signal_handler);

  rh_init_sines();
  
  double cx = 0.5 * sizex;
  double cy = 0.5 * sizey;

  bool stop = false;
  for (alpha = 0; !stop; alpha += step)
  {
    goto_0_0();
    double sina = sines[alpha];
    double cosa = sines[(uint8_t)(alpha + (1 << 6))];
    for (y = 0; y < sizey; y++)
    for (x = 0; x < sizex; x++)
    {
      double dx =       (x - cx) * cosa - 2 * (y - cy) * sina + 0.5 * c_vimagewidth;
      double dy = 0.5 * (x - cx) * sina +     (y - cy) * cosa + 0.5 * c_vimageheight;

      c = get_pixel_ex(dx, dy) * c_colorcount / c_vimagecolorcount;
      if (c >= 0 && c < c_colorcount)
        fputs(colors[c], stdout);
    }
    fflush(stdout);
    static struct pollfd ufd = { .fd = STDIN_FILENO, .events = POLLIN };
    stop = poll(&ufd, 1, 40 * step) > 0;
  }
  return 0;
}

/* vim:set ts=2 sw=2 et: */
