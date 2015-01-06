#!/bin/bash
executable=md
infile=
outdir=output
epsilon=1.0
sigma=1.0
pot=0.0
cutoff=2.5
cx=3
cy=3
cz=3
iters=1000
printstep=100
timestep=0.001
run=y

while getopts ":f:i:o:e:s:p:c:x:y:z:I:P:T:h" opt; do
  case $opt in
	f)
	  executable=$OPTARG
      run=$run-y
	  ;;
    i)
	  infile=$OPTARG
      run=$run-y
	  ;;
    o)
	  outdir=$OPTARG
      run=$run-y
      ;;
    e)
	  epsilon=$OPTARG
      ;;
    s)
	  sigma=$OPTARG
      ;;
    p)
	  pot=$OPTARG
      ;;
    c)
	  cutoff=$OPTARG
      ;;
    x)
	  cx=$OPTARG
      ;;
    y)
	  cy=$OPTARG
      ;;
    z)
	  cz=$OPTARG
      ;;
    I)
	  iters=$OPTARG
      ;;
    P)
	  printstep=$OPTARG
      ;;
    T)
	  timestep=$OPTARG
      ;;
    h)
	  echo "USAGE: runmd.sh -[options]" >&2
	  echo "-[options] are:" >&2
	  echo "-f :	Executable file" >&2
	  echo "-i :	Input file name" >&2
	  echo "-o :	Output directory" >&2
	  echo "-e :	epsilon (default 1.0)" >&2
	  echo "-s :	sigma (default 1.0)" >&2
	  echo "-p :	shift potential (default 0.0)" >&2
	  echo "-c :	cutoff distance (default 2.5)" >&2
	  echo "-x :	cells in x dimension (default 3)" >&2
	  echo "-y :	cells in y dimension (default 3)" >&2
	  echo "-z :	cells in z dimension (default 3)" >&2
	  echo "-I :	number of iterations (default 1000)" >&2
	  echo "-P :	interval of print output (in number of iterations, default 100)" >&2
	  echo "-T :	timestep (default 0.001)" >&2
	  echo "-h :	help" >&2
	  run=
      ;;
  esac
done

if [ "$run" = "y-y-y-y" ]; then
	echo "RUNNING JOB $executable.... Infile:$infile; Output:$outdir; timestep:$timestep; ep:$epsilon; sig:$sigma; cutoff:$cutoff; numcells:<$cx,$cy,$cz>; iterations: $iters; dump: $printstep; pot:$pot"
	$executable $infile $outdir $epsilon $sigma $pot $cutoff $cx $cy $cz $iters $printstep $timestep

	outfile=$(basename "$infile")
	outfile="${outfile%.*}"
	outfile=$outfile-$cx-$cy-$cz-$iters-$printstep

	cd $outdir
	lockdir=$outfile.lock

	if mkdir "$lockdir"
	then
		for i in `seq 0 $printstep $iters`; do cat $i.itr >> $outfile.out; done
		rm *.itr
		echo "Created output file $outdir/$outfile.out"
		trap 'rm -rf "$outdir/$lockdir"' 0
	fi
	cd ..
else
	echo "USAGE: runmd.sh -[options]" >&2
	echo "-[options] are:" >&2
	echo "-f :	Executable file" >&2
	echo "-i :	Input file name" >&2
	echo "-o :	Output directory" >&2
	echo "-e :	epsilon (default 1.0)" >&2
	echo "-s :	sigma (default 1.0)" >&2
	echo "-p :	shift potential (default 0.0)" >&2
	echo "-c :	cutoff distance (default 2.5)" >&2
	echo "-x :	cells in x dimension (default 3)" >&2
	echo "-y :	cells in y dimension (default 3)" >&2
	echo "-z :	cells in z dimension (default 3)" >&2
	echo "-I :	number of iterations (default 1000)" >&2
	echo "-P :	interval of print output (in number of iterations, default 100)" >&2
	echo "-T :	timestep (default 0.001)" >&2
	echo "-h :	help" >&2
fi
