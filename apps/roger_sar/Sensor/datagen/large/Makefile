# Synthetic Aperture Radar (SAR) point target simulator.
# USAGE: datagen <Data output file>
#                <Platform position outputfile>
#                <Pulse transmission time output file>
#                <Radar parameters>
# NOTES: The code expects Targets.txt and Parameters.txt to be
# located in the working directory.

all:	Radar.txt

Radar.txt:	Parameters.txt Targets.txt Makefile
	../datagen Data.bin PlatformPosition.bin \
		PulseTransmissionTime.bin Radar.txt
	../utils/viewData 1 500.0 <Data.bin >ref.bmp
	../utils/viewData 2 250.0 <Data.bin >cur.bmp

run:	all

clean:
	$(RM)	Data.bin PlatformPosition.bin \
		PulseTransmissionTime.bin Radar.txt \
		ref.bmp cur.bmp
