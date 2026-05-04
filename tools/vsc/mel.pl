#!/usr/bin/perl
# 指定された周波数を、メル周波数に変換し、FFT 単位を返す。

$freq = $ARGV[0];

$PI = 3.14159265358979323846;
$S  = 16000;
$N  = 512;
$alpha = 0.47;


$omega = 2 * $PI * $S / ($freq * $N);

$mel_omega = $omega + 2 * tan(($alpha * sin($omega)) / (1 - $alpha * cos($omega)));

$fft = $mel_omega / (2 * $PI);

print $fft."\n";
1;

