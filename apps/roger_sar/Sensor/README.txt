trunk/apps/roger_sar/Sensor/README.txt
----------------------------------------------------------------------
May 16,2012 Update:
H-C/AFL/SIM versions now support Peter Tang's optimizations.
-DRAG_PETER_DIST_AND_TRIG or -DRAG_PETER_DIST_AND_TRIG_OFF

Under -DRAG_PETER_DIST_AND_TRIG the AFL/SIM versions 
also support a -DRAG_SPAD or -DRAG_SPAD_OFF option.
With -DRAG_SPAD, the image subblock is brought to SPAD, worked on with a
3-deep loop nest and then put back to BSM or DRAM.
With -DRAG_SPAD_OFF then caching is used to capture locality for
the image subblock.

Under -DRAG_PETER_AND_TRIG_OFF there is a -DSINCOS or -DSINCOS_OFF option.

roger.a.golliver@gmail.com
(503) 866-9331

----------------------------------------------------------------------
To pre-build datagen and other utils, from Sensor directory do:

	make utils

To use datagen to create the radar input files, from Sensor directory do:

	make radar_data

To build H-C, AFL and SIM versions of SAR for small problem, from Sensor directory do:

	make small

To build H-C and AFL versions of SAR for large problem, from Sensor directory do:

	make large

To remove all but the files under svn control from, Sensor directory do:

	make clean

To run SAR, go into one of the Sensor/SSCP_[hc|afl|sim]/[small|large] directories and do:

	make run

To run power analysis on the small problem, from Sensor/SSCP_sim/small directory do:

	make power

Roger
