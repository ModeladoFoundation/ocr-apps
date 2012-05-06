all:	clean run

run:	../src/sscp
	/usr/bin/time ../src/sscp --nproc 8 \
		../../datagen/large/Data.bin \
		../../datagen/large/PlatformPosition.bin \
		../../datagen/large/PulseTransmissionTime.bin \
		Detects.txt
	sort -o	Detects.txt Detects.txt
	../../SSCP_afl/utils/viewImage 1 <Detects.txt >cur.bmp
	../../SSCP_afl/utils/viewImage 2 <Detects.txt >ref.bmp
	../../SSCP_afl/utils/viewImage 3 <Detects.txt >all.bmp
	../../SSCP_afl/utils/viewCorr    >corr.bmp
	wc  Detects.txt Detects.gold
	cmp Detects.txt Detects.gold

clean:
	$(RM)	Detects.txt images_debug.bin corr_debug.bin \
		cur.bmp ref.bmp all.bmp corr.bmp
