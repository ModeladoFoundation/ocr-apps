#ifndef NEKO_HALO_UTIL_H
#include "neko_halo_util.h"
#endif

#include "app_ocr_err_util.h"

Idz calculate_length_rankIndexedValue(unsigned int in_pDOF, Triplet in_Elattice)
{
    //This assumes the maximum length vector which stores disconnected DOF
    //over the surface of an entire rank.

    //This length is not in byte size, but in actual count.

    //The following calculations are for a 3D lattice.  It will over-estimate
    //the count for 2D lattices.

    Triplet ed = in_Elattice;
    ed.a *= in_pDOF;
    ed.b *= in_pDOF;
    ed.c *= in_pDOF;

    Idz sz =0;
    sz += 8; //1 for each of the 8 corners

    //There are 12 edges.
    sz += 4 * ed.a;
    sz += 4 * ed.b;
    sz += 4 * ed.c;

    //There are 6 faces.
    sz += ed.a * ed.b * 2;
    sz += ed.a * ed.c * 2;
    sz += ed.b * ed.c * 2;

    sz += 1;  //For luck.  But the above count should be exact.

    return sz;
}

int lessthan_rankIndexedValue(rankIndexedValue_t * in_lhs, rankIndexedValue_t * in_rhs)
{
    if(in_lhs->rankID == in_rhs->rankID){
        return (in_lhs->eleDofID < in_rhs->eleDofID);
    }
    return (in_lhs->rankID < in_rhs->rankID);
}

void sort_rankIndexedValue(Idz in_size, rankIndexedValue_t * io_vals)
{
    //A simple insertion sort
    if(in_size<=1 || !io_vals) return;

    Idz i;
    for(i = 1 ; i < in_size; ++i){
        Idz j = i;

        int tf = lessthan_rankIndexedValue(io_vals+j, io_vals+j-1);

        while( j > 0 && tf ){
            rankIndexedValue_t t = io_vals[j];
            io_vals[j]   = io_vals[j-1];
            io_vals[j-1] = t;
            --j;
            if(j>0) tf = lessthan_rankIndexedValue(io_vals+j, io_vals+j-1);
        }
    }
}

int halo_exchanges2(Triplet in_Rlattice, Triplet in_Elattice, Triplet in_DOFlattice,
                    Idz in_rankID, Idz in_sz_V, NBN_REAL * io_V,
                    Idz * io_sz_values, rankIndexedValue_t * io_rankedValues)
{
//    const int debug1 = 0;
//    const int debug_all_candidates = 0;
//    const int debug_output_externalHalo = 0;
    int err=0;
    while(!err){
        Triplet RE = build_latticeRE(in_Elattice, in_Rlattice);

//        if(debug1){
//            char bufR[256] = {0};
//            char bufE[256] = {0};
//            char bufD[256] = {0};
//            char bufRE[256] = {0};
//            printf("lattice D,E,R,RE = %s %s %s %s\n",
//                triplet2text(bufD, in_DOFlattice), triplet2text(bufE, in_Elattice),
//                triplet2text(bufR, in_Rlattice), triplet2text(bufRE, RE) );
//        }

        const Idz endE = abcT(in_Elattice);
        const Idz endD = abcT(in_DOFlattice);

        if(in_sz_V < endE * endD){
            err = __LINE__; break;
        }

        Triplet r = index_to_coords(in_rankID, in_Rlattice);
//        if(debug1){
//            char bufr[256] = {0};
//            printf("Current rank_id, r= %ld %s\n", in_rankID, triplet2text(bufr,r));
//        }

        Idz sz_didates = 0;
        Candidate_t didates[8];

        Idz de; //This is what Nekbone iterates over, within each rank.
        for(de=0; de < in_sz_V; ++de){
            //-----
            Idz e_id = de/endD;
            Idz d_id = de - e_id * endD;

            Triplet e = {0}; //The element coordinates
            Triplet d = {0}; //The pDOF coordinates

            e = index_to_coords(e_id, in_Elattice);
            d = index_to_coords(d_id, in_DOFlattice);

            Triplet re = build_coordinateRE(in_Elattice, e, r);

            //-----
            sz_didates=0;
            didates[0].d  = d;
            didates[0].re = re;
            didates[0].gdid = disconect_to_gdid(d,re, in_DOFlattice,RE);
            didates[0].e = e;
            didates[0].r = r;
            didates[0].rid = in_rankID;
            ++sz_didates;

//            if(debug1){
//                char buf_cand[1024];
//                candidate2text(buf_cand, didates[0]);
//                printf("Current de=%ld\td_id,e_id,r_id=(%ld, %ld, %ld)\t%s\n",
//                   de, d_id, e_id, in_rankID, buf_cand);
//            }

            int cx=0, cy=0, cz=0;
            Triplet tmp_d, tmp_re;
            tmp_d = d;
            tmp_re = re;
            cx = elementMoveX(in_DOFlattice, &tmp_d, &tmp_re);
            if( ! isinLattice(tmp_re, RE)) cx=0; //Only valid because the geometric is convex.
            if(cx){
                cx = sz_didates;
                didates[sz_didates].d  = tmp_d;
                didates[sz_didates].re = tmp_re;
                didates[sz_didates].gdid = disconect_to_gdid(tmp_d, tmp_re, in_DOFlattice, RE);
                unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                ++sz_didates;
            }

            tmp_d = d;
            tmp_re = re;
            cy = elementMoveY(in_DOFlattice, &tmp_d, &tmp_re);
            if( ! isinLattice(tmp_re, RE)) cy=0; //Only valid because the geometric is convex.
            if(cy){
                cy = sz_didates;
                didates[sz_didates].d  = tmp_d;
                didates[sz_didates].re = tmp_re;
                didates[sz_didates].gdid = disconect_to_gdid(tmp_d, tmp_re, in_DOFlattice, RE);
                unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                ++sz_didates;
            }

            tmp_d = d;
            tmp_re = re;
            cz = elementMoveZ(in_DOFlattice, &tmp_d, &tmp_re);
            if( ! isinLattice(tmp_re, RE)) cz=0; //Only valid because the geometric is convex.
            if(cz){
                cz = sz_didates;
                didates[sz_didates].d  = tmp_d;
                didates[sz_didates].re = tmp_re;
                didates[sz_didates].gdid = disconect_to_gdid(tmp_d, tmp_re, in_DOFlattice, RE);
                unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                ++sz_didates;
            }

            if(cx && cy){
                didates[sz_didates].d = didates[0].d;
                didates[sz_didates].d.a = didates[cx].d.a;
                didates[sz_didates].d.b = didates[cy].d.b;
                didates[sz_didates].re = didates[0].re;
                didates[sz_didates].re.a = didates[cx].re.a;
                didates[sz_didates].re.b = didates[cy].re.b;
                if(isinLattice(didates[sz_didates].re, RE)){
                    didates[sz_didates].gdid = disconect_to_gdid(didates[sz_didates].d, didates[sz_didates].re, in_DOFlattice, RE);
                    unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                    didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                    ++sz_didates;
                }
            }
            if(cx && cz){
                didates[sz_didates].d = didates[0].d;
                didates[sz_didates].d.a = didates[cx].d.a;
                didates[sz_didates].d.c = didates[cz].d.c;
                didates[sz_didates].re = didates[0].re;
                didates[sz_didates].re.a = didates[cx].re.a;
                didates[sz_didates].re.c = didates[cz].re.c;
                if(isinLattice(didates[sz_didates].re, RE)){
                    didates[sz_didates].gdid = disconect_to_gdid(didates[sz_didates].d, didates[sz_didates].re, in_DOFlattice, RE);
                    unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                    didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                    ++sz_didates;
                }
            }
            if(cy && cz){
                didates[sz_didates].d = didates[0].d;
                didates[sz_didates].d.b = didates[cy].d.b;
                didates[sz_didates].d.c = didates[cz].d.c;
                didates[sz_didates].re = didates[0].re;
                didates[sz_didates].re.b = didates[cy].re.b;
                didates[sz_didates].re.c = didates[cz].re.c;
                if(isinLattice(didates[sz_didates].re, RE)){
                    didates[sz_didates].gdid = disconect_to_gdid(didates[sz_didates].d, didates[sz_didates].re, in_DOFlattice, RE);
                    unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                    didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                    ++sz_didates;
                }
            }
            if(cx && cy && cz){
                didates[sz_didates].d = didates[0].d;
                didates[sz_didates].d.a = didates[cx].d.a;
                didates[sz_didates].d.b = didates[cy].d.b;
                didates[sz_didates].d.c = didates[cz].d.c;
                didates[sz_didates].re = didates[0].re;
                didates[sz_didates].re.a = didates[cx].re.a;
                didates[sz_didates].re.b = didates[cy].re.b;
                didates[sz_didates].re.c = didates[cz].re.c;
                if(isinLattice(didates[sz_didates].re, RE)){
                    didates[sz_didates].gdid = disconect_to_gdid(didates[sz_didates].d, didates[sz_didates].re, in_DOFlattice, RE);
                    unbuild_coordinateRE(didates[sz_didates].re, in_Elattice, &didates[sz_didates].e, &didates[sz_didates].r);
                    didates[sz_didates].rid = Coords_to_index(didates[sz_didates].r, in_Rlattice);
                    ++sz_didates;
                }
            }

//            if(debug_all_candidates){
//                printf("ALL>\tCandidates:\n");
//                Idz i;
//                for(i=0; i<sz_didates; ++i){
//                    char buff_cand[1024];
//                    candidate2text(buff_cand, didates[i]);
//                    printf("ALL>\t%ld %s\n", i, buff_cand);
//                }
//            }

            if(sz_didates <= 1){
                //Nothing to do since we cannot exchange halo with oneself.
                continue;
            }

            Candidate_t zeroCandidate = didates[0];
            Idz min_gdid = 0x7FFFFFFFL;  //Just a big number
            Idz i;
            for(i=0; i<sz_didates; ++i){
                if( didates[i].rid != in_rankID){
                    //Only survey the candidates for the current rank.
                    continue;
                }
                if(min_gdid > didates[i].gdid){
                    min_gdid = didates[i].gdid;
                }
            }

            if(min_gdid != zeroCandidate.gdid){
                //Only process if current is the smallest gdid.
                //This insures the processing is done only once, per rank.
                continue;
            }

//            if(debug1){
//                char buf_cand[1024];
//                printf("C>\tCandidates:\n");
//                Idz i;
//                for(i=0; i<sz_didates; ++i){
//                    candidate2text(buf_cand, didates[i]);
//                    printf("C>\t%ld %s\n", i, buf_cand);
//                }
//            }

            for(i=0; i<sz_didates; ++i){
                Idz c_d_id = Coords_to_index(didates[i].d, in_DOFlattice);
                Idz c_e_id = Coords_to_index(didates[i].e, in_Elattice);
                Idz c_de = c_d_id + endD * c_e_id;
                didates[i].de = c_de;
            }

            //Do the internal halo exchange
            //Now update the values for the current rank
            //  1) Accumulate all values for the current rank
            //  2) Copy accumulated value to all entries of current rank
            NBN_REAL accumulV = 0;
            for(i=0; i<sz_didates; ++i){
                if( didates[i].rid != in_rankID){
                    continue;
                }
                accumulV += io_V[didates[i].de];
            }

            for(i=0; i<sz_didates; ++i){
                if(didates[i].rid != in_rankID){
                    continue;
                }

                io_V[didates[i].de] = accumulV;
            }

            //Do the external halo exchange
            //Send accumulV to all other candidates in all other ranks.
            for(i=0; i<sz_didates; ++i){
                if(didates[i].rid == in_rankID){
                    continue;
                }
//                if(debug_output_externalHalo){
//                    char bufd[256], bufe[256];
//                    printf("EXHALO> FromR=%ld val=%24.14e ToR:%ld At de=%ld d=%s e=%s\n",
//                           in_rankID, accumulV, didates[i].rid,
//                           didates[i].de,
//                           triplet2text(bufd,didates[i].d),
//                           triplet2text(bufe,didates[i].e)
//                          );
//                }
                io_rankedValues[*io_sz_values].rankID   = didates[i].rid;
                io_rankedValues[*io_sz_values].eleDofID = didates[i].de;
                io_rankedValues[*io_sz_values].value    = accumulV;
                ++(*io_sz_values);
            }
        }//for(de=0;
        IFEB;

        if(*io_sz_values) {
            sort_rankIndexedValue(*io_sz_values, io_rankedValues);
        }

        break;
    }

    return err;
}
