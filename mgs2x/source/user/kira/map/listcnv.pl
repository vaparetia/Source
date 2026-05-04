#!/usr/bin/perl


$prefix = $ARGV[0];


@list = <STDIN>;

foreach $_ (@list) {

  s/\n//;        # 改行を除去

  s/#.*$//g;     # 注釈部を除去
  s/^[ \t]*//g;  # 行頭の空白を除去
  s/[ \t]*$//g;  # 行末の空白を除去

  if(!(/:/)) { next; }   # 意味を持った行でなければ、無視

  # マクロ名と注釈名を分割
  ($macro, $comment) = split ':', $_;

  # 行頭、行末の空白を除去
  $macro =~ s/^[ \t]*//g;
  $macro =~ s/[ \t]*$//g;

  $comment =~ s/^[ \t]*//g;
  $comment =~ s/[ \t]*$//g;

  # strcode を呼出し、strcode を求める
  open SCD, "strcode $macro |"  || die "could not execute strcode.";
  @ans = <SCD>;

  ($hx, $dc) = split ':', $ans[0];
  # $hx の前後の空白を除去
  $hx =~ s/^[\t\ ]*//;
  $hx =~ s/[\t\ ]*$//;
  close SCD;

  printf "#define %s%s\t0x%s\t/* %s */\n", $prefix, $macro, $hx, $comment;
}

1;
