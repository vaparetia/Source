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

  name2 = $17 " " $18
  country2 = $23
  gsub( /[ ]+/, " ", name2 )
  year2  = $19;
  month2 = $20;
  day2   = $21;

  # 生年月日 yyyy/mm/dd を 0xyyyymmdd 形式に変換
  y1 = year % 10;
  y2 = int( ((year % 100) - y1) / 10) ;
  y3 = int( ((year % 1000) - (y1+y2)) / 100) ;
  y4 = int( ((year % 10000) - (y1+y2+y3)) / 1000) ;
  m1 = month % 10;
  m2 = int( ((month % 100) - m1) / 10) ;
  d1 = day % 10;
  d2 = int( ((day % 100) - d1) / 10) ;

  y1_2 = year2 % 10;
  y2_2 = int( ((year2 % 100) - y1_2) / 10) ;
  y3_2 = int( ((year2 % 1000) - (y1_2+y2_2)) / 100) ;
  y4_2 = int( ((year2 % 10000) - (y1_2+y2_2+y3_2)) / 1000) ;
  m1_2 = month2 % 10;
  m2_2 = int( ((month2 % 100) - m1_2) / 10) ;
  d1_2 = day2 % 10;
  d2_2 = int( ((day2 % 100) - d1_2) / 10) ;

  print "\t\t$*: { " $1 ", " "'" name "', " "'" country "', 0x"y4""y3""y2""y1""m2""m1""d2""d1", " "'" name2 "', " "'" country2 "', 0x"y4_2""y3_2""y2_2""y1_2""m2_2""m1_2""d2_2""d1_2" },"
}

END {
  print "};"
}

