#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _window_h_
#include "window.h"
#include "env.h"

void wndHanning(double *wav, int len)
{
  int i;
  double a;

  a = 2 * M_PI / (len - 1);

  for(i = 0; i < len; i++)
    wav[i] *= 0.5 - 0.5 * cos(a * (double)i);
}

void wndHamming(double *wav, int len)
{
  int i;
  double a;

  a = 2 * M_PI / (len - 1);

  for(i = 0; i < len; i++)
    wav[i] *= 0.54 - 0.46 * cos(a * (double)i);
}

void wndBlackman(double *wav, int len)
{
  int i;
  double a, b;

  a = 2 * M_PI / (len - 1);
  b = 4 * M_PI / (len - 1);

  for(i = 0; i < len; i++)
    wav[i] *= 0.42 - 0.5 * cos(a * (double)i) + 0.08 * cos(b * (double)i);
}
