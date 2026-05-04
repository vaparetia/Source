#! /bin/tcsh -f

# あるディレクトリの.de2から.SCRを作成する

foreach demofile ( $1/*/*.de2 )
	set base = `echo $demofile | sed "s/\.de2//g" | sed "s/.*\///g"`

	echo "block $base"
	echo  "{"

	# demo block output
	echo "	demo $demofile"

	# sound block output
	if( -e $2/$base.pcm ) then
		echo "	sound $2/$base.pcm"
	endif
	echo "}"
end
	
