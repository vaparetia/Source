Author
======

This tool was made by Emil Persson, AKA Humus.
http://www.humus.name



Usage
=====

On the commandline, use this syntax:

ParticleTrimmer <imagefile> <threshold 0-255> <vertex_count 3-8> [options]

	Option | Type   | Default | Description
	-------+--------+---------+------------------------------------------------------------
	 -ax   | int    |    1    | Atlas tile count in x
	 -ay   | int    |    1    | Atlas tile count in y
	 -sx   | float  |   1.0   | Scale factor of final x-coordinates
	 -sy   | float  |   1.0   | Scale factor of final y-coordinates
	 -bx   | float  |   0.0   | Bias factor of final x-coordinate
	 -by   | float  |   0.0   | Bias factor of final y-coordinate
	 -h    | int    |   50    | Maximum convex hull size to reduce to before optimization
	 -sp   | int    |   16    | Sub-pixel division when generating the convex hull
	 -d    | int    |    0    | Number of times to initially dilate the image
	 -i    | int[n] |  NULL   | Index buffer. Used to optimize vertex ordering if provided.
	-------+--------+---------+------------------------------------------------------------



Legal stuff
===========

This tool is freeware and may be used by anyone for any purpose
and may be distributed freely to anyone using any distribution
media or distribution method as long as this file is included.



Troubleshooting
===============

Is there a problem running the tool?
Refer to my site for information and ways to contact me if there is a problem.
I appreciate bug-reports and feedback.


Compiling code
==============

To compile this code, you need Framework3.zip from my site.
If there's a problem with compiling the code, make sure that you
got the right 
framework, and that you have a recent enough copy of it.
If it still won't compile, refer to my site for ways to contact me.
