Description:

MiniAMR with full communication/synchronization constructs, full refinement, full coarsening, dummy compute. Stencil coming soon.

Each refinement, each child block is provided with a coarsening event that they must satisfy to coalesce into the parent of larger size. If the
code runs to completion without all children coarsening, each child satisfies their event whilst indicating that the code has run its coarse to
the coalescing edt, thus creating a chain-reaction join. This will enable the runtime to satisfy the finish event on which wrapupEdt() depends.

To run:

FOR REFINEMENT AND COARSENING
./go.sh [dimension] [maxRefLvl] [timesteps] [refineFreq]

FOR REFINEMENT WITHOUT COARSENING
./goNC.sh [dimension] [maxRefLvl] [timesteps] [refineFreq]

NOTE: The non-coarsening option is for Bala to run studies on his load balancer. For any normal runs for this version,

Recommendations:

My recommendation is to keep [timesteps] clamped to something like 500. You will see a spike in running times when increasing dimensions.

Play around a bit with [maxRefLvl] if you're feeling adventurous. just remember that the number of potential blocks will
be dimension^3 * 8^maxRefLvl, with all blocks refining to their finest granularity at the worst case, with Coarsening turned off.

For larger workload sizes (above 6^3 root blocks) it is recommended to decrease the number of timesteps to 150 to
200 steps (so as to not be waiting all day), while having a fairly high [refineFreq] (refine every 2 timesteps should suffice).
The dial for maxRefLvl may be turned freely to stress the load balancer.


With coarsening included:

Play around with any size; you should see that the running time should balloon for the middle of the problem, and as the object passes through the mesh,
the mesh coarsens back to its root domain size. The user should see the running time per-timestep greatly reduce.
