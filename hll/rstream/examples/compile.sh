BENCHMARK=$1
if [ -n "$OCR_INSTALL" ]; then
 if [ -n "$BENCHMARK" ]; then
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I . -DNDEBUG -I${OCR_INSTALL}/include -I ${OCR_INSTALL}/include/extensions -DOCR_TARGET micro_kernels.c -o micro_kernels.o
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I .  -DNDEBUG  -I${OCR_INSTALL}/include -I ${OCR_INSTALL}/include/extensions -DOCR_TARGET flop.c -o flop.o
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I .  -DNDEBUG  -I${OCR_INSTALL}/include -I ${OCR_INSTALL}/include/extensions -DOCR_TARGET $BENCHMARK.ocr.c -o $BENCHMARK.ocr.o
gcc -O3 $BENCHMARK.ocr.o micro_kernels.o flop.o -lrstream_ocr librstream_x86.a  -lm  -lstdc++ -lm -L${OCR_INSTALL}/lib -locr -Wl,-rpath=${OCR_INSTALL}/lib -L. -Wl,-rpath=. -lstdc++  -o $BENCHMARK.ocr
 else
    echo "Please specify the benchmark to compile. For example, ./compile.sh adi"
 fi
else
    echo "Please set OCR_INSTALL to the OCR installation path"
fi

