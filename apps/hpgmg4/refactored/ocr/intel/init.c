
void initializeEvents(u64 level, privateBlock_t * pbPTR){

    s64 level = pbPTR->levels;
    s64 boxsize = pbPTR->boxsize;
    s64 npx = pbPTR->npx;
    s64 myrank = pbPTR->myrank;
    s64 expansion = 1;

    s64 nrank = npx*npx*npx;
    s64 myx = myrank%npx;
    s64 myy = (myrank/npx)%npx;
    s64 myz = myrank/(npx*npx);

    s64 i, j, k;
    s64 lx = myx;
    s64 ly = myy;
    s64 lz = myz;
    s64 lp = npx;

    for(i=levels;i>level;i--)
        if(boxsize > AGGLOMERATION_SIZE) boxsize >>= 1;
          else lp >>= 1;

    expansion = npx/lp;

    for(j=0;j<26;j++) {
        pbPTR->haloSendEVT[level][j] = UNINITIALIZED_GUID;
        pbPTR->haloRecvEVT[level][j] = UNINITIALIZED_GUID;
    }
    for(j=0;j<7;j++) {
        pbPTR->coarsenRecvEVT[level][j] = UNINITIALIZED_GUID;
        pbPTR->refineSendEVT[level][j] = UNINITIALIZED_GUID;
    }
    pbPTR->coarsenSendEVT[level] = UNINITIALIZED_GUID;
    pbPTR->refineRecvEVT[level] = UNINITIALIZED_GUID;


    if((myx%expansion) || (myy%expansion) || (myz%expansion)) continue; //I'm not part of this level

    lx = myx/expansion;
    ly = myy/expansion;
    lz = myz/expansion;

printf("myrank %d level %d myx %d myy %d myz %d lx %d ly %d lz %d \n");

        if(lz == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][1] = NULL_GUID;
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][3] = NULL_GUID;
            pbPTR->haloSendEVT[level][4] = NULL_GUID;
            pbPTR->haloSendEVT[level][5] = NULL_GUID;
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][7] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
        }

        if(lz == lp-1) {
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][18] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
            pbPTR->haloSendEVT[level][20] = NULL_GUID;
            pbPTR->haloSendEVT[level][21] = NULL_GUID;
            pbPTR->haloSendEVT[level][22] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
            pbPTR->haloSendEVT[level][24] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }


        if(ly == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][1] = NULL_GUID;
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][9] = NULL_GUID;
            pbPTR->haloSendEVT[level][10] = NULL_GUID;
            pbPTR->haloSendEVT[level][11] = NULL_GUID;
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][18] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
        }

        if(ly == lp-1) {
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][7] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
            pbPTR->haloSendEVT[level][14] = NULL_GUID;
            pbPTR->haloSendEVT[level][15] = NULL_GUID;
            pbPTR->haloSendEVT[level][16] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
            pbPTR->haloSendEVT[level][24] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }


        if(lx == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][3] = NULL_GUID;
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][9] = NULL_GUID;
            pbPTR->haloSendEVT[level][12] = NULL_GUID;
            pbPTR->haloSendEVT[level][14] = NULL_GUID;
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][20] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
        }

        if(lx == lp-1) {
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][5] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
            pbPTR->haloSendEVT[level][11] = NULL_GUID;
            pbPTR->haloSendEVT[level][13] = NULL_GUID;
            pbPTR->haloSendEVT[level][16] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
            pbPTR->haloSendEVT[level][22] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }


    u32 ind = 0;
    u32 index = 0;
    u32 ind = 0;

        for(k=-1;k<2;k++)
            for(j=-1;j<2;j++)
                for(i=-1;i<2;i++) {
                    if(i==0 && j==0 && k==0) continue; //skip "center" of cube
if(pbPTR->debug > 1) PRINTF("ind %d \n", ind);
                    if(ocrGuidIsNull(pbPTR->haloSendEVT[level][ind])) {
                        pbPTR->haloRecvEVT[level][ind] = NULL_GUID;
                        ind++;
                    } else {

                    ocrGuidFromIndex(&(stickyEVT), sbPTR->haloRangeGUID, 26*nrank*(nlevels - level) + 26*myrank + ind);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    pbPTR->haloSendEVT[level][ind] = stickyEVT;

//receive
                    index = 26*nrank*(nlevels-level) + 26*(myrank + k*npx*npx*expansion + j*npx*expansion + i*expansion) + 25-ind;
PRINTF("I%d i%d j%d k%d ind%d index in %d \n", myrank, i, j, k, ind, index);
                    ocrGuidFromIndex(&(stickyEVT[level]), sbPTR->haloRangeGUID, index);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    pbPTR->haloRecvEVT[level][ind] = stickyEVT;

                    ind++;
                }
        }


for(i=0;i<26;i++) PRINTF("HI%d i %d sendEVT "GUIDF" recvEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->haloSendEVT[level][i]), GUIDA(pbPTR->haloRecvEVT[level][i]));

        if(boxsize != AGGLOMERATION_SIZE) return;

        if((lx%2) || (ly%2) || (lz%2)) {  //my last level
            ind = 4*(lz%2) + 2*(ly%2) + (lx%2);
            ocrGuidFromIndex(&(stickyEVT), sbPTR->coarsenRangeGUID, 7*nrank*(nlevels - level) + 7*myrank + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->coarsenSendEVT = stickyEVT;

            target = 2*(lx/2)*expansion + 2*(ly/2)*npx*expansion + 2*(lz/2)*npx*npx*expansion;
            ocrGuidFromIndex(&(stickyEVT), sbPTR->refineRangeGUID, 7*nrank*(nlevels - level) + 7*target + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->refindRecvEVT = stickyEVT;
        } else {

        ind = 0;

        for(k=0;k<2;k++)
            for(j=0;j<2;j++)
                for(i=0;i<2;i++) {
                    if(i==0 && j==0 && k==0) continue; //don't pair with myself
            ocrGuidFromIndex(&(stickyEVT), sbPTR->refineRangeGUID, 7*nrank*(nlevels - level) + 7*myrank + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->refineSendEVT[ind] = stickyEVT;

            target = myrank + i*expansion + j*npx*expansion + k*npx*npx*expansion;
            ocrGuidFromIndex(&(stickyEVT), sbPTR->agglomRangeGUID, 7*nrank*(nlevels - level) + 7*target + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->refineRecvEVT[ind] = stickyEVT;

            ind++;
            }

    }
for(i=0;i<7;i++) PRINTF("HI%d i %d coarseRecvEVT "GUIDF" refineSendEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->coarseRecvEVT[i]), GUIDA(pbPTR->refineSendEVT[i]));
PRINTF("HI%d i %d coarseSendEVT "GUIDF" refineRecvEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->coarseSendEVT), GUIDA(pbPTR->refineRecvEVT));

}


