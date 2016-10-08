# N-Queens problem solver
Author: Jorge Bellon-Castro <jbellonc@intel.com>

This benchmark application implements a bitwise recursive algorithm[1] that
computes the number of solutions to the N-queens problem for a given N.

Base version is implemented using OpenMP tasks (OpenMP 3.0 or greater required).

Refactored versions include:
 * OpenMP:
Task creation is bounded (using final clause) to reduce runtime overhead
overall impact.

 * OmpSs:
Final clause is not supported for nanos6 API. Bounding is implemented manually.
C11 atomics are compiled with GCC in a separate file, as C11 support is not completed.

[1] Martin Richards, "Backtracking algorithms in MCPL using Bit
                      Patterns and Recursion"; online, available 10/07/2016
http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.51.7113&rep=rep1&type=pdf

