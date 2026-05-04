#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N  512.0
#define S  16000.0
#define ALPHA 0.47

int main(int argc, char **argv)
{
  double f;
  double omega, mel_omega;
  double n;

  if(argc != 2)
    {
      printf("usage: %s freq\n", argv[0]);
      return EXIT_FAILURE;
    }

  f = atof(argv[1]);
  printf("f = %8.3f\n", f);
  omega = 2 * M_PI * f / S;
  printf("omega = %8.3f\n", omega);
  printf("rev f = %8.3f\n", S * omega / (2 * M_PI));

  mel_omega = omega + 2 * atan((0.47 * sin(omega))/ (1 - 0.47 * cos(omega)));

  printf("mel omega = %8.3f\n", mel_omega);
  n = mel_omega * N / (2 * M_PI);
  printf("n = %8.3f (mel scaled)\n", n);

  return EXIT_SUCCESS;
}
