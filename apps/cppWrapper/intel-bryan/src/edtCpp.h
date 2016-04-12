#include <vector>

class EdtProducer { //meant to be a producer EDT.

    public:

        EdtProducer( );
        EdtProducer( ocrEdt_t, u32, u32, bool );
        EdtProducer( ocrGuid_t, u32, u32, bool );

        /*-------------member functions--------------*/
        ocrGuid_t create( );
        /*-----------end member functions------------*/

        /*-------------getters/setters---------------*/
        void        setDepSize( u64 ds ){ depSize = ds; }
        void        setParamSize( u64 ps ){ paramSize = ps; }
        void        setFuncPtr( ocrEdt_t );
        void        setParams( u64 * );

        u64         getDepSize( void ){ return depSize; }
        u64         getParamSize( void ){ return paramSize; }
        ocrGuid_t   getTML( void ){ return TML; }
        ocrGuid_t   getFinishEVT( void ){ return finishEVT; }
        /*-----------end getters/setters-------------*/

    private:
        ocrGuid_t TML;
        ocrGuid_t finishEVT;
        u64 depSize;
        u64 paramSize;
        bool finish;
        u64 params[512];        //just take the extra space for now, to avoid extra allocation. This
                                //will change when the change to unsigned char* params introduced.

};

class EdtLauncher {

    public:
        /*---------------Constructors----------------*/
        EdtLauncher( ){ edtGUID = NULL_GUID; }
        EdtLauncher( EdtProducer, ocrGuid_t );
        /*-------------End Constructors--------------*/


        /*-------------member functions--------------*/
        u32 depPush( ocrGuid_t );
        void launch( );
        void reset( );
        /*-----------end member functions------------*/


        /*-------------getters/setters---------------*/
        ocrGuid_t getActiveEdt( ){ return edtGUID; }

        /*-----------end getters/setters-------------*/

    private:
        ocrGuid_t edtGUID;
        u32 depSize;
        std::vector<ocrGuid_t> deps;
};
