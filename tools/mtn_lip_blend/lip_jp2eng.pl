#!/usr/bin/perl

if ( @ARGV < 1 ){
	print "mtn lip jp2eng batch command \n" ;
	print "Usage: lip_jp2eng.pl out_mtn_path [org_mtn_path(jp_mod)] [lip_mtn_path(eng)]  \n" ;
	exit 0 ;
}

$org_motion_path = $ARGV[1] ;
$lip_motion_path = $ARGV[2] ;
$out_motion_path = $ARGV[0] ;
if ( $ARGV[1] eq "" ){
	$org_motion_path = "/u/develop/mj001data/runtime/sound/vox1/mtn1c_mod" ;
}
if ( $ARGV[2] eq "" ){
	$lip_motion_path = "/u/develop/mj001data/runtime/sound.eng/vox1/mtn1c" ;
}


print "$org_motion_path $lip_motion_path $out_motion_path\n" ;

# main
{
	my @file_list ;
	my $file ;
	my $org_file ;
	my $body_file ;
	my $lip_file ;
	my $out_file ;
	my $outbody_file ;

	@file_list = split( '\n', `ls -XpB -w 1 $org_motion_path | grep -v / | grep -v '\_b\.mtn' ` );
	if ( $#file_list eq "-1" ){
		exit -1 ;
	}
	foreach $file ( @file_list ){
		$org_file = $org_motion_path."/".$file ;
		$lip_file = $lip_motion_path."/".$file ;
		$out_file = $out_motion_path."/".$file ;

		$body_file = $org_file ;
		$body_file =~ s/.mtn$/_b.mtn/ ;

		$outbody_file = $out_file ;
		$outbody_file =~ s/.mtn$/_b.mtn/ ;

		#print "$file\n" ;
		#print "$org_file\n" ;
		#print "$lip_file\n" ;
		#print "$out_file\n" ;

		if ( -e $lip_file ){
			print "convert $org_file\n" ;
			#print "body $body_file\n" ;
			system("mtn_lip_blend $org_file $lip_file $out_file");
			system("cp $body_file $outbody_file");
		} else {
			print "skip $org_file\n" ;
		}
	}
}

