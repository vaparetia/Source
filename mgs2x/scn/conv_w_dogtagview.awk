#! /bin/awk -f
# ビュワー用ののヘッダファイルを生成

BEGIN {
  FS = "\t"

  print "#ifndef _dogtag_view_h_"
  print "#define _dogtag_view_h_ 1"
  print ""

  print "resource dogtag_view {"
  print "\ttable:"
  print "\t\t$*:{"
}

$1 !~ /\/+/ {
  name = $7 " " $8
  gsub( /[ ]+/, " ", name )

  name2 = $17 " " $18
  gsub( /[ ]+/, " ", name2 )

  printf "\t\t { "
  printf "%s, ", $1
  printf "%s, ", "d:ステージ名:" $5

  printf "'%s', ", name
  if( $10 == "?" ) $10 = "0";
  if( $11 == "?" ) $11 = "0";
  printf "%s,%s, ", $10, $11
  printf "'%s',%s,%s,%s ", $13, $14,$15,$16

  printf "'%s', ", name2
  if( $20 == "?" ) $20 = "0";
  if( $21 == "?" ) $21 = "0";
  printf "%s,%s, ", $20, $21
  printf "'%s',%s,%s,%s ", $23, $24,$25,$26

  printf "},\n"
}

END {
  print "\t\t};"
  print "};"

  print ""
  print "#endif"
}
