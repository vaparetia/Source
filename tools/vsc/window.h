#ifndef _window_h_
#define _window_h_

#define W_Hamming(n, N)  (0.54 - 0.46 * cos(2 * M_PI * (n) / (N)))
#define W_Hanning(n, N)  (0.5 - 0.5 * cos(2 * M_PI * (n) / (N)))
#define W_Blackman(n, N) (0.42 - 0.5 * cos(2 * M_PI * (n) / N) \
                                  + 0.08 * cos(4 * M_PI * (n) / N))


/* =========================================================================
 * Áë´Ø¿ô
 * ========================================================================= */
void wndHamming(double *wav, int len);
void wndHanning(double *wav, int len);
void wndBlackman(double *wav, int len);

#endif /* _window_h_ */
