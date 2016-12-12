BENCHMARK=$1
CC="gcc"
CFLAGS="-O3 -DRSTREAM_INLINE=inline -DRSTREAM_FLOATING_POINT_CEIL_FLOOR -I . -DNDEBUG -I${OCR_INSTALL}/include -I ${OCR_INSTALL}/include/extensions -I rocr_include -DOCR_TARGET"
LDFLAGS="-Wl,-rpath="." -L . -lrstream_ocr -lm -L${OCR_INSTALL}/lib -locr_x86 -pthread"

if [ -n "$OCR_INSTALL" ]; then
 if [ -n "$BENCHMARK" ]; then
     $CC $CFLAGS -c micro_kernels.c -o micro_kernels.o
     $CC $CFLAGS -c flop.c -o flop.o
     $CC $CFLAGS -c flop.c -o flop.o
     $CC $CFLAGS -c $BENCHMARK.gen.c -o $BENCHMARK.gen.o
     $CC $BENCHMARK.gen.o micro_kernels.o flop.o $LDFLAGS -o $BENCHMARK
 else
    echo "Please specify the benchmark to compile. For example, ./compile.sh adi"
 fi
else
    echo "Please set OCR_INSTALL to the OCR installation path"
fi

