
void addDepsAndSatisfy( ocrGuid_t childGUID, ocrGuid_t * rcvs, ocrGuid_t * snds )
{
    u64 i;

    for( i = 1; i < 7; i++ ){
        ocrAddDependence( rcvs[i-1], childGUID, i, DB_MODE_RW );
    }

    for( i = 0; i < 6; i++ ){
        ocrEventSatisfy( snds[i], NULL_GUID );
    }

}
