trunk/apps/ocr-sar/Sensor/README.txt
----------------------------------------------------------------------
May 12, 2014 Update:
linux/fsim ocr versions

(1) Use -DRAG_PETER_DIST_AND_TRIG to support Peter Tang's optimizations.
Under -DRAG_PETER_DIST_AND_TRIG, there is also support for a -DRAG_SPAD option.
With -DRAG_SPAD, the image subblock is brought to SPAD, worked on with a
3-deep loop nest and then put back to BSM or DRAM.
Without -DRAG_SPAD, caching is used to capture locality for the image subblock.

(2) Under -DRAG_PETER_AND_TRIG_OFF there is also -DSINCOS option, to use the
merged sincosf() function.

(3) -DRAG_FINISH_EDT  enables the use of FINISH_EDTs, this is still being
debugged, and the conversion of all the places FINISH_EDTs could be used
hasn't been finished.  Currently this works on linux, but had an assert in
the CE on fsim.
CONSOLE: XE7 >>> //////// leave backproject_async slot 0
CONSOLE: XE7 >>> BSM_FREE (ptr=xxx) (dbg=yyy)
CONSOLE: XE7 >>> WARN: No room to add dependence

(4) -DRAG_NULL_GUID_WORKAROUND, workes around the fsim problem of not being able
to ocrAddDependency with a NULL_GUID.

(5) To run large problems on linux, one has to rebuild ocr to have a larger
INIT_DEQUE_CAPASITY value.  The script rebuild-ocr-for-sar.sh will do this.

(6) -DRAG_DRAM and -DRAG_BSM enable explicit data movement in the memory
hierarchy.

TODO:

(1) finish the convertion to FINISH_EDTs

(2) incrementially add back using DMA operations to move data between DRAM
    and BSM on FSIM.

(3) get rid of the GET_(INT|FLT|ect) macros and turn them back into normal C
    pointer arith/array index operations

(4) figure out why randomly, some of the linux ocr runs have a SEGV in the
    OCR runtime after the user EDTs finish.

(5) Map sincosf to the fsim instruction

roger.a.golliver@gmail.com
(503) 866-9331

----------------------------------------------------------------------
To pre-build datagen and other utils, from Sensor directory do:

	make utils

To use datagen to create the radar input files, from Sensor directory do:

	make radar_data

To built OCR linux and fsim versions of SAR for the "tiny" problem, from Sensor directory do:

	cd linix/tiny; make
and
	cd fsim/tiny;  make

To run the SAR tiny problem, go into one of the Sensor/[linux|fsim]/tiny directories and do:

	make run

To check the Detects.txt file and produce .bmp files, the after make run do:

	make view

Roger
