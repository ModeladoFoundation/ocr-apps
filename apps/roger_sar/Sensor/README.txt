trunk/apps/roger_sar/Sensor/README.txt

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
