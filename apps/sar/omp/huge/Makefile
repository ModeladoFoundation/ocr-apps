INPUTS=/opt/share/xstack_app_input/sar/huge
TARGET=../sscp
UTILS=../../utils

RM=rm -f

all:	$(TARGET) $(INPUTS)/Data.bin $(INPUTS)/PlatformPosition.bin $(INPUTS)/PulseTransmissionTime.bin

$(TARGET).dbg:
$(TARGET):
	$(MAKE) -C .. all

$(INPUTS)/Data.bin:
$(INPUTS)/PlatformPosition.bin:
$(INPUTS)/PulseTransmissionTime.bin:
	$(MAKE) -C ../../datagen-huge/huge all run install

run:	Detects.txt

Detects.txt: $(TARGET) $(INPUTS)/Data.bin $(INPUTS)/PlatformPosition.bin $(INPUTS)/PulseTransmissionTime.bin
	$(RUN_TOOL) $(TARGET) $(INPUTS)/Data.bin $(INPUTS)/PlatformPosition.bin $(INPUTS)/PulseTransmissionTime.bin Detects.txt

images_debug.bin:
corr_debug.bin: $(TARGET).dbg $(INPUTS)/Data.bin $(INPUTS)/PlatformPosition.bin $(INPUTS)/PulseTransmissionTime.bin
	$(TARGET).dbg $(INPUTS)/Data.bin $(INPUTS)/PlatformPosition.bin $(INPUTS)/PulseTransmissionTime.bin Detects.txt

view:	Detects.txt images_debug.bin corr_debug.bin $(UTILS)/viewImage
	$(UTILS)/viewImage 1 <Detects.txt >cur.bmp
	$(UTILS)/viewImage 2 <Detects.txt >ref.bmp
	$(UTILS)/viewImage 3 <Detects.txt >all.bmp
	$(UTILS)/viewCorr    >corr.bmp

$(UTILS)/viewImage:
	$(MAKE) -C $(UTILS) viewImage

check:
	wc  Detects.txt

clean:
	$(RM) -rf cur.bmp ref.bmp all.bmp corr.bmp \
	Detects.txt detects.txt
