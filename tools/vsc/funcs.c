#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double func_TildeOmega(double omega, double alpha)
{
  return omega + 2 * atan(alpha * sin(omega) / (1 - alpha * cos(omega)));
}

