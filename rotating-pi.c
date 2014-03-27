/* Copyright © 2004, 2006, 2007, 2014 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "color.h"
#include "pi.h"

static struct termios term_attr;
static bool term_attr_saved = false;

#define DEFAULT_COLOR "7"

#define T_GOTO00 ESC "[H"
#define T_CLEAR ESC "[H" ESC "[2J"
#define T_COLOR(color) ESC "[0;3" color "m"
#define T_SAVE ESC "7"
#define T_RESTORE ESC "8"
#define T_DRAW_ON ESC "[12m"
#define T_DRAW_OFF ESC "[10;0m"
#define T_CURSOR_ON ESC "[?25h"
#define T_CURSOR_OFF ESC "[?25l"

static double sines[1 << 8];

static int get_pixel(int x, int y)
{
  if (x < 0 || y < 0)
    return 0;
  if (x >= IMAGE_WIDTH || y >= IMAGE_HEIGHT)
    return 0;
  char c = image[IMAGE_WIDTH * y + x];
  if (c == '#')
    return IMAGE_COLOR_COUNT - 1;
  if (c >= 'A' && c <= 'Z')
    return IMAGE_COLOR_COUNT - (1 + c - 'A');
  return 0;
}

static double get_pixel_ex(double x, double y)
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
  fputs(T_CURSOR_ON T_DRAW_OFF T_DRAW_OFF, stdout);
  fflush(stdout);
  if (term_attr_saved)
    tcsetattr(STDIN_FILENO, TCSANOW, &term_attr);
}

static bool init_screen(void)
{
  struct termios curr_attr;
  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
    return false;
  if (strcmp(getenv("TERM"), "linux") != 0)
    return false;
  tcgetattr(STDIN_FILENO, &term_attr);
  term_attr_saved = true;
  atexit(restore_screen);
  curr_attr = term_attr;
  curr_attr.c_lflag &= ~(ICANON | ECHO);
  curr_attr.c_cc[VMIN] = 0;
  curr_attr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &curr_attr);
  fputs(T_CLEAR T_COLOR(DEFAULT_COLOR) T_DRAW_ON T_CURSOR_OFF, stdout);
  fflush(stdout);
  return true;
}

static void goto_0_0(void)
{
  fputs(T_GOTO00, stdout);
}

static void init_sines(void)
{
  for (int i = 0; i < (1 << 8); i++)
    sines[i] = sin(i * M_PI / (1 << 7));
}

static bool get_term_size(unsigned int *x, unsigned int *y)
{
  struct winsize terminal_size;

  *x = *y = 0;
  if (ioctl(0, TIOCGWINSZ, &terminal_size) != -1)
  {
    *x = terminal_size.ws_col;
    *y = terminal_size.ws_row;
    return true;
  }
  return false;
}

static void signal_handler(int sn)
{
  restore_screen();
  fprintf(stderr, "Interrupted by user!\n");
  exit(EXIT_FAILURE);
}  
  
static void setup_signals()
{
  int signums[] = { SIGHUP, SIGINT, SIGTERM, SIGABRT, SIGQUIT, SIGKILL, 0 };
  struct sigaction sa = {
    .sa_handler = signal_handler,
    .sa_flags = SA_RESETHAND
  };
  sigfillset(&sa.sa_mask);
  for (int *signum_ptr = signums; *signum_ptr != 0; signum_ptr++)
    sigaction(*signum_ptr, &sa, NULL);
}

int main(int argc, char **argv)
{
  int step = 0;
  if (argc >= 2)
    step = atoi(argv[1]);
  if (step < 1)
    step = 1;
  if (step > 10)
    step = 10;
  
  unsigned int sizex, sizey;
  if (!get_term_size(&sizex, &sizey) || !init_screen())
  {
    fprintf(stderr, "I don't like this terminal!\n");
    return EXIT_FAILURE;
  }

  setup_signals();
  init_sines();
  
  double cx = 0.5 * sizex;
  double cy = 0.5 * sizey;

  bool stop = false;
  for (uint8_t alpha = 0; !stop; alpha += step)
  {
    goto_0_0();
    double sina = sines[alpha];
    double cosa = sines[(uint8_t)(alpha + (1 << 6))];
    for (unsigned int y = 0; y < sizey; y++)
    for (unsigned int x = 0; x < sizex; x++)
    {
      double dx =       (x - cx) * cosa - 2 * (y - cy) * sina + 0.5 * IMAGE_WIDTH;
      double dy = 0.5 * (x - cx) * sina +     (y - cy) * cosa + 0.5 * IMAGE_HEIGHT;

      int c = get_pixel_ex(dx, dy) * COLOR_COUNT / IMAGE_COLOR_COUNT;
      if (c >= 0 && c < COLOR_COUNT)
        fputs(colors[c], stdout);
    }
    fflush(stdout);
    static struct pollfd ufd = { .fd = STDIN_FILENO, .events = POLLIN };
    stop = poll(&ufd, 1, 40 * step) > 0;
  }
  return EXIT_SUCCESS;
}

/* vim:set ts=2 sw=2 et: */
