#ifndef NEKO_HALO_UTIL_H
#define NEKO_HALO_UTIL_H

#include "nekos_triplet.h"
#include "nbn_setup.h"

typedef struct rankIndexedValue
{
    Idz rankID;     //From triplet2index and rankLattice
    Idz eleDofID;   //From elementLocalTriplets2index()
    NBN_REAL value;
} rankIndexedValue_t;

//Calculates the maximum entry count in the rankIndexedValue_t vector; NOT in bytes.
Idz calculate_length_rankIndexedValue(unsigned int in_pDOF, Triplet in_Elattice);
void sort_rankIndexedValue(Idz in_size, rankIndexedValue_t * io_vals);

//=Upon exit, the vector io_V will have had all of its local halo exchanges applied.
//=The io_returnValues represents the data to be exchanged via the external halos,
//i.e. the channel exchanges.
//io_returnValues is sorted by rank, then de number.
int halo_exchanges2(Triplet in_Rlattice, Triplet in_Elattice, Triplet in_DOFlattice,
                    Idz in_rankID, Idz in_sz_V, NBN_REAL * io_V,
                    Idz * io_sz_values, rankIndexedValue_t * io_rankedValues);

#endif // NEKO_HALO_UTIL_H
