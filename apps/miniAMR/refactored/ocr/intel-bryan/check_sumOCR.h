//what do we need going down the tree?
// - event to percolate back up
// - block's data
//
//what do we need coming back up the tree?
// - create an Edt that waits for the next checksum.
// - create events on which that Edt depends.
// -
//




ocrGuid_t setCheckSum( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    //block_t * PRM_block = depv[0].ptr;

    return NULL_GUID;
}

ocrGuid_t evaluateCheckSum_Bottom( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{

    //take all data from each of the inputs.
    //

    //u32 numSums = depc - 1;



    return NULL_GUID;
}

ocrGuid_t evaluateCheckSum_Intermediate( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t * depv )
{
    //take all data from 8 children, sum them and send it on through the channel.


    return NULL_GUID;

}
