#ifndef NEKO_HALO_H
#define NEKO_HALO_H

#include "app_ocr_err_util.h"
#include "nekos_triplet.h"
#include "nbn_setup.h"
#include "app_ocr_debug_util.h"
#include "neko_globals.h"

#include "ocr.h" //ocrGuid_t

//See neko_globals.h::NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
//in order to enable or disable channels for halo exchanges.
//Moved to neko_globals.h --> #define NKEBONE_USE_CHANNEL_FOR_HALO_EXCHANGES
//That minimizes dependencies.

#define SLOTCNT_offset0_channelExchange 5 // The number of slots needed by channelExchange_stop to get its non-channels related DBKs.
                                          // The first offset for channel related DBKs.
#define SLOTCNT_total_channelExchange (SLOTCNT_offset0_channelExchange+NEKbone_regionCount) // The number of slots for usual DBKs
                                                                                            // and channel DBKs.

#define SLOTCNT_offset0_channels4multiplicity 6 // The number of slots needed by nekMultiplicity_stop to get its non-channels related DBKs.
                                                // The first offset for channel related DBKs.
#define SLOTCNT_total_channels4multiplicity (SLOTCNT_offset0_channels4multiplicity + NEKbone_regionCount)

#define SLOTCNT_offset0_channels4setf 7 // The number of slots needed by nekSetF_stop to get its non-channels related DBKs.
                                        // The first offset for channel related DBKs.
#define SLOTCNT_total_channels4setf (SLOTCNT_offset0_channels4setf + NEKbone_regionCount)

#define SLOTCNT_offset0_channels4ax 18 // The number of slots needed by nekCG_axi_stop to get its non-channels related DBKs.
                                        // The first offset for channel related DBKs.
#define SLOTCNT_total_channels4ax (SLOTCNT_offset0_channels4ax + NEKbone_regionCount)

typedef struct NekosTools NEKOtools_t; //Forward declaration

Err_t start_channelExchange(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                            ocrGuid_t * in_haloLabeledGuids, ocrGuid_t * io_futureselfEDT,
                            NEKOglobals_t * in_globals, NEKOglobals_t * o_globals);

Err_t stop_channelExchange(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                           NEKOglobals_t * in_globals, NEKOglobals_t * o_globals);

//==================================
Idz triplet2index(Triplet in_position, Triplet in_lattice);
void index2triplet(Idz in_index, Triplet in_lattice, Triplet * o_x);

Idz elementLocalTriplets2index(Triplet in_dofLattice, Triplet in_Elattice, Triplet in_dof, Triplet in_element);

void posOnNewRank2indices(Triplet in_dofLattice, Triplet in_Elattice, Triplet in_rankLattice,
                     Triplet in_dof, Triplet in_element, Triplet in_rank, Idz * o_dofEleIndex, Idz * o_rankID);

typedef struct rankIndexedValue
{
    Idz rankID;     //From triplet2index and rankLattice
    Idz eleDofID;   //From elementLocalTriplets2index()
    NBN_REAL value;
} rankIndexedValue_t;

//Calculates the maximum entry count in the rankIndexedValue_t vector; NOT in bytes.
Idz calculate_length_rankIndexedValue(unsigned int in_pDOF, Triplet in_Elattice);
void set_rankIndexedValue(rankIndexedValue_t * io_riv, unsigned int in_offset,
                           Idz in_rankID, Idz in_eleDofID, NBN_REAL in_value);
void sort_rankIndexedValue(Idz in_size, rankIndexedValue_t * io_vals);

typedef struct IndexedValue
{
    Idz eleDofID;   //From elementLocalTriplets2index()
    NBN_REAL value;
} IndexedValue_t;

//Calculates the maximum entry count in the IndexedValue_t vector; NOT in bytes.
Idz calculate_length_IndexedValue(unsigned int in_pDOF, Triplet in_Elattice);

//==================================
// halo_exchanges
//Returns zero upon success
//OUTPUTS:
//  = io_V : The rank-local vector of all the DOF values. Upon returning, all rank-local
//           halo exchanges will have been done and applied to it.
//  = io_sz_values : The length of io_rankedValues
//  = io_rankedValues : This will have a sorted-by-rank-first array of rank-to-rank
//                      related modification to be made to the other ranks io_V vectors.
int halo_exchanges(Triplet in_Rlattice, Triplet in_Elattice, unsigned int in_pDOF,
                   unsigned int in_rankID, NBN_REAL * io_V,
                   Idz * io_sz_values, rankIndexedValue_t * io_rankedValues);

Err_t start_halo_multiplicity(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                              Triplet in_Rlattice, Triplet in_Elattice,
                              NEKOglobals_t * in_globals, NBN_REAL * io_V,
                              rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT);

Err_t stop_halo_multiplicity(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                             NBN_REAL * io_V);

Err_t start_halo_setf(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                      Triplet in_Rlattice, Triplet in_Elattice,
                      NEKOglobals_t * in_globals, NBN_REAL * io_V,
                      rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT);

Err_t stop_halo_setf(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                     NBN_REAL * io_V);


Err_t start_halo_ai(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools,
                    Triplet in_Rlattice, Triplet in_Elattice,
                    NEKOglobals_t * in_globals, NBN_REAL * io_V,
                    rankIndexedValue_t * io_riValues, ocrGuid_t * io_destEDT);

Err_t stop_halo_ai(OA_DEBUG_ARGUMENT, NEKOtools_t * in_nekoTools, ocrEdtDep_t * in_depv,
                   NBN_REAL * io_V);






#endif // NEKO_HALO_H
