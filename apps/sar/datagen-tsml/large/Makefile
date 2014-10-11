# Synthetic Aperture Radar (SAR) point target simulator.
# USAGE: datagen <Data output file>
#                <Platform position outputfile>
#                <Pulse transmission time output file>
#                <Radar parameters>
# NOTES: The code expects Targets.txt and Parameters.txt to be
# located in the working directory.

INSTALL_DIR = /opt/share/xstack_app_input/sar/large

RM=rm -f
CP=cp -f
MKDIR=mkdir -p

TG_AS=../../../../ss/install/bin/rmd-linux-elf-as

all:	Radar.txt

Radar.txt:	Parameters.txt Targets.txt Makefile
	../datagen Data.bin PlatformPosition.bin PulseTransmissionTime.bin Radar.txt
	../../utils/viewData 1 500.0 <Data.bin >ref.bmp
	../../utils/viewData 2 250.0 <Data.bin >cur.bmp

run:	all

clean:
	$(RM)	Data.bin PlatformPosition.bin PulseTransmissionTime.bin Radar.txt \
		ref.bmp cur.bmp

install:
	$(MKDIR) $(INSTALL_DIR) $(INSTALL_DIR)/x86 $(INSTALL_DIR)/tg
	$(CP)	Data.bin PlatformPosition.bin PulseTransmissionTime.bin \
		Parameters.txt Targets.txt Radar.txt \
		$(INSTALL_DIR)
	(cd ../../ocr/large;../../utils/dataImage    1 <$(INSTALL_DIR)/Data.bin >$(INSTALL_DIR)/x86/Data.c)
	(cd ../../ocr/large;../../utils/dataImage    2 <$(INSTALL_DIR)/PlatformPosition.bin >$(INSTALL_DIR)/x86/PlatformPosition.c)
	(cd ../../ocr/large;../../utils/dataImage    3 <$(INSTALL_DIR)/PulseTransmissionTime.bin >$(INSTALL_DIR)/x86/PulseTransmissionTime.c)
	(cd ../../ocr/large;../../utils/dataImageAsm 1 <$(INSTALL_DIR)/Data.bin >$(INSTALL_DIR)/tg/Data.s)
	(cd ../../ocr/large;../../utils/dataImageAsm 2 <$(INSTALL_DIR)/PlatformPosition.bin >$(INSTALL_DIR)/tg/PlatformPosition.s)
	(cd ../../ocr/large;../../utils/dataImageAsm 3 <$(INSTALL_DIR)/PulseTransmissionTime.bin >$(INSTALL_DIR)/tg/PulseTransmissionTime.s)
	$(CC)    -c -o $(INSTALL_DIR)/x86/Data.o \
		       $(INSTALL_DIR)/x86/Data.c
	$(CC)    -c -o $(INSTALL_DIR)/x86/PlatformPosition.o \
		       $(INSTALL_DIR)/x86/PlatformPosition.c
	$(CC)    -c -o $(INSTALL_DIR)/x86/PulseTransmissionTime.o \
		       $(INSTALL_DIR)/x86/PulseTransmissionTime.c
	$(TG_AS) -o $(INSTALL_DIR)/tg/Data.o \
		    $(INSTALL_DIR)/tg/Data.s
	$(TG_AS) -o $(INSTALL_DIR)/tg/PlatformPosition.o \
		    $(INSTALL_DIR)/tg/PlatformPosition.s
	$(TG_AS) -o $(INSTALL_DIR)/tg/PulseTransmissionTime.o \
		    $(INSTALL_DIR)/tg/PulseTransmissionTime.s

uninstall:
	$(RM)	$(INSTALL_DIR)/Data.bin \
		$(INSTALL_DIR)/PlatformPosition.bin \
		$(INSTALL_DIR)/PulseTransmissionTime.bin \
		$(INSTALL_DIR)/Parameters.txt \
		$(INSTALL_DIR)/Targets.txt \
		$(INSTALL_DIR)/Radar.txt \
		$(INSTALL_DIR)/x86/Data.c \
		$(INSTALL_DIR)/x86/PlatformPosition.c \
		$(INSTALL_DIR)/x86/PulseTransmissionTime.c \
		$(INSTALL_DIR)/tg/Data.s \
		$(INSTALL_DIR)/tg/PlatformPosition.s \
		$(INSTALL_DIR)/tg/PulseTransmissionTime.s \
		$(INSTALL_DIR)/x86/Data.o \
		$(INSTALL_DIR)/x86/PlatformPosition.o \
		$(INSTALL_DIR)/x86/PulseTransmissionTime.o \
		$(INSTALL_DIR)/tg/Data.o \
		$(INSTALL_DIR)/tg/PlatformPosition.o \
		$(INSTALL_DIR)/tg/PulseTransmissionTime.o
