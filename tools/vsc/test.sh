#!/bin/csh

set home=`hostname`

switch($home)
  case 'kanpyo':
	echo kanpyo
        breaksw
  case 'tambour':
	echo tambour
  breaksw
endsw

