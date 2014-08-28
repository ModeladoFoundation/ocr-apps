`rm benchmark_bfs_timings.txt`

for nni in 10
do
    `mpirun -np 2 ./bfs_benchmark rmat_graph_"$nni_" >>benchmark_bfs_timings.txt`
done
