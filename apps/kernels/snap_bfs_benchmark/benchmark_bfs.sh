`rm benchmark_bfs_timings.txt`

for nni in 5 10 20 40 80 100
do
    nn=$(($nni*50000))
    echo "n $nn" > graph_rmat_$nni.conf
    m=$(($nn*8))
    echo "m $m" >> graph_rmat_$nni.conf
    echo "a 0.6">> graph_rmat_$nni.conf
    echo "b 0.15">> graph_rmat_$nni.conf
    echo "c 0.15">>graph_rmat_$nni.conf
    echo "permute_vertices 1" >> graph_rmat_$nni.conf
    gent=`./rmat-gen graph_rmat_$nni.conf graph_rmat_tmp_$nni.txt` 
    `./edit_snap_graph.sh graph_rmat_tmp_$nni.txt graph_rmat_$nni.txt`
    bfst=`./rmat-bfs graph_rmat_$nni.txt`
    echo "$nn $gent $bfst" >> benchmark_bfs_timings.txt
done

 rm graph_rmat_*.conf
 rm graph_rmat_*.txt
 rm graph_rmat_tmp_*.txt