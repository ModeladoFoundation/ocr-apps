# N-Queens problem solver
Author: Jorge Bellon-Castro <jbellonc@intel.com>

## Introduction
This benchmark application implements a bitwise recursive algorithm[1] that
computes the number of solutions to the N-queens problem for a given N.

## Implemented versions
Base version is implemented using OpenMP tasks (OpenMP 3.0 or greater required).

### Refactored versions
Base OpenMP version has been improved. Task creation is now bounded (using final
clause) to reduce runtime overhead's overall impact.

Ported versions include:
 * OmpSs:
Final clause is not supported for nanos6 API. Bounding is implemented manually.
C11 atomics are compiled with GCC in a separate file, as C11 support is not completed.

 * OCR:
OmpSs version ported to OCR. Keeping hand-coded final implementation.
Using global variables for EDT templates and solution reduction. Planned to move them
to a single shared datablock and keep using atomics for read-modify-write accesses.

## Algorithm explanation
Figure placement is represented using three different bit masks:
* `cols` represents which columns are currently occupied by a figure
* `ldiag` represents wether a column is being attacked by a figure from the left diagonal
* `rdiag` represents wether a column is being attacked by a figure from the right diagonal

Given a row i, masks' values can be obtained as:
```C
ldiag = cols >> i
rdiag = cols << i
cols = cols | selected_square
```

Which squares are being attacked: `(cols|ldiag|rdiag)`.
Its complementary value is used to calculate which squares are available. Note that
it is necessary to clear the positions greater than the most significant bit (i>N):
```C
available = ~(cols|ldiag|rdiag) & ((1<<N)-1)
```

## References
[1] Martin Richards, "Backtracking algorithms in MCPL using Bit
                      Patterns and Recursion"; online, available 10/07/2016
http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.51.7113&rep=rep1&type=pdf

