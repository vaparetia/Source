#! /bin/tcsh -f

set pss = `echo $1 | sed -e "s/\.mpv/.pss/g"`

echo "convert $1 -> $pss"
echo "pss\nstream video:0\ninput $1\nend\nend" > /tmp/tmp.mux
ps2str mux -o -v /tmp/tmp.mux $pss
rm /tmp/tmp.mux

echo "convert $pss -> $2"
pss2stream $pss $2

echo "remove $pss"
rm -f $pss
