for nni in 10 12 14
do
    `mpirun -np 2 ./rmat_gen $nni rmat_graph_"$nni"_`
done
