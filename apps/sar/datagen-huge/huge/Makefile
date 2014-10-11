# Synthetic Aperture Radar (SAR) point target simulator.
# USAGE: datagen <Data output file>
#                <Platform position outputfile>
#                <Pulse transmission time output file>
#                <Radar parameters>
# NOTES: The code expects Targets.txt and Parameters.txt to be
# located in the working directory.

INSTALL_DIR = /opt/share/xstack_app_input/sar/huge

RM=rm -f
CP=cp -f

all:	Radar.txt

Radar.txt:	Parameters.txt Targets.txt Makefile
	../datagen Data.bin PlatformPosition.bin PulseTransmissionTime.bin Radar.txt

run:	all
	../../utils/viewData 1 500.0 <Data.bin >ref.bmp
	../../utils/viewData 2 250.0 <Data.bin >cur.bmp

clean:
	$(RM)	Data.bin PlatformPosition.bin PulseTransmissionTime.bin Radar.txt \
		ref.bmp cur.bmp

install:
	$(CP)	Data.bin PlatformPosition.bin PulseTransmissionTime.bin \
		Parameters.txt Targets.txt Radar.txt \
		$(INSTALL_DIR)

uninstall:
	$(RM)	$(INSTALL_DIR)/Data.bin \
		$(INSTALL_DIR)/PlatformPosition.bin \
		$(INSTALL_DIR)/PulseTransmissionTime.bin \
		$(INSTALL_DIR)/Parameters.txt \
		$(INSTALL_DIR)/Targets.txt \
		$(INSTALL_DIR)/Radar.txt
