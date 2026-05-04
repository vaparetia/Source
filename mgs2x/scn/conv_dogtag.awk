#! /bin/awk -f
# ゲーム用のヘッダファイルを生成

BEGIN {
  FS = "\t"
  print "resource dogtag {"
}

$1 !~ /\/+/ {
  print "\t" $2 ":"
  name = $7 " " $8
  country = $13
  gsub( /[ ]+/, " ", name )
  year  = $9;
  month = $10;
  day   = $11;

  # 生年月日 yyyy/mm/dd を 0xyyyymmdd 形式に変換
  y1 = year % 10;
  y2 = int( ((year % 100) - y1) / 10) ;
  y3 = int( ((year % 1000) - (y1+y2)) / 100) ;
  y4 = int( ((year % 10000) - (y1+y2+y3)) / 1000) ;
  m1 = month % 10;
  m2 = int( ((month % 100) - m1) / 10) ;
  d1 = day % 10;
  d2 = int( ((day % 100) - d1) / 10) ;

  print "\t\t$*: { " $1 ", " "'" name "', " "'" country "', 0x"y4""y3""y2""y1""m2""m1""d2""d1" },"
}

END {
  print "};"
}

