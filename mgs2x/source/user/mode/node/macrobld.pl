#!/usr/bin/perl

if($ARGV[0] eq "-h") {
    print "usage: macrobld.pl < listfile > header.h\n";
    exit 1;
}


@list = <STDIN>;

foreach $line (@list) {
  $line =~ s/\n$//;                    # 改行を除去
  if($line eq "") {                    # 空行は空行を表示して次へ
    print "\n";
    next;
  }
  if($line =~ /^\ *\#/) {
      $line =~ s/^\ *\#\ *//;
      printf "/* %s */\n", $line;
      next;
  }

  ($macro, $name) = split ':', $line;  # マクロ名と文字列に分ける

  # $macro の前後の空白を除去
  $macro =~ s/^[\t\ ]*//;
  $macro =~ s/[\t\ ]*$//;

  # $name の前後の空白を除去
  if( !($name =~ /^$/) ) {
      $name =~ s/^[\t\ ]*//;
      $name =~ s/[\t\ ]*$//;
      open SCD, "strcode $name |" || die "could not execute strcode.";
      @ans = <SCD>;

      ($hx, $dc) = split ':', $ans[0];
      # $hx の前後の空白を除去
      $hx =~ s/^[\t\ ]*//;
      $hx =~ s/[\t\ ]*$//;

      printf "#define %s\t0x%s\t/* \"%s\" */\n", $macro, $hx, $name;
  }
  close SCD;
}

1;


