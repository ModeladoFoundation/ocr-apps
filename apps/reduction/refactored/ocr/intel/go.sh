#export ENABLE_PROFILER=yes
#export PROFILER_FOCUS=userCode
#export PROFILER_IGNORE_RT=yes
#export PROFILER_COUNT_OTHER=yes

#export NO_DEBUG="yes"
#export RUN_TOOL=/usr/bin/time
#module use /opt/rice/modulefiles
#module load hpctoolkit-mpi
#OCR_TYPE=x86  make -j32 uninstall clean install
#OCR_TYPE=x86-mpi  make -j32 uninstall clean install
#WORKLOAD_ARGS="30 1 10" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 make uninstall clean run
WORKLOAD_ARGS="8 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
WORKLOAD_ARGS="16 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile2" make uninstall clean run
WORKLOAD_ARGS="32 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile4" make uninstall clean run
WORKLOAD_ARGS="64 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
WORKLOAD_ARGS="128 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
WORKLOAD_ARGS="256 1 90" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=2 OCR_NUM_NODES=1  make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 2 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9  make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="2 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
#WORKLOAD_ARGS="4 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile2" make uninstall clean run
#WORKLOAD_ARGS="4 2 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile2" make uninstall clean run
#WORKLOAD_ARGS="4 4 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile4" make uninstall clean run
#WORKLOAD_ARGS="4 4 2 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile4" make uninstall clean run
#WORKLOAD_ARGS="4 4 4 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
#WORKLOAD_ARGS="4 4 4 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
#WORKLOAD_ARGS="8 4 4 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
#WORKLOAD_ARGS="8 4 4 64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
#WORKLOAD_ARGS="8 8 4  64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
#WORKLOAD_ARGS="8 8 4  64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
#WORKLOAD_ARGS="8 8 8  64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile64" make uninstall clean run
#WORKLOAD_ARGS="8 8 8  64 30 0" OCR_TYPE=x86-mpi CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile64" make uninstall clean run
#WORKLOAD_ARGS="8 6 10 64 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 64 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile64" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 128 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 128 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 128 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile4" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 128 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile2" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 128 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile1" make uninstall clean run
###WORKLOAD_ARGS="4 6 10 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="8 6 10 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="8 6 5 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="8 6 10 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
###WORKLOAD_ARGS="8 12 10 92 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile32" make uninstall clean run
###WORKLOAD_ARGS="3 3 3 16 100 0" OCR_TYPE=x86 make uninstall clean run
###WORKLOAD_ARGS="3 4 5 16 100 0" CONFIG_NUM_THREADS=9 OCR_TYPE=x86 OCR_NUM_NODES=8  make uninstall clean run
###WORKLOAD_ARGS="6 8 10 64 10 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9  NODEFILE=/mnt/nordicfs/dsscott/repo/apps/apps/hpcg/refactored/ocr/intel/nodefile16 make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=4 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=5 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=5 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=6 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=6 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=7 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
###WORKLOAD_ARGS="12 10 8 64 30 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=7 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile16" make uninstall clean run
####WORKLOAD_ARGS="12 7 8 16 100 0" OCR_TYPE=x86 CONFIG_NUM_THREADS=9 OCR_NODEFILE="/home/dsscott/repo5/apps/apps/hpcg/refactored/ocr/intel/nodefile8" make uninstall clean run
