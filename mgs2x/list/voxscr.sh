#! /bin/tcsh -f

# あるディレクトリの.PCMから.SCRを作成する

foreach pcmfile ( $1/*.pcm )
	set base = `echo $pcmfile | sed "s/\.pcm//g" | sed "s/.*\///g"`

	echo "block $base"
	echo  "{"

	# sound block output
	echo "	sound $pcmfile"

	set mstfile = `echo $pcmfile | sed -e "s/pcm/mst/g"`
	set mstfile_mod = `echo $mstfile | sed -e "s/\/vc/_mod\/vc/g"`
	if ( -e $mstfile_mod ) then
		echo "	motion $mstfile_mod"
	else if ( -e $mstfile ) then
		echo "	motion $mstfile"
	endif

	set capfile = `echo $pcmfile | sed -e "s/pcm/cap/g"`
	if( -e $capfile ) then
		echo "	cdccap $capfile"
	endif

	echo "}"
end
	
