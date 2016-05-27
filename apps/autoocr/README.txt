This directory structure contains examples for autoocr.

Directory contents:
 keyword - source input for autoocr
 autoocr - generated output from autoocr
 raw     - raw (non ocr) source
 hero    - hand modified ocr or autoocr files.

Common Make targets:

 clean: removes the contents of the build directores and the generated
        autoocr output in autoocr/<test>.c
 ocr: generates the autoocr output and compiles it into autoocr/build
      note that the generated autoocr output is now called <test>.c
 raw: compiles the raw source into raw/build
 run.ocr: runs autoocr/build/<>
 run.raw: runs raw/build/<>

hero-only  Make targets

fib/Makefile
   fib.con - compiles hero/fib.con.c into hero/build/fib.con
   run.fib.con - executes hero/build/fib.con
   fib.rag - compiles hero/fib.rag.c into hero/build/fib.rag
   run.fib.rag - executes hero/build/fib.rag

   fib-block - compiles hero/fib-block.c into hero/build/fib-block
   run.fib-block - execites hero/build/fib-block
                     (THIS EXAMPLE DOES NOT WORK - needs to be upgraded to OCR 1.1.0)

   fib-contn - compiles hero/fib-contn.c into hero/build/fib-contn
   run.fib-contn - executes hero/build/fib-contn
                     (THIS EXAMPLE DOES NOT WORK - needs to be upgraded to OCR 1.1.0)

qsort/Makefile
   qsort.con - compiles hero/qsort.con.c into hero/build/qsort.con
   run.qsort.con - executes hero/build/qsort.con

   qsort-block - compiles hero/qsort-block.c into hero/build/qsort-block ???
   run.qsort-block - executes hero/build/qsort-block ???
                     (THIS EXAMPLE DOES NOT WORK - needs to be upgraded to OCR 1.1.0)
