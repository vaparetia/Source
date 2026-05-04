#! /bin/awk -f

BEGIN {
  printf "proc vox_test {\n";
  printf "\tchara sound_test test -s {\n";
}

$2 !~ /.*_len/ {
  printf "\t\t'%s' t:%s\n", $2, $2;
}

END {
  printf "\t}\n";
  printf "}\n";
}
