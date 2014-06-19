BENCHMARK=$1
if [ -n "$OCR_INSTALL" ]; then
 if [ -n "$BENCHMARK" ]; then
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I . -DNDEBUG  -I${OCR_INSTALL}/include  -DOCR_TARGET micro_kernels.c -o micro_kernels.o
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I .  -DNDEBUG  -I${OCR_INSTALL}/include  -DOCR_TARGET flop.c -o flop.o
gcc -O3 -c -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I .  -DNDEBUG  -I${OCR_INSTALL}/include  -DOCR_TARGET $BENCHMARK.ocr.c -o $BENCHMARK.ocr.o
gcc -O3 $BENCHMARK.ocr.o micro_kernels.o flop.o librstream_ocr.a libx86_dma.a  -lm  -lstdc++ -lm -L${OCR_INSTALL}/lib -locr -Wl,-rpath=${OCR_INSTALL}/lib -lstdc++  -o $BENCHMARK.ocr
 else
    echo "Please specify the benchmark to compile. For example, ./compile.sh matmult"
 fi
else
    echo "Please set the OCR_INSTALL path in the compilation script"
fi

