#include <time.h>
#include <math.h>
#define NO_MAP
#define ENABLE_EXTENSION_AFFINITY
#include <ocr.h>
#ifndef NO_MAP
//#define ENABLE_EXTENSION_LABELING
#include <extensions/ocr-labeling.h>
#endif
#include <extensions/ocr-affinity.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>
#include <inttypes.h>

#ifndef PRId64
#define PRId64 "lld"
#endif

#ifdef __cplusplus
#include <vector>
#define LOCAL_VAR_ARRAY(TYPE,NAME,SIZE) std::vector<TYPE> NAME(SIZE)
#define LOCAL_VAR_ARRAY_PTR(VAR) (&VAR.front())
#else
#define LOCAL_VAR_ARRAY(TYPE,NAME,SIZE) TYPE NAME[SIZE]
#define LOCAL_VAR_ARRAY_PTR(VAR) (VAR)
#endif

// TORUN GUIDS (for each workerEDT)
#define INDEX_OF_TORUNGUID0 (0)
#define INDEX_OF_TORUNGUID1 (1)
#define SIZE_OF_TORUNGUIDDBK (2)
// END OF TORUN GUIDS

// CONSTGUIDS DB CONSTANTS
#define INDEX_OF_DISTRIBUTETMP (0)
#define INDEX_OF_SEARCHTMP (1)
#define INDEX_OF_APPLYTMP (2)
#define INDEX_OF_STOPTMP (3)
#define INDEX_OF_FINISHTMP (4)
#define INDEX_OF_DISTRIBUTE_MAP (5)	//empty for NO_MAP
#define INDEX_OF_SEARCH_MAP (6)		//empty for NO_MAP
#define INDEX_OF_APPLY_MAP (7)		//empty for NO_MAP
#define INDEX_OF_LOAD_MAP (8)		//empty for NO_MAP
#define INDEX_OF_CREATE_MAP (9)		//empty for NO_MAP
#define INDEX_OF_NEXTEVT_MAP (10)	//empty for NO_MAP
#define INDEX_OF_DATAEVT_MAP (11) 	//empty for NO_MAP
#define INDEX_OF_START_MAP (12)		//empty for NO_MAP
#define INDEX_OF_STOPEDT (13)
#define INDEX_OF_FINISHEDT (14)
#define INDEX_OF_SHUTDOWNEDT (15)
#define INDEX_OF_LOADENDEVT (16)
#ifdef NO_MAP
#define MAX_LEVEL 10
#define INDEX_OF_APPLYEDT(w,level) (17 + (level)*R*C + (w))
#define INDEX_OF_SEARCHEDT(w,level) (17 + MAX_LEVEL*R*C + (level)*R*C + (w))
#define INDEX_OF_DISTRIBUTEEDT(w,level) (17 + 2*MAX_LEVEL*R*C + (level)*R*C + (w))
#define INDEX_OF_DATAEVT(w) (17 + 3*MAX_LEVEL*R*C +(w))
#define INDEX_OF_NEXTEVT(level) (17 + 3*MAX_LEVEL*R*C + R*C + level)
#define SIZE_OF_CONSTGUIDDBK (17 + 3*MAX_LEVEL*R*C + R*C + MAX_LEVEL)
#else
#define SIZE_OF_CONSTGUIDDBK (17)
#endif
// END OF GUID DB CONSTANTS

#define NUM_OF_DEP_LOAD (2)
#define LOAD_SLOT_PARAM (0)
#define LOAD_SLOT_CONSTGUID (1)

#define NUM_OF_DEP_CREATE (2)
#define CREATE_SLOT_PARAM (0)
#define CREATE_SLOT_CONSTGUID (1)

#define NUM_OF_DEP_START (4)
#define START_SLOT_TRIGGER (0)
#define START_SLOT_PARAM (1)
#define START_SLOT_CONSTGUID (2)
#define START_SLOT_TIME (3)

#define NUM_OF_DEP_DISTRIBUTE (5)
#define DISTRIBUTE_SLOT_TORUN (0)
#define DISTRIBUTE_SLOT_PARAM (1)
#define DISTRIBUTE_SLOT_TORUNGUID (2)
#define DISTRIBUTE_SLOT_NEXTEVT (3)
#define DISTRIBUTE_SLOT_CONSTGUID (4)

#define NUM_OF_DEP_SEARCH ((u32)(C+4))
#define SEARCH_SLOT_TORUN(c) ((u32)(c))
#define SEARCH_SLOT_EDGES ((u32)(C))			//knowing R,C,SIZE
#define SEARCH_SLOT_PARAM ((u32)(C+1))			//knowing R,C,SIZE
#define SEARCH_SLOT_PARAM1 (depc-3)				//knowing depc
#define SEARCH_SLOT_TORUNGUID ((u32)(C+2))		//knowing R,C,SIZE
#define SEARCH_SLOT_CONSTGUID ((u32)(C+3))		//knowing R,C,SIZE

#define NUM_OF_DEP_APPLY ((u32)(R+6))
#define APPLY_SLOT_TORUNx(r) ((u32)(r))
#define APPLY_SLOT_VISITED ((u32)(R))			//knowing R,C,SIZE
#define APPLY_SLOT_EDGES ((u32)(R+1))			//knowing R,C,SIZE
#define APPLY_SLOT_PARAM ((u32)(R+2))			//knowing R,C,SIZE
#define APPLY_SLOT_PARAM1 (depc-4)				//knowing depc
#define APPLY_SLOT_TORUNGUID ((u32)(R+3))		//knowing R,C,SIZE
#define APPLY_SLOT_CONSTGUID ((u32)(R+4))		//knowing R,C,SIZE
#define APPLY_SLOT_TORUN ((u32)(R+5))

#define NUM_OF_DEP_FINISH ((u32)(2*R*C+3))
#define FINISH_SLOT_VISITED(w) ((u32)(w))
#define FINISH_SLOT_TORUNGUID(w) ((u32)(R*C+w))	//knowing R,C,SIZE
#define FINISH_SLOT_PARAM ((u32)(2*R*C))		//knowing R,C,SIZE
#define FINISH_SLOT_PARAM1 (depc-3)				//knowing depc
#define FINISH_SLOT_CONSTGUID ((u32)(2*R*C+1))	//knowing R,C,SIZE
#define FINISH_SLOT_FROMSTOP ((u32)(2*R*C+2))	//knowing R,C,SIZE

#define NUM_OF_DEP_STOP ((u32)(R*C+1))
#define STOP_SLOT_FROMAPPLY(w) ((u32)(w))
#define STOP_SLOT_TIME1 (depc-1)				//knowing depc
#define STOP_SLOT_TIME ((u32)(R*C))				//knowing R,C,SIZE

#define NUM_OF_DEP_SHUTDOWN ((u32)(NUMBER_OF_SEARCH+2))
#define SHUTDOWN_SLOT_FROMFINISH(s) ((u32)(s))
#define SHUTDOWN_SLOT_CONSTGUID ((u32)(NUM_OF_DEP_SHUTDOWN-2))
#define SHUTDOWN_SLOT_PARAM ((u32)(NUM_OF_DEP_SHUTDOWN-1))

#define PARAMDBK_MODE DB_MODE_CONST 	//for parallel run
//#define PARAMDBK_MODE DB_MODE_EW		//for serial run

#define ARRAYDBK_MODE DB_MODE_CONST		//do not change adjacency matrix/list
//#define ARRAYDBK_MODE DB_MODE_EW		//may change adjacency matrix/list

//#define EDGE_MODE_MATRIX
#define EDGE_MODE_LIST 0 			// ARRAYDBK_MODE := DB_MODE_CONST
//#define EDGE_MODE_LIST 1			// ARRAYDBK_MODE := DB_MODE_EW

//#define PRINT_DEBUG_INFORMATION
//#define FLOWGRAPH_VISUALIZATION_MODE

#define VALIDATION_MODE 0			//no Validation
//#define VALIDATION_MODE 1			//offline Validation
//#define VALIDATION_MODE 2			//validation in FinishEdt

#define TORUN_DESTROY_MODE

#define FILE_NAME_WRITE "input.dat"
#define FILE_NAME_READ "input.dat"
//#define FILE_NAME_READ "out.dat"
#define CHUNK_FILE_NAME_PATTERN "data/s%d.edgefactor%d.r%d.c%d.worker%05d.dat"
#define SAVE_CHUNKS 1
#define TRY_LOAD_CHUNKS 1
#define TRY_SKIP_GRAPH_GENERATION TRY_LOAD_CHUNKS
//#define HAND_OVER_TORUN_IN_SEARCH		//mostly helpful if rows are on the same node, i.e., the wokers are "transposed"
//#define HAND_OVER_TORUN_IN_DISTRIBUTE	//mostly helpful if rows are on different nodes, i.e., the default layout, with C==comm_size

#if TRY_SKIP_GRAPH_GENERATION==1
#if VALIDATION_MODE>0
#error validation mode requires the graph to be fully generated, not loaded from cache
#endif
#endif

//u8 fakeDbDestroy(ocrGuid_t g){ return 0;}
//#define ocrDbDestroy fakeDbDestroy

#ifdef __cplusplus
struct u48
{
	u16 x[3];
	u48() {}
	u48(u64 other)
	{
		x[0] = u16(other & 0xffff);
		x[1] = u16((other & 0xffff0000) >> 16);
		x[2] = u16((other & 0xffff00000000) >> 32);
		/*u16* p = (u16*)&other;
		assert(p[3] == 0 || p[3] == (u16)-1);
		x[0] = p[0];
		x[1] = p[1];
		x[2] = p[2];*/
	}
	/*u48(const u48& other)
	{
		x[0] = other.x[0];
		x[1] = other.x[1];
		x[2] = other.x[2];
	}*/
	operator u64()
	{
		u64 res;
		res = ((x[0] == (u16)-1 && x[1] == (u16)-1 && x[2] == (u16)-1) ? 0xffff000000000000 : 0) | (u64(x[2]) << 32) | (u64(x[1]) << 16) | u64(x[0]);
		return res;
		/*u16* p = (u16*)&res;
		p[0] = x[0];
		p[1] = x[1];
		p[2] = x[2];
		if (p[0] == (u16)-1 && p[1] == (u16)-1 && p[2] == (u16)-1) p[3] = (u16)-1;
		else p[3] = 0;
		return res;*/
	}
};
bool operator==(u48 l, u48 r)
{
	return l.x[0] == r.x[0] && l.x[1] == r.x[1] && l.x[2] == r.x[2];
}
bool operator==(u48 l, int r)
{
	assert(r == -1);
	return l.x[0] == u16(-1) && l.x[1] == u16(-1) && l.x[2] == u16(-1);
}
bool operator!=(u48 l, u48 r)
{
	return !(l == r);
}
bool operator!=(u48 l, int r)
{
	return !(l == r);
}
bool operator<(u48 l, u48 r)
{
	if (l.x[2] < r.x[2]) return true;
	if (l.x[2] > r.x[2]) return false;
	if (l.x[1] < r.x[1]) return true;
	if (l.x[1] > r.x[1]) return false;
	if (l.x[0] < r.x[0]) return true;
	if (l.x[0] > r.x[0]) return false;
	return false;
}
bool operator<(u48 l, u64 r)
{
	return u64(l) < r;
}
bool operator>(u48 l, u48 r)
{
	return r < l;
}
/*u48 operator/(u48 l, u64 r)
{
	u64* pl = (u64*)&l;
	u64 old = *pl;
	*pl &= 0x0000ffffffffffff;
	u64 res = *pl / r;
	*pl = old;
	return res;
	u8 pow = 0;
	while (r)
	{
		++pow;
		assert((r & 1) == 0 || r==1);
		r >>= 1;
	}
	--pow;
	while (pow--)
	{
		l.x[0] >>= 1;
		l.x[0] |= (l.x[1] & 1) << 15;
		l.x[1] >>= 1;
		l.x[1] |= (l.x[2] & 1) << 15;
		l.x[2] >>= 1;
	}
	return l;
}*/
#endif

typedef u64 vertexType;

bool fitsInVertexType(u64 x)
{
	return ((x >> (4 * sizeof(vertexType))) >> (4 * sizeof(vertexType))) == 0;
}

typedef struct {
	u64 level;		//if !visited level equals -1
	vertexType parent;
}vInfo;			//vInfo = (level, parent)

typedef struct {
	vertexType source;
	vertexType destination;
}edge;			//edge= (source, destination)

typedef struct {
	double kernel_2_time;
	double kernel_1_time;
	u64 kernel_2_nedge;
	u64 kernel_2_nvertex;
	u64 SCALE;
	u64 edgefactor;
	vertexType ROOT;
}evalData;		//evalData

// WORKER
// get worker's row (in the workers 2D array
u64 workerIndex2Row(u64 w, u64 R, u64 C, u64 SIZE) {
	return w / C;
}
// get worker's column (in the workers 2D array
u64 workerIndex2Column(u64 w, u64 R, u64 C, u64 SIZE) {
	return w%C;
}
// index from neighborhood array is transformed to worker
u64 arrayindex2workerid(vertexType u, vertexType v, u64 R, u64 C, u64 SIZE) {
	return (u%R)*C + v / (SIZE / C);
}

//VERTEX
// index in worker's vertex array (its size is vSIZE) is transformed to original vertex number
vertexType vIndex2vertex(u64 w, u64 id, u64 R, u64 C, u64 SIZE) {
	u64 myC = workerIndex2Column(w, R, C, SIZE);
	u64 myR = workerIndex2Row(w, R, C, SIZE);
	return (vertexType)(((SIZE / (R*C))*myC + id)*R + myR);
}
// original vertex number is transformer to worker and index in worker's vertex array (its size is vSIZE)
void vertex2vIndex(vertexType v, u64 * w, u64 * id, u64 R, u64 C, u64 SIZE) {
	u64 y = v % (SIZE / C);
	u64 r = y%R;
	u64 c = v / (SIZE / C);
	*id = y / R;
	*w = r*C + c;
	return;
}
void vertex2vIndex2(vertexType v, u64 * id, u64 R, u64 C, u64 SIZE) {
	u64 y = v % (SIZE / C);
	*id = y / R;
	return;
}

//GRAPH
void fillEdges(FILE* f, u64* M, u64 N, u64 MAX_EDGE_SIZE) {
	struct timeval pt0;
	gettimeofday(&pt0, 0);
	srand((int)pt0.tv_sec);
	for (u64 i = 0; i<MAX_EDGE_SIZE; i++) {
		edge A;
		A.source = (vertexType)(rand() % N);
		A.destination = (vertexType)(rand() % N);
		size_t written = fwrite(&A, sizeof(edge), 1, f);
		assert(written == 1);
#ifdef PRINT_DEBUG_INFORMATION
		PRINTF("[%" PRId64 " %" PRId64 "]\n", A.source, A.destination);
#endif
	}
	*M = MAX_EDGE_SIZE;
}

// sorting edges and vertices
int edgeCompare(const void* vl, const void* vr) {
	vertexType* l = (vertexType*)vl;
	vertexType* r = (vertexType*)vr;
	if (l[0] < r[0]) return -1;
	if (l[0] > r[0]) return 1;
	if (l[1] < r[1]) return -1;
	if (l[1] > r[1]) return 1;
	return 0;
}
int u64Compare(const void* vl, const void* vr) {
	u64 l = *(u64*)vl;
	u64 r = *(u64*)vr;
	if (l < r) return -1;
	if (l > r) return 1;
	return 0;
}

//AUXILIARY FUNKCIONS FOR EDTS
void myAddDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode) { // to achieve nice flow graph
#ifdef FLOWGRAPH_VISUALIZATION_MODE
	ocrGuid_t myEVT;
	ocrEventCreate(&myEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
	ocrAddDependence(myEVT, destination, slot, mode);
	ocrEventSatisfy(myEVT, source);
#else
	ocrAddDependence(source, destination, slot, mode);
#endif
}

ocrGuid_t getDistribute(ocrGuid_t* constguidPTR, u64 level, u64 w, u64 R, u64 C) {
#ifdef NO_MAP
	return constguidPTR[INDEX_OF_DISTRIBUTEEDT(w, level)];
#else
	ocrGuid_t ret;
	ocrGuidFromIndex(&ret, constguidPTR[INDEX_OF_DISTRIBUTE_MAP], w);
	return ret;
#endif
}
ocrGuid_t getSearch(ocrGuid_t* constguidPTR, u64 level, u64 w, u64 R, u64 C) {
#ifdef NO_MAP
	return constguidPTR[INDEX_OF_SEARCHEDT(w, level)];
#else
	ocrGuid_t ret;
	ocrGuidFromIndex(&ret, constguidPTR[INDEX_OF_SEARCH_MAP], w);
	return ret;
#endif
}
ocrGuid_t getApply(ocrGuid_t* constguidPTR, u64 level, u64 w, u64 R, u64 C) {
#ifdef NO_MAP
	return constguidPTR[INDEX_OF_APPLYEDT(w, level)];
#else
	ocrGuid_t ret;
	ocrGuidFromIndex(&ret, constguidPTR[INDEX_OF_APPLY_MAP], w);
	return ret;
#endif
}

#ifdef NO_MAP
u64 getAffinityIndex(u64 w, u64 R, u64 C, u64 affinityCount) {
	return w % affinityCount;
}
#endif

//EDTS
ocrGuid_t loadEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {		//creates graph data for each worker (matrix, visited)
	u64 myId = paramv[0];

	u64* paramPTR = (u64*)depv[LOAD_SLOT_PARAM].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	ocrDbRelease(depv[LOAD_SLOT_PARAM].guid);

	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[LOAD_SLOT_CONSTGUID].ptr;
	ocrGuid_t arrayDBK;

#ifndef EDGE_MODE_LIST
	PRINTF("ERR (loadEdt): this EDGE_MODE is not implemented\n");
	assert(0);
#endif

	vertexType* arrayPTR;
	bool loaded = false;
	if (TRY_LOAD_CHUNKS) {
		char filename[256];
		u64 s = SIZE;
		u64 p = 0;
		while (s > 1) {
			s /= 2;
			++p;
		}
		sprintf(filename, CHUNK_FILE_NAME_PATTERN, (int)p, (int)(EDGE_SIZE / SIZE), (int)R, (int)C, (int)myId);
		FILE* f = fopen(filename, "rb");
		if (f) {
			u64 myEdgeCount;
			u64 read = fread(&myEdgeCount, sizeof(u64), 1, f);
			assert(read == 1);
			ocrDbCreate(&arrayDBK, (void**)&arrayPTR, (1 + 2 * myEdgeCount) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
			assert(fitsInVertexType(myEdgeCount));
			arrayPTR[0] = (vertexType)myEdgeCount;
			if (sizeof(vertexType) == sizeof(u64))
			{
				read = fread(arrayPTR + 1, sizeof(vertexType), 2 * myEdgeCount, f);
				assert(read == 2 * myEdgeCount);
			}
			else
			{
				u64 index = 0;
				while (index < 2*myEdgeCount)
				{
					u64 edge[2];
					read = fread(edge, 2 * sizeof(u64), 1, f);
					assert(read == 1);
					arrayPTR[1 + index++] = (vertexType)edge[0];
					arrayPTR[1 + index++] = (vertexType)edge[1];
				}
			}
			loaded = true;
			fclose(f);

		}
	}
	if (!loaded) {
		FILE *f = fopen(FILE_NAME_READ, "rb");
		assert(f);
		u64 myEdgeCount = 0;
		for (u64 i = 0; i<EDGE_SIZE; i++) {
			edge A;
			size_t read = fread(&A, sizeof(edge), 1, f);
			assert(read == 1);
			if (A.destination == A.source) continue;
			u64 worker1 = arrayindex2workerid(A.source, A.destination, R, C, SIZE);
			u64 worker2 = arrayindex2workerid(A.destination, A.source, R, C, SIZE);
			if (myId == worker1) myEdgeCount++;
			if (myId == worker2) myEdgeCount++;
		}
		assert(fitsInVertexType(myEdgeCount));
		fclose(f);

		ocrDbCreate(&arrayDBK, (void**)&arrayPTR, (1 + 2 * myEdgeCount) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

		f = fopen(FILE_NAME_READ, "rb");
		assert(f);
		arrayPTR[0] = (vertexType)myEdgeCount;
		u64 index = 1;
#ifdef PRINT_DEBUG_INFORMATION
		PRINTF("Load %" PRId64 ": ", myId);
#endif
		for (u64 i = 0; i<EDGE_SIZE; i++) {
			edge A;
			size_t read = fread(&A, sizeof(edge), 1, f);
			assert(read == 1);
			if (A.destination == A.source) continue;
			u64 worker1 = arrayindex2workerid(A.source, A.destination, R, C, SIZE);
			u64 worker2 = arrayindex2workerid(A.destination, A.source, R, C, SIZE);
			if (myId == worker1) {
				arrayPTR[index++] = A.source;
				arrayPTR[index++] = A.destination;
#ifdef PRINT_DEBUG_INFORMATION
				PRINTF("(%" PRId64 ",%" PRId64 ") ", A.source, A.destination);
#endif
			}
			if (myId == worker2) {
				arrayPTR[index++] = A.destination;
				arrayPTR[index++] = A.source;
#ifdef PRINT_DEBUG_INFORMATION
				PRINTF("(%" PRId64 ",%" PRId64 ") ", A.destination, A.source);
#endif
			}
		}
		qsort(arrayPTR + 1, myEdgeCount, 2 * sizeof(vertexType), &edgeCompare);
#ifdef PRINT_DEBUG_INFORMATION
		PRINTF("\n");
#endif
		fclose(f);
		if (SAVE_CHUNKS) {
			assert(sizeof(vertexType) == sizeof(u64));
			char filename[256];
			u64 s = SIZE;
			u64 p = 0;
			while (s > 1) {
				s /= 2;
				++p;
			}
			sprintf(filename, CHUNK_FILE_NAME_PATTERN, (int)p, (int)(EDGE_SIZE / SIZE), (int)R, (int)C, (int)myId);
			FILE* f = fopen(filename, "wb");
			if (f) {
				std::size_t written = fwrite(arrayPTR, sizeof(vertexType), 1 + 2 * myEdgeCount, f);
				assert(written == 1 + 2 * myEdgeCount);
				fclose(f);
			}
		}
	}
	ocrDbRelease(arrayDBK);

	ocrGuid_t dataEVT;
#ifndef NO_MAP
	ocrGuidFromIndex(&dataEVT, constguidPTR[INDEX_OF_DATAEVT_MAP], myId);
	ocrEventCreate(&dataEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG | GUID_PROP_IS_LABELED);
#else
	dataEVT = constguidPTR[INDEX_OF_DATAEVT(myId)];
#endif
	ocrAddDependence(arrayDBK, dataEVT, 0, ARRAYDBK_MODE);

	ocrEventSatisfySlot(constguidPTR[INDEX_OF_LOADENDEVT], NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);

	ocrDbRelease(depv[LOAD_SLOT_CONSTGUID].guid);

	return NULL_GUID;
}

ocrGuid_t createEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) { 	//creates first level data (toRun) and starts first level
	u64 myId = paramv[0];
	vertexType ROOT = (vertexType)paramv[1];
#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("Create %" PRId64 "", myId); fflush(0);
#endif
	u64* paramPTR = (u64*)depv[CREATE_SLOT_PARAM].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	ocrDbRelease(depv[CREATE_SLOT_PARAM].guid);

	u64 ROOT_WORKER;
	u64 ROOT_ID;
	vertex2vIndex(ROOT, &ROOT_WORKER, &ROOT_ID, R, C, SIZE);

	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[CREATE_SLOT_CONSTGUID].ptr;
	ocrGuid_t distributeTMP = constguidPTR[INDEX_OF_DISTRIBUTETMP];
	ocrGuid_t searchTMP = constguidPTR[INDEX_OF_SEARCHTMP];
	ocrGuid_t applyTMP = constguidPTR[INDEX_OF_APPLYTMP];

	// create toRun DBs (for odd and even levels)
	ocrGuid_t toRun0DBK, toRun1DBK;
	vertexType* toRunPTR;
#ifdef TORUN_DESTROY_MODE
	ocrDbCreate(&toRun0DBK, (void**)&toRunPTR, (2) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
#else
	ocrDbCreate(&toRun0DBK, (void**)&toRunPTR, (1+vSIZE) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
#endif // TORUN_DESTROY_MODE
	if (myId == ROOT_WORKER) { toRunPTR[0] = 1; toRunPTR[1] = ROOT; }
	else { toRunPTR[0] = 0; }
	ocrDbRelease(toRun0DBK);
#ifdef TORUN_DESTROY_MODE
	ocrDbCreate(&toRun1DBK, (void**)&toRunPTR, sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
#else
	ocrDbCreate(&toRun1DBK, (void**)&toRunPTR, (1 + vSIZE) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
#endif // TORUN_DESTROY_MODE
	ocrDbRelease(toRun1DBK);

	// store GUIDs of toRun DBs in toRunGuid DB
	ocrGuid_t toRunGuidDBK;
	ocrGuid_t* toRunGuidPTR;
	ocrDbCreate(&toRunGuidDBK, (void**)&toRunGuidPTR, SIZE_OF_TORUNGUIDDBK * sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	toRunGuidPTR[INDEX_OF_TORUNGUID0] = toRun0DBK;
	toRunGuidPTR[INDEX_OF_TORUNGUID1] = toRun1DBK;
	ocrDbRelease(toRunGuidDBK);

	// create visitedDBK for your workers
	ocrGuid_t visitedDBK;
	vInfo* visitedPTR;
	ocrDbCreate(&visitedDBK, (void**)&visitedPTR, (vSIZE) * sizeof(vInfo), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	for (u64 i = 0; i<vSIZE; i++) {
		visitedPTR[i].level = (u64)-1;
		visitedPTR[i].parent = (vertexType)-1;
	}
	// ROOT is already visited
	if (myId == ROOT_WORKER) {
		visitedPTR[ROOT_ID].level = 0;
		visitedPTR[ROOT_ID].parent = ROOT;
	}
	ocrDbRelease(visitedDBK);

#ifdef PRINT_DEBUG_INFORMATION
	if (myId == ROOT_WORKER) PRINTF("*"); fflush(0);
#endif

	// create Level0 (distribute, search, apply)
	ocrGuid_t dEDT, sEDT, aEDT, dataEVT, nextEVT;
	u64 PRM[2];	PRM[0] = myId; PRM[1] = 0; //Level

	dEDT = getDistribute(constguidPTR, 0, myId, R, C);
	sEDT = getSearch(constguidPTR, 0, myId, R, C);
	aEDT = getApply(constguidPTR, 0, myId, R, C);
#ifndef NO_MAP
	ocrGuidFromIndex(&dataEVT, constguidPTR[INDEX_OF_DATAEVT_MAP], myId);
	ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
#else
	dataEVT = constguidPTR[INDEX_OF_DATAEVT(myId)];
	nextEVT = constguidPTR[INDEX_OF_NEXTEVT(0)];
#endif
	ocrDbRelease(depv[CREATE_SLOT_CONSTGUID].guid);


#ifndef NO_MAP
	ocrEdtCreate(&dEDT, distributeTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
	ocrEdtCreate(&sEDT, searchTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
	ocrEdtCreate(&aEDT, applyTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
#endif

	// add dependences at Level 0
	ocrAddDependence(toRun0DBK, dEDT, DISTRIBUTE_SLOT_TORUN, DB_MODE_CONST);
	ocrAddDependence(toRunGuidDBK, dEDT, DISTRIBUTE_SLOT_TORUNGUID, DB_MODE_CONST);
	myAddDependence(visitedDBK, aEDT, APPLY_SLOT_VISITED, DB_MODE_EW);
	ocrAddDependence(dataEVT, sEDT, SEARCH_SLOT_EDGES, DB_MODE_CONST);
	ocrAddDependence(nextEVT, dEDT, DISTRIBUTE_SLOT_NEXTEVT, DB_MODE_NULL);

	myAddDependence(depv[CREATE_SLOT_PARAM].guid, dEDT, DISTRIBUTE_SLOT_PARAM, PARAMDBK_MODE);
	myAddDependence(depv[CREATE_SLOT_CONSTGUID].guid, dEDT, DISTRIBUTE_SLOT_CONSTGUID, DB_MODE_CONST);
	myAddDependence(depv[CREATE_SLOT_CONSTGUID].guid, sEDT, SEARCH_SLOT_CONSTGUID, DB_MODE_CONST);
	myAddDependence(depv[CREATE_SLOT_CONSTGUID].guid, aEDT, APPLY_SLOT_CONSTGUID, DB_MODE_CONST);

	ocrEventSatisfySlot(nextEVT, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);

	return NULL_GUID;
}

ocrGuid_t startEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {		//starts creates for each worker, stops kernel1 timing and starts kernel2 timing
	vertexType ROOT = (vertexType)paramv[0];
	u64 searchId = paramv[1];
	u64 timeDestroy = 0;
	if (searchId + 1 == paramv[2]) timeDestroy = 1;

	u64* paramPTR = (u64*)depv[START_SLOT_PARAM].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	ocrDbRelease(depv[START_SLOT_PARAM].guid);

	struct timeval* pt0 = (struct timeval*)depv[START_SLOT_TIME].ptr;
	if (searchId == 0) {
		u64 t0sec = pt0->tv_sec;
		u64 t0usec = pt0->tv_usec;
		struct timeval t1;
		gettimeofday(&t1, 0);
		double elapsed = (double)((t1.tv_sec - t0sec) * 1000000 + t1.tv_usec - t0usec);
		PRINTF("[kernel1 time %f]\n", elapsed / 1000000); fflush(0);
	}
	// create finish, stop etc.
	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[START_SLOT_CONSTGUID].ptr;

	ocrGuid_t local_affinity = NULL_GUID;
	ocrHint_t* local_hint = NULL_HINT;
	ocrAffinityGetCurrent(&local_affinity);
	ocrHint_t hint;
	ocrHintInit(&hint, OCR_HINT_EDT_T);
	ocrSetHintValue(&hint, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(local_affinity));
	local_hint = &hint;

	u64 af_count;
	ocrAffinityCount(AFFINITY_PD, &af_count);
	assert(af_count >= 1);
	LOCAL_VAR_ARRAY(ocrGuid_t, affinities, af_count);
	ocrAffinityGet(AFFINITY_PD, &af_count, LOCAL_VAR_ARRAY_PTR(affinities));
	LOCAL_VAR_ARRAY(ocrHint_t, hints, af_count);
	for (u64 i = 0; i < af_count; ++i) {
		ocrHintInit(&hints[i], OCR_HINT_EDT_T);
		ocrSetHintValue(&hints[i], OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affinities[i]));
	}

	ocrGuid_t finishTMP = constguidPTR[INDEX_OF_FINISHTMP];
	ocrGuid_t finishEDT;
	ocrEdtCreate(&finishEDT, finishTMP, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, local_hint, NULL);
	constguidPTR[INDEX_OF_FINISHEDT] = finishEDT;
	myAddDependence(depv[START_SLOT_PARAM].guid, finishEDT, FINISH_SLOT_PARAM, PARAMDBK_MODE);

	ocrGuid_t stopTMP = constguidPTR[INDEX_OF_STOPTMP];
	ocrGuid_t stopEDT, stopEVT;
	ocrEdtCreate(&stopEDT, stopTMP, EDT_PARAM_DEF, &timeDestroy, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, local_hint, &stopEVT);
	constguidPTR[INDEX_OF_STOPEDT] = stopEDT;

	ocrAddDependence(stopEVT, finishEDT, FINISH_SLOT_FROMSTOP, DB_MODE_EW);

	PRINTF("START Kernel 2 (id=%" PRId64 ")\n", searchId);

	gettimeofday(pt0, 0);
	ocrDbRelease(depv[START_SLOT_TIME].guid);
	myAddDependence(depv[START_SLOT_TIME].guid, stopEDT, STOP_SLOT_TIME, DB_MODE_EW);

#ifndef NO_MAP
	ocrGuid_t create_map = constguidPTR[INDEX_OF_CREATE_MAP];

	ocrGuid_t nextEVT;
	ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
	ocrEventCreate(&nextEVT, OCR_EVENT_LATCH_T, GUID_PROP_IS_LABELED);
	for (u64 i = 0; i<R*C; i++)
		ocrEventSatisfySlot(nextEVT, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
#endif

	ocrDbRelease(depv[START_SLOT_CONSTGUID].guid);
	myAddDependence(depv[START_SLOT_CONSTGUID].guid, finishEDT, FINISH_SLOT_CONSTGUID, DB_MODE_CONST);

	ocrGuid_t createTMP;
	u32 paramCount = 2;
	ocrEdtTemplateCreate(&createTMP, createEdt, paramCount, NUM_OF_DEP_CREATE);
	u64 PRM[2]; PRM[1] = ROOT;

	for (u64 w = 0; w<R*C; w++) {
		PRM[0] = w;
		ocrGuid_t cEDT;
#ifndef NO_MAP
		ocrGuidFromIndex(&cEDT, create_map, w);
		ocrEdtCreate(&cEDT, createTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
#else
		ocrEdtCreate(&cEDT, createTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &hints[getAffinityIndex(w, R, C, af_count)], NULL);
#endif
		ocrAddDependence(depv[START_SLOT_CONSTGUID].guid, cEDT, CREATE_SLOT_CONSTGUID, DB_MODE_CONST);
		ocrAddDependence(depv[START_SLOT_PARAM].guid, cEDT, CREATE_SLOT_PARAM, PARAMDBK_MODE);
	}


	ocrEdtTemplateDestroy(createTMP);

	return NULL_GUID;
}

ocrGuid_t distributeEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	u64 myId = paramv[0];
	u64 Level = paramv[1];

#ifdef PRINT_DEBUG_INFORMATION
	if (Level == 0) {
		FILE *f = fopen("input.dat", "rb");
		assert(f);
		int cislo = 47;
		fread(&cislo, sizeof(int), 1, f);
		PRINTF("%" PRId64 ": %" PRId64 "\n", myId, cislo);
		fclose(f);
	}
#endif

	u64* paramPTR = (u64*)depv[DISTRIBUTE_SLOT_PARAM].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	//vertexType* toRunPTR = (vertexType*)depv[DISTRIBUTE_SLOT_TORUN].ptr;
#ifndef HAND_OVER_TORUN_IN_DISTRIBUTE
	ocrDbRelease(depv[DISTRIBUTE_SLOT_TORUN].guid);
#endif
	ocrGuid_t* toRunGuidPTR = (ocrGuid_t*)depv[DISTRIBUTE_SLOT_TORUNGUID].ptr;
	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[DISTRIBUTE_SLOT_CONSTGUID].ptr;
	//depv3 event

#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("L%" PRId64 ": distribute%" PRId64 " (", Level, myId);
	PRINTF(GUIDF, GUIDA(depv[DISTRIBUTE_SLOT_TORUN].guid));
	PRINTF(")[size%" PRId64 "]\n", toRunPTR[0]);
#endif

	//create nextEVT if you are worker 0
#ifndef NO_MAP
	if (myId == 0) {
		ocrGuid_t nextEVT;
		ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
		ocrEventCreate(&nextEVT, OCR_EVENT_LATCH_T, GUID_PROP_IS_LABELED);
		for (u64 i = 0; i<R*C; i++)
			ocrEventSatisfySlot(nextEVT, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
	}
#endif

	//send work to all in my row
	u64 myRow = workerIndex2Row(myId, R, C, SIZE);
	u64 myColumn = workerIndex2Column(myId, R, C, SIZE);
	u64 id = myRow*C;
#ifdef HAND_OVER_TORUN_IN_DISTRIBUTE
	ocrGroupBegin();
#endif
	for (u64 col = 0; col<C; col++) {
		//		PRINTF("DAddDep -> %" PRId64 " (sl%" PRId64 ")\n",3*id+1,myColumn);
		ocrGuid_t searchEDT;
		//		ocrGuidFromIndex(&searchEDT, constguidPTR[INDEX_OF_SEARCH_MAP], id);
		searchEDT = getSearch(constguidPTR, Level, id, R, C);
#ifdef HAND_OVER_TORUN_IN_DISTRIBUTE
		ocrAddDependenceByValue(depv[DISTRIBUTE_SLOT_TORUN].guid, searchEDT, SEARCH_SLOT_TORUN(myColumn), DB_MODE_CONST);
#else
		myAddDependence(depv[DISTRIBUTE_SLOT_TORUN].guid, searchEDT, SEARCH_SLOT_TORUN(myColumn), DB_MODE_CONST);
#endif
		id++;
	}
#ifdef HAND_OVER_TORUN_IN_DISTRIBUTE
	ocrDbDestroy(depv[DISTRIBUTE_SLOT_TORUN].guid);
	ocrGroupEnd();
#endif

	//send necessary DBs to SEARCH with myId
	ocrGuid_t myNext;
	myNext = getSearch(constguidPTR, Level, myId, R, C);
	ocrDbRelease(depv[DISTRIBUTE_SLOT_PARAM].guid);
	myAddDependence(depv[DISTRIBUTE_SLOT_PARAM].guid, myNext, SEARCH_SLOT_PARAM, PARAMDBK_MODE);
	ocrDbRelease(depv[DISTRIBUTE_SLOT_TORUNGUID].guid);
	myAddDependence(depv[DISTRIBUTE_SLOT_TORUNGUID].guid, myNext, SEARCH_SLOT_TORUNGUID, DB_MODE_CONST);
	return NULL_GUID;
}

vertexType next_origin(u64 C, u64* counts, vertexType** positions) {
	vertexType mini = (vertexType)-1;
	u64 idx = -1;
	for (u64 c = 0; c < C; c++) {
		if (counts[c] && positions[c][0] < mini)
		{
			mini = positions[c][0];
			idx = c;
		}
	}
	if (idx != -1)
	{
		++positions[idx];
		--counts[idx];
	}
	return mini;
}

ocrGuid_t searchEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	u64 myId = paramv[0];
	u64 Level = paramv[1];

#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("L%" PRId64 ": search%" PRId64 " ", Level, myId);
#endif

	u64* paramPTR = (u64*)depv[SEARCH_SLOT_PARAM1].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];

#ifdef EDGE_MODE_LIST
#if EDGE_MODE_LIST==0
	vertexType* edges = (vertexType*)depv[SEARCH_SLOT_EDGES].ptr;
	u64 myEdgeCount = edges[0];
#else
	PRINTF("ERR(searchEdt): this EDGE_MODE is not implemented\n");
	assert(0);
#endif
#endif

	ocrGuid_t* toRunGuidPTR = (ocrGuid_t*)depv[SEARCH_SLOT_TORUNGUID].ptr;
	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[SEARCH_SLOT_CONSTGUID].ptr;

	LOCAL_VAR_ARRAY(vertexType, destVertices, SIZE / C);	// contains parents of vertices found on this level or -1 if they were not found from searchVertices
	for (u64 i = 0; i<SIZE / C; i++) destVertices[i] = -1;
	LOCAL_VAR_ARRAY(u64, counts, C);
	LOCAL_VAR_ARRAY(vertexType*, positions, C);
	u64 COUNT = 0; // SUM OF ALL MY WORK
	for (u64 c = 0; c < C; c++) {
		vertexType* toRun = (vertexType*)depv[SEARCH_SLOT_TORUN(c)].ptr;
		counts[c] = toRun[0];
		positions[c] = toRun + 1;
		COUNT += toRun[0];
	}

	u64 myRow = workerIndex2Row(myId, R, C, SIZE);
	u64 myColumn = workerIndex2Column(myId, R, C, SIZE);
	vertexType vertex = next_origin(C, LOCAL_VAR_ARRAY_PTR(counts), LOCAL_VAR_ARRAY_PTR(positions));
	u64 lastEdge = 0;
	s64 lastVertex = -1;
	while (vertex != -1) {
		assert((s64)vertex > lastVertex);
		lastVertex = vertex;
#ifdef PRINT_DEBUG_INFORMATION
		PRINTF("%" PRId64 ", ", vertex);
#endif

#ifdef EDGE_MODE_LIST
#if EDGE_MODE_LIST==0
		for (u64 e = lastEdge; e<myEdgeCount; e++) {
			if (edges[2 * e + 1] == vertex) {
				destVertices[edges[2 * e + 2] % (SIZE / C)] = vertex;
			}
			if (edges[2 * e + 1]>vertex) {
				lastEdge = e;
				break;
			}
		}
#else
		PRINTF("this EDGE_MODE is not implemented");
		assert(0);
#endif
#endif
		vertex = next_origin(C, LOCAL_VAR_ARRAY_PTR(counts), LOCAL_VAR_ARRAY_PTR(positions));
	}
	for (u64 c = 0; c < C; c++) {
		assert(counts[c] == 0);
	}
#ifdef PRINT_DEBUG_INFORMATION
	PRINTF(" -> ");
	for (u64 i = 0; i<SIZE / C; i++) {
		if (destVertices[i] != -1) {
			PRINTF("%" PRId64 "(%" PRId64 "),", i, (myColumn)*(SIZE / C) + i);
		}
	}
	PRINTF("\n");
#endif

	//divide the found work to be done on next level among workers in my column
	u64 id = myColumn;
	for (u64 row = 0; row<R; row++) {
		ocrGuid_t toRunxDBK;
		edge* toRunxPTR;
		u64 size = 0;
		for (u64 i = 0; i<vSIZE; i++) {		//vSIZE equals SIZE/(R*C)
			u64 neighbor = i*R + row;
			if (destVertices[neighbor] != -1) {
				size++;
			}
		}
		ocrDbCreate(&toRunxDBK, (void**)&toRunxPTR, (size + 1) * sizeof(edge), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		u64 index = 0;
		for (u64 i = 0; i<vSIZE; i++) {		//vSIZE equals SIZE/(R*C)
			u64 neighbor = i*R + row;
			if (destVertices[neighbor] != -1) {
				toRunxPTR[index + 1].destination = (vertexType)((myColumn)*(SIZE / C) + neighbor);
				toRunxPTR[index + 1].source = destVertices[neighbor];
				index++;
			}
		}
		assert(index == size);
		fitsInVertexType(COUNT);
		toRunxPTR[0].source = (vertexType)COUNT;
		toRunxPTR[0].destination = (vertexType)index;
#ifndef HAND_OVER_TORUN_IN_SEARCH
		ocrDbRelease(toRunxDBK);
#endif

		ocrGuid_t applyEDT;
		applyEDT = getApply(constguidPTR, Level, id, R, C);

#ifdef HAND_OVER_TORUN_IN_SEARCH
		ocrGroupBegin();
		ocrAddDependenceByValue(toRunxDBK, applyEDT, ((u32)myRow), DB_MODE_CONST);
		ocrDbDestroy(toRunxDBK);
		ocrGroupEnd();
#else
		myAddDependence(toRunxDBK, applyEDT, APPLY_SLOT_TORUNx(myRow), DB_MODE_CONST);
#endif
		id += C;
	}

	//send necessary DBs (edgesDBK - slot R+1, paramDBK - slot R+2, guidDBK - slot R+3) to APPLY with myId
	ocrGuid_t myNext;
	myNext = getApply(constguidPTR, Level, myId, R, C);

	if (Level % 2 == 0) assert(!ocrGuidIsNull(toRunGuidPTR[INDEX_OF_TORUNGUID1]));
	else assert(!ocrGuidIsNull(toRunGuidPTR[INDEX_OF_TORUNGUID0]));

#ifdef TORUN_DESTROY_MODE
	ocrAddDependence(NULL_GUID, myNext, APPLY_SLOT_TORUN, DB_MODE_NULL);
#else
	if (Level % 2 == 0) ocrAddDependence(toRunGuidPTR[INDEX_OF_TORUNGUID1], myNext, APPLY_SLOT_TORUN, DB_MODE_EW);
	else ocrAddDependence(toRunGuidPTR[INDEX_OF_TORUNGUID0], myNext, APPLY_SLOT_TORUN, DB_MODE_EW);
#endif

	ocrDbRelease(depv[SEARCH_SLOT_EDGES].guid);
	myAddDependence(depv[SEARCH_SLOT_EDGES].guid, myNext, APPLY_SLOT_EDGES, DB_MODE_CONST);
	ocrDbRelease(depv[SEARCH_SLOT_PARAM].guid);
	myAddDependence(depv[SEARCH_SLOT_PARAM].guid, myNext, APPLY_SLOT_PARAM, PARAMDBK_MODE);
	ocrDbRelease(depv[SEARCH_SLOT_TORUNGUID].guid);
	myAddDependence(depv[SEARCH_SLOT_TORUNGUID].guid, myNext, APPLY_SLOT_TORUNGUID, DB_MODE_EW);

	return NULL_GUID;
}

ocrGuid_t applyEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	u64 myId = paramv[0];
	u64 Level = paramv[1];

	u64* paramPTR = (u64*)depv[APPLY_SLOT_PARAM1].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	ocrDbRelease(depv[APPLY_SLOT_PARAM1].guid);

	vInfo* visitedPTR = (vInfo*)depv[APPLY_SLOT_VISITED].ptr;

	ocrDbRelease(depv[APPLY_SLOT_EDGES].guid);

	ocrGuid_t* toRunGuidPTR = (ocrGuid_t*)depv[APPLY_SLOT_TORUNGUID].ptr;

	u64 SUM_COUNT = 0;
#ifdef TORUN_DESTROY_MODE
	LOCAL_VAR_ARRAY(bool, toRunBool, vSIZE);
	for (u64 i = 0; i<vSIZE; i++) toRunBool[i] = false;

	u64 count = 0;		// count of vertices in toRun for next level
	for (u64 i = 0; i<R; i++) {
		edge* toRunPart = (edge*)depv[APPLY_SLOT_TORUNx(i)].ptr;
		u64 N = toRunPart[0].destination;
		SUM_COUNT += toRunPart[0].source;
		for (u64 j = 0; j<N; j++) {
			vertexType vertex = toRunPart[j + 1].destination;
			u64 vIndex;
			//u64 w;
			vertex2vIndex2(vertex, &vIndex, R, C, SIZE);
			//assert(w == myId);
			if (!toRunBool[vIndex] && visitedPTR[vIndex].level == (u64)-1) {
				toRunBool[vIndex] = true;
				visitedPTR[vIndex].level = Level + 1;
				visitedPTR[vIndex].parent = toRunPart[j + 1].source;
				count++;
			}
		}
		ocrDbRelease(depv[APPLY_SLOT_TORUNx(i)].guid);
		ocrDbDestroy(depv[APPLY_SLOT_TORUNx(i)].guid);
	}

	ocrGuid_t toRunDBK;
	vertexType* toRunPTR;
	ocrDbCreate(&toRunDBK, (void**)&toRunPTR, (1 + count) * sizeof(vertexType), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

	u64 index = 0;
#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("ToRun %" PRId64 "", myId);
#endif
	for (u64 i = 0; i<vSIZE; i++) {
		if (toRunBool[i]) {
			vertexType vertex = vIndex2vertex(myId, i, R, C, SIZE);
			toRunPTR[index + 1] = vertex;
			index++;
#ifdef PRINT_DEBUG_INFORMATION
			PRINTF("%" PRId64 " ", vertex);
#endif
		}
	}
#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("\n");
#endif
	assert(index == count);
	assert(fitsInVertexType(count));
	toRunPTR[0] = (vertexType)count;
	if (Level % 2 == 0) {
#ifndef HAND_OVER_TORUN_IN_DISTRIBUTE
		if (!ocrGuidIsNull(toRunGuidPTR[INDEX_OF_TORUNGUID1])) ocrDbDestroy(toRunGuidPTR[INDEX_OF_TORUNGUID1]);
#endif
		toRunGuidPTR[INDEX_OF_TORUNGUID1] = toRunDBK;
	}
	else {
#ifndef HAND_OVER_TORUN_IN_DISTRIBUTE
		if (!ocrGuidIsNull(toRunGuidPTR[INDEX_OF_TORUNGUID0])) ocrDbDestroy(toRunGuidPTR[INDEX_OF_TORUNGUID0]);
#endif
		toRunGuidPTR[INDEX_OF_TORUNGUID0] = toRunDBK;
	}

	ocrDbRelease(depv[APPLY_SLOT_TORUNGUID].guid);
	ocrDbRelease(toRunDBK);
#else
	vertexType* toRunPTR = (vertexType*)depv[APPLY_SLOT_TORUN].ptr;

	LOCAL_VAR_ARRAY(bool, toRunBool, vSIZE);
	for (u64 i = 0; i<vSIZE; i++) toRunBool[i] = false;

	u64 count = 0;		// count of vertices in toRun for next level
	for (u64 i = 0; i<R; i++) {
		edge* toRunPart = (edge*)depv[APPLY_SLOT_TORUNx(i)].ptr;
		u64 N = toRunPart[0].destination;
		SUM_COUNT += toRunPart[0].source;
		for (u64 j = 0; j<N; j++) {
			vertexType vertex = toRunPart[j + 1].destination;
			u64 vIndex;
			u64 w;
			vertex2vIndex(vertex, &w, &vIndex, R, C, SIZE);
			assert(w == myId);
			if (!toRunBool[vIndex] && visitedPTR[vIndex].level == (u64)-1) {
				toRunBool[vIndex] = true;
				visitedPTR[vIndex].level = Level + 1;
				visitedPTR[vIndex].parent = toRunPart[j + 1].source;
				toRunPTR[++count] = vertex;
			}
		}
		ocrDbDestroy(depv[APPLY_SLOT_TORUNx(i)].guid);
	}
	qsort(toRunPTR + 1, count, sizeof(vertexType), &vertexTypeCompare);
	assert(fitsInVertexType(count));
	toRunPTR[0] = (vertexType)count;
	ocrDbRelease(depv[APPLY_SLOT_TORUN].guid);
#endif

#ifdef PRINT_DEBUG_INFORMATION
	PRINTF("L%" PRId64 ": apply%" PRId64 " SUM=%" PRId64 " Vis ", Level, myId, SUM_COUNT);
	for (u64 i = 0; i<vSIZE; i++) {
		if (visitedPTR[i].level != (u64)-1) PRINTF("%" PRId64 "(%" PRId64 ") {l%" PRId64 "p%" PRId64 "} ", i, vIndex2vertex(myId, i, R, C, SIZE), visitedPTR[i].level, visitedPTR[i].parent);
	}
	PRINTF("\n");
#endif

	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[APPLY_SLOT_CONSTGUID].ptr;
	ocrGuid_t finishEDT = constguidPTR[INDEX_OF_FINISHEDT];
	ocrGuid_t stopEDT = constguidPTR[INDEX_OF_STOPEDT];
	if (myId == 0) PRINTF("%" PRId64 ": %" PRId64 "\n", Level, SUM_COUNT);
	if (SUM_COUNT == 0) { // whole level had nothing to do - THE END
#ifdef HAND_OVER_TORUN_IN_DISTRIBUTE
		ocrDbDestroy(toRunDBK);
#endif

		myAddDependence(depv[APPLY_SLOT_TORUNGUID].guid, finishEDT, FINISH_SLOT_TORUNGUID(myId), DB_MODE_CONST);

		ocrAddDependence(NULL_GUID, stopEDT, STOP_SLOT_FROMAPPLY(myId), DB_MODE_NULL);
		//		PRINTF("AAddDep -> Stop(sl%" PRId64 ")\n",myId);
#if VALIDATION_MODE<2
		u64 cVisited = 0;
		for (u64 i = 0; i<vSIZE; i++)
			if (visitedPTR[i].level != (u64)-1) {
				cVisited++;
#if VALIDATION_MODE==1
				PRINTF("WARNING:Print to file not implemented yet\n");
#endif
			}
		ocrDbDestroy(depv[APPLY_SLOT_VISITED].guid);
		ocrGuid_t cVisitedDBK;
		u64* cVisitedPTR;
		ocrDbCreate(&cVisitedDBK, (void**)&cVisitedPTR, sizeof(u64), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		*cVisitedPTR = cVisited;
		ocrDbRelease(cVisitedDBK);
		myAddDependence(cVisitedDBK, finishEDT, FINISH_SLOT_VISITED(myId), DB_MODE_CONST);
#else
		ocrDbRelease(depv[APPLY_SLOT_VISITED].guid);
		myAddDependence(depv[APPLY_SLOT_VISITED].guid, finishEDT, FINISH_SLOT_VISITED(myId), DB_MODE_CONST);
#endif

		// the Neighborhood matrix should not be destroyed, but if you want to destroy it use (only for last search!!)
		//	ocrDbDestroy(depv[APPLY_SLOT_EDGES].guid);
		//		PRINTF("AAddDep -> Finish(sl%" PRId64 ")\n",0);
		return NULL_GUID;
	}

	// GET TEMPLATES FROM constGuidDBK
	ocrGuid_t distributeTMP = constguidPTR[INDEX_OF_DISTRIBUTETMP];
	ocrGuid_t searchTMP = constguidPTR[INDEX_OF_SEARCHTMP];
	ocrGuid_t applyTMP = constguidPTR[INDEX_OF_APPLYTMP];
	u32 paramCount = 2;
	u64 PRM[2]; PRM[0] = myId; PRM[1] = Level + 1;

	ocrGuid_t dEDT, sEDT, aEDT;
	dEDT = getDistribute(constguidPTR, Level + 1, myId, R, C);
	sEDT = getSearch(constguidPTR, Level + 1, myId, R, C);
	aEDT = getApply(constguidPTR, Level + 1, myId, R, C);

#ifndef NO_MAP
	ocrEdtCreate(&dEDT, distributeTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
	ocrEdtCreate(&sEDT, searchTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
	ocrEdtCreate(&aEDT, applyTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
#endif

	//PRINTF("AAddDep -> %" PRId64 " (sl0-3)\n",3*myId);
#ifdef TORUN_DESTROY_MODE
	myAddDependence(toRunDBK, dEDT, DISTRIBUTE_SLOT_TORUN, DB_MODE_CONST);
#else
	myAddDependence(depv[APPLY_SLOT_TORUN].guid, dEDT, DISTRIBUTE_SLOT_TORUN, DB_MODE_CONST);
#endif
	myAddDependence(depv[APPLY_SLOT_PARAM].guid, dEDT, DISTRIBUTE_SLOT_PARAM, PARAMDBK_MODE);
	myAddDependence(depv[APPLY_SLOT_TORUNGUID].guid, dEDT, DISTRIBUTE_SLOT_TORUNGUID, DB_MODE_CONST);

	//PRINTF("AAddDep -> %" PRId64 " (sl%" PRId64 ")\n",3*myId+1,C);
	myAddDependence(depv[APPLY_SLOT_EDGES].guid, sEDT, SEARCH_SLOT_EDGES, ARRAYDBK_MODE);
	//PRINTF("AAddDep -> %" PRId64 " (sl%" PRId64 ")\n",3*myId+2,R);
	ocrDbRelease(depv[APPLY_SLOT_VISITED].guid);
	myAddDependence(depv[APPLY_SLOT_VISITED].guid, aEDT, APPLY_SLOT_VISITED, DB_MODE_EW);

	ocrGuid_t nextEVT;
#ifndef NO_MAP
	ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
#else
	nextEVT = constguidPTR[INDEX_OF_NEXTEVT(Level + 1)];
#endif

	ocrDbRelease(depv[APPLY_SLOT_CONSTGUID].guid);
	myAddDependence(depv[APPLY_SLOT_CONSTGUID].guid, aEDT, APPLY_SLOT_CONSTGUID, DB_MODE_CONST);
	myAddDependence(depv[APPLY_SLOT_CONSTGUID].guid, sEDT, SEARCH_SLOT_CONSTGUID, DB_MODE_CONST);
	myAddDependence(depv[APPLY_SLOT_CONSTGUID].guid, dEDT, DISTRIBUTE_SLOT_CONSTGUID, DB_MODE_CONST);

	ocrAddDependence(nextEVT, dEDT, DISTRIBUTE_SLOT_NEXTEVT, DB_MODE_NULL);
	ocrEventSatisfySlot(nextEVT, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT);

	return NULL_GUID;
}

ocrGuid_t stopEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	struct timeval* pt0 = (struct timeval*)depv[STOP_SLOT_TIME1].ptr;
	u64 t0sec = pt0->tv_sec;
	u64 t0usec = pt0->tv_usec;

	struct timeval t1;
	gettimeofday(&t1, 0);

	double etime = (double)((t1.tv_sec - t0sec) * 1000000 + t1.tv_usec - t0usec);
	PRINTF("[kernel2 time %f]\n", (etime) / 1000000); fflush(0);

	if (paramv[0] == 1) ocrDbDestroy(depv[STOP_SLOT_TIME1].guid);

	ocrGuid_t eTimeDBK;
	evalData* dat;
	ocrDbCreate(&eTimeDBK, (void**)&dat, sizeof(evalData), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	dat->kernel_2_time = etime;

	ocrDbRelease(eTimeDBK);
	return eTimeDBK;
}

ocrGuid_t finishEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	vertexType ROOT = (vertexType)paramv[0];
	u64 searchId = paramv[1];
	u64 NUMBER_OF_SEARCH = paramv[2];

	u64* paramPTR = (u64*)depv[FINISH_SLOT_PARAM1].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];
	ocrDbRelease(depv[FINISH_SLOT_PARAM].guid);

	evalData* dat = (evalData*)depv[FINISH_SLOT_FROMSTOP].ptr;
	dat->ROOT = ROOT;
	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[FINISH_SLOT_CONSTGUID].ptr;
	ocrGuid_t shutDownEDT = constguidPTR[INDEX_OF_SHUTDOWNEDT];

	for (u64 i = 0; i < R*C; i++){
		ocrGuid_t* toRunPTR = (ocrGuid_t*)depv[FINISH_SLOT_TORUNGUID(i)].ptr;
#ifndef HAND_OVER_TORUN_IN_DISTRIBUTE
		ocrDbDestroy(toRunPTR[INDEX_OF_TORUNGUID0]);
		ocrDbDestroy(toRunPTR[INDEX_OF_TORUNGUID1]);
#endif
		ocrDbDestroy(depv[FINISH_SLOT_TORUNGUID(i)].guid);
	}


#if VALIDATION_MODE<2  //we do not have information for complete validation
	u64 vertex_count = 0;
	for (u64 w = 0; w<R*C; w++) {
		u64* cVisitedPart = (u64*)depv[w].ptr;
		vertex_count = vertex_count + (*cVisitedPart);
	}
	u64 edge_factor = EDGE_SIZE / SIZE;
	u64 edge_count = vertex_count*edge_factor;
#else
	LOCAL_VAR_ARRAY(vInfo, visited, SIZE);
	//complete the partial information from slots 0..R*C-1 and destroy their last toRun0 and toRun1 DBs
	for (u64 w = 0; w<R*C; w++) {
		vInfo* visitedPart = (vInfo*)depv[w].ptr;
		for (u64 id = 0; id<SIZE / (R*C); id++) {
			vertexType vertex = vIndex2vertex(w, id, R, C, SIZE);
			visited[vertex] = visitedPart[id];
		}
	}

	if (SIZE <= 64) {
		for (vertexType i = 0; i < SIZE; i++)
			if (visited[i].level != (u64)-1) PRINTF("%" PRId64 ": level=%" PRId64 " parent=%" PRId64 "\n", i, visited[i].level, visited[i].parent);
	}

	u64 vertex_count = 0;
	for (vertexType i = 0; i < SIZE; i++)
		if (visited[i].level != (u64)-1)	++vertex_count;

	// ROOT has level 0 and parent itself
	if ((visited[ROOT].level != 0) || (visited[ROOT].parent != ROOT))
		PRINTF("ERR: ROOT INCORRECT %" PRId64 "", ROOT);
	assert((visited[ROOT].level == 0) && (visited[ROOT].parent == ROOT));

	// each tree edge connects vertices whose BFS levels differ by exactly one (VAL2), parent having smaller level
	// it implies that the BFS tree is a tree and does not contain cycles (VAL1)
	for (vertexType vertex = 0; vertex<SIZE; vertex++) {
		if (visited[vertex].level != 0 && visited[vertex].level != (u64)-1) {	// not for unvisited and not for root (level0)
			vertexType parent = visited[vertex].parent;
			assert((parent >= 0) && (parent<SIZE));
			if ((visited[vertex].level - visited[parent].level) != 1)
				PRINTF("ERR:PARENT LEVEL DIF vertex %" PRId64 "(l%" PRId64 ") parent %" PRId64 "(l%" PRId64 ")\n", vertex, visited[vertex].level, parent, visited[parent].level);
			assert(visited[vertex].level - visited[parent].level == 1);
		}
	}
	PRINTF("VAL1 OK\nVAL2 OK\n");

	FILE *f = fopen(FILE_NAME_READ, "rb");
	assert(f);
	u64 edge_count = 0;

	for (u64 i = 0; i < EDGE_SIZE; i++) {
		edge A;
		size_t read = fread(&A, sizeof(edge), 1, f);
		if ((visited[A.destination].level != (u64)-1) || (visited[A.source].level != (u64)-1)) {
			// the BFS tree spans an entire connected component's vertices (VAL4)
			if ((visited[A.destination].level == (u64)-1) || (visited[A.source].level == (u64)-1))
				PRINTF("ERR:NOT IN COMP vertex %" PRId64 " or vertex %" PRId64 "\n", A.source, A.destination);
			assert((visited[A.destination].level != (u64)-1) && (visited[A.source].level != (u64)-1));
			// every edge in the input list has vertices with levels that differ by at most one (or that both are not in the BFS tree) (VAL3)
			u64 level1 = visited[A.destination].level;
			u64 level2 = visited[A.source].level;
			if (abs((int)(level1 - level2))>1)
				PRINTF("ERR:EDGE LEVEL DIF vertex %" PRId64 "(l%" PRId64 ") and vertex %" PRId64 "(l%" PRId64 ")\n", A.source, level2, A.destination, level1);
			assert(abs((int)(level1 - level2)) <= 1);
			++edge_count;
		}
	}
	fclose(f);
	PRINTF("VAL3 OK\nVAL4 OK\n");

	// a node and its parent are joined by an edge of the original graph (VAL5)
	f = fopen(FILE_NAME_READ, "rb");
	assert(f);
	for (u64 i = 0; i < EDGE_SIZE; i++) {
		edge A;
		size_t read = fread(&A, sizeof(edge), 1, f);
		if (A.source == A.destination) continue;
		if (visited[A.source].parent == A.destination) {
			assert(visited[A.destination].parent != A.source);
			visited[A.source].level = 0;
		}
		else if (visited[A.destination].parent == A.source) {
			visited[A.destination].level = 0;
		}
	}
	fclose(f);
	for (vertexType i = 0; i<SIZE; i++) {
		if ((visited[i].level != 0) && (visited[i].level != (u64)-1))
			PRINTF("ERR:PARENT-CHILD EDGE MISSING vertex %" PRId64 " and parent %" PRId64 "\n", i, visited[i].parent);
		assert((visited[i].level == 0) || (visited[i].level == (u64)-1));
	}
	PRINTF("VAL5 OK\n");
#endif

	dat->kernel_2_nedge = edge_count;
	dat->kernel_2_nvertex = vertex_count;
	ocrDbRelease(depv[FINISH_SLOT_FROMSTOP].guid);

	//destroy your DBs Visited
	for (u64 i = 0; i<R*C; i++)
		ocrDbDestroy(depv[FINISH_SLOT_VISITED(i)].guid);

	myAddDependence(depv[FINISH_SLOT_FROMSTOP].guid, shutDownEDT, (u32)searchId, DB_MODE_EW);

	ocrGuid_t nextEVT;
#ifndef NO_MAP
	ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
	ocrEventDestroy(nextEVT);
#else
	/*	for (int i=0; i<MAX_LEVEL; i++){
	ocrEventDestroy(constguidPTR[INDEX_OF_NEXTEVT(i)]);
	}*/
#endif

	if (searchId + 1<NUMBER_OF_SEARCH) {
		ocrGuid_t startEVT;
#ifndef NO_MAP
		ocrGuidFromIndex(&startEVT, constguidPTR[INDEX_OF_START_MAP], searchId + 1);
		ocrEventSatisfy(startEVT, NULL_GUID);
#else
		assert(0);	//multiple searches are not supported without maps
#endif
	}
	ocrDbRelease(depv[FINISH_SLOT_CONSTGUID].guid);
	return NULL_GUID;
}

ocrGuid_t shutDownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	u64 NUMBER_OF_SEARCH = paramv[0];

	u64* paramPTR = (u64*)depv[SHUTDOWN_SLOT_PARAM].ptr;
	u64 SIZE = paramPTR[0]; u64 R = paramPTR[1]; u64 C = paramPTR[2]; u64 aSIZE = paramPTR[3]; u64 vSIZE = paramPTR[4];
	u64 EDGE_SIZE = paramPTR[5];

	ocrGuid_t* constguidPTR = (ocrGuid_t*)depv[SHUTDOWN_SLOT_CONSTGUID].ptr;
	double mean = 0;
	for (u64 s = 0; s<NUMBER_OF_SEARCH; s++) {
		evalData* dat = (evalData*)depv[SHUTDOWN_SLOT_FROMFINISH(s)].ptr;

		u64 vertex_count = dat->kernel_2_nvertex;
		u64 edge_count = dat->kernel_2_nedge;
		double elapsed = dat->kernel_2_time;
		mean += 1 / (((double)edge_count) / elapsed);

		PRINTF("--------------SEARCH %" PRId64 "\n", s);
		PRINTF("nodes %" PRId64 " edges %" PRId64 " edge factor %f root %" PRId64 "\n", vertex_count, edge_count, ((double)edge_count) / vertex_count, (u64)dat->ROOT);
		PRINTF("MTEPS %f\n", ((double)edge_count) / elapsed);
		PRINTF("\n");
	}
	PRINTF("mean MTEPS %f\n", NUMBER_OF_SEARCH / mean);
	//BEGIN CLEANING
	// destroy templates and events
	//ocrGuid_t nextEVT;
	//ocrGuidFromIndex(&nextEVT, constguidPTR[INDEX_OF_NEXTEVT_MAP], 0);
	//ocrEventDestroy(nextEVT);
	ocrEdtTemplateDestroy(constguidPTR[INDEX_OF_DISTRIBUTETMP]);
	ocrEdtTemplateDestroy(constguidPTR[INDEX_OF_SEARCHTMP]);
	ocrEdtTemplateDestroy(constguidPTR[INDEX_OF_APPLYTMP]);
	ocrEdtTemplateDestroy(constguidPTR[INDEX_OF_FINISHTMP]);
	ocrEdtTemplateDestroy(constguidPTR[INDEX_OF_STOPTMP]);

	for (u64 i = 0; i<depc; i++)
		ocrDbDestroy(depv[i].guid);
	//END CLEANING

	ocrShutdown();
	return NULL_GUID;
}

#ifdef __cplusplus
extern "C"
#endif
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	if (getArgc(depv[0].ptr) < 5) {
		//PRINTF("%i\n",(int)getArgc(depv[0].ptr));
		//for(int i=0;i<getArgc(depv[0].ptr);++i) PRINTF("%i: %s\n",i,getArgv(depv[0].ptr, i));
		PRINTF("arguments are: SCALE EDGEFACTOR R C\n");
		ocrShutdown();
		return NULL_GUID;
	}
	u64 SCALE = atoi(getArgv(depv[0].ptr, 1));
	u64 EDGEFACTOR = atoi(getArgv(depv[0].ptr, 2));
	// row and column number of workers (the number of workers is R*C)
	u64 R = atoi(getArgv(depv[0].ptr, 3));
	u64 C = atoi(getArgv(depv[0].ptr, 4));

	u64 NUMBER_OF_SEARCH = 1;
#ifdef NO_MAP
	assert(NUMBER_OF_SEARCH == 1);
#endif

	u64 SIZE = (u64)pow(2, SCALE);
	assert(SIZE%R == 0);
	assert(SIZE%C == 0);
	assert(SIZE % (R*C) == 0);
	u64 MAX_EDGE_SIZE = SIZE*EDGEFACTOR;
	u64 EDGE_SIZE;

	PRINTF("STARTING Graph 500 SCALE=%" PRId64 " SIZE=%lld NUMBER_OF_SEARCH=%" PRId64 "\n", SCALE, SIZE, NUMBER_OF_SEARCH); fflush(0);
	PRINTF("WORKERS %" PRId64 ": R=%" PRId64 " C=%" PRId64 "\n", R*C, R, C); fflush(0);

	bool generate = true;
	if (TRY_SKIP_GRAPH_GENERATION) {
		char filename[256];
		sprintf(filename, CHUNK_FILE_NAME_PATTERN, (int)SCALE, (int)(EDGEFACTOR), (int)R, (int)C, (int)0);
		FILE* f = fopen(filename, "rb");
		if (f) {
			generate = false;
			fclose(f);
			PRINTF("will REUSE saved data\n");
		}
	}
#ifdef HAND_OVER_TORUN_IN_SEARCH
	PRINTF("using DB handover in search\n");
#endif
#ifdef HAND_OVER_TORUN_IN_DISTRIBUTE
	PRINTF("using DB handover in distribute\n");
#endif
	if (sizeof(vertexType) == sizeof(u32)) {
		PRINTF("using u32 vertices\n");
	}
#ifdef __cplusplus
	if (sizeof(vertexType) == sizeof(u48)) {
		PRINTF("using u48 C++ vertices\n");
	}
#endif
	if (generate) {
		FILE *f = fopen(FILE_NAME_WRITE, "wb");
		assert(f);
		fillEdges(f, &EDGE_SIZE, SIZE, MAX_EDGE_SIZE);
		fclose(f);
	} else {
		EDGE_SIZE = MAX_EDGE_SIZE;
	}
	PRINTF("EDGE_SIZE=%" PRId64 "\n", EDGE_SIZE);

	u64 aSIZE = (SIZE / C)*(SIZE / R);  	//array size for worker
	u64 vSIZE = SIZE / (R*C);			//vertex amount for worker

	//find a worker who gets ROOT and ROOT's id on that worker
	vertexType ROOT = 4;
	assert(ROOT<SIZE);
	u64 ROOT_WORKER;
	u64 ROOT_ID;
	vertex2vIndex(ROOT, &ROOT_WORKER, &ROOT_ID, R, C, SIZE);

#ifdef PRINT_DEBUG_INFORMATION_MORE
	PRINTF("SIZE=%" PRId64 " EDGE_SIZE=%" PRId64 " asize=%" PRId64 " vsize=%" PRId64 "\n", SIZE, EDGE_SIZE, aSIZE, vSIZE); fflush(0);
	PRINTF("ROOT=%" PRId64 " ROOT_WORKER=%" PRId64 " ROOT_ID=%" PRId64 "\n", ROOT, ROOT_WORKER, ROOT_ID); fflush(0);

	PRINTF("Matrix distributed to workers\n");
	for (u64 u = 0; u<SIZE; u++) {
		for (u64 v = 0; v<SIZE; v++) {
			u64 worker, vertex, neighbor, index;
			arrayindex2worker(u, v, &worker, &vertex, &neighbor, &index, R, C, SIZE);
			PRINTF("(%" PRId64 " %" PRId64 " %" PRId64 " %" PRId64 ") ", worker, vertex, neighbor, index);
		}
		PRINTF("\n");
	}
	PRINTF("\n");

	PRINTF("List of workers and their work from matrix\n");
	for (u64 w = 0; w<R*C; w++) {
		PRINTF("\n%" PRId64 ": ", w);
		for (u64 index = 0; index<aSIZE; index++) {
			u64 vertex, neighbor;
			index2vertexneighbor(index, &vertex, &neighbor, R, C, SIZE);
			u64 i, j;
			worker2arrayindex(w, vertex, neighbor, &i, &j, R, C, SIZE);
			PRINTF("[%" PRId64 " %" PRId64 "] ", i, j);
		}
	}
	PRINTF("\n");
#endif

	ocrGuid_t local_affinity = NULL_GUID;
	ocrHint_t* local_hint = NULL_HINT;
	ocrAffinityGetCurrent(&local_affinity);
	ocrHint_t hint;
	ocrHintInit(&hint, OCR_HINT_EDT_T);
	ocrSetHintValue(&hint, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(local_affinity));
	local_hint = &hint;

#ifdef NO_MAP
	u64 af_count;
	ocrAffinityCount(AFFINITY_PD, &af_count);
	assert(af_count >= 1);
	LOCAL_VAR_ARRAY(ocrGuid_t, affinities, af_count);
	ocrAffinityGet(AFFINITY_PD, &af_count, LOCAL_VAR_ARRAY_PTR(affinities));
	LOCAL_VAR_ARRAY(ocrHint_t, hints, af_count);
	for (u64 i = 0; i < af_count; ++i) {
		ocrHintInit(&hints[i], OCR_HINT_EDT_T);
		ocrSetHintValue(&hints[i], OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(affinities[i]));
	}
	PRINTF("NOT using MAPS, affinity count: %" PRId64 "\n", af_count);
#endif

	struct timeval* pt0;
	ocrGuid_t timeDBK;
	ocrDbCreate(&timeDBK, (void**)&pt0, sizeof(struct timeval), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	gettimeofday(pt0, 0);
	u64 t0sec = pt0->tv_sec;
	u64 t0usec = pt0->tv_usec;
	ocrDbRelease(timeDBK);

	ocrGuid_t paramDBK;
	u64* paramPTR;
	ocrDbCreate(&paramDBK, (void**)&paramPTR, 6 * sizeof(u64), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	paramPTR[0] = SIZE; paramPTR[1] = R; paramPTR[2] = C; paramPTR[3] = aSIZE; paramPTR[4] = vSIZE; paramPTR[5] = EDGE_SIZE;
	ocrDbRelease(paramDBK);

	ocrGuid_t constguidDBK;
	ocrGuid_t* constguidPTR;
	ocrDbCreate(&constguidDBK, (void**)&constguidPTR, (SIZE_OF_CONSTGUIDDBK) * sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

	// TEAMPLATES
	u32 paramCount = 2;
	ocrGuid_t distributeTMP, searchTMP, applyTMP, loadTMP, finishTMP, stopTMP, shutDownTMP, startTMP;
	ocrEdtTemplateCreate(&distributeTMP, distributeEdt, paramCount, NUM_OF_DEP_DISTRIBUTE);
	ocrEdtTemplateCreate(&searchTMP, searchEdt, paramCount, NUM_OF_DEP_SEARCH);
	ocrEdtTemplateCreate(&applyTMP, applyEdt, paramCount, NUM_OF_DEP_APPLY);
	ocrEdtTemplateCreate(&loadTMP, loadEdt, 1, NUM_OF_DEP_LOAD);
	ocrEdtTemplateCreate(&finishTMP, finishEdt, 3, NUM_OF_DEP_FINISH);
	ocrEdtTemplateCreate(&stopTMP, stopEdt, 1, NUM_OF_DEP_STOP);
	ocrEdtTemplateCreate(&shutDownTMP, shutDownEdt, 1, NUM_OF_DEP_SHUTDOWN);
	ocrEdtTemplateCreate(&startTMP, startEdt, 3, NUM_OF_DEP_START);
	constguidPTR[INDEX_OF_FINISHTMP] = finishTMP;
	constguidPTR[INDEX_OF_STOPTMP] = stopTMP;
	constguidPTR[INDEX_OF_DISTRIBUTETMP] = distributeTMP;
	constguidPTR[INDEX_OF_SEARCHTMP] = searchTMP;
	constguidPTR[INDEX_OF_APPLYTMP] = applyTMP;

	//	MAPS
#ifndef NO_MAP
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_DISTRIBUTE_MAP], R*C, GUID_USER_EDT);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_SEARCH_MAP], R*C, GUID_USER_EDT);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_APPLY_MAP], R*C, GUID_USER_EDT);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_LOAD_MAP], R*C, GUID_USER_EDT);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_CREATE_MAP], R*C, GUID_USER_EDT);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_START_MAP], NUMBER_OF_SEARCH, GUID_USER_EVENT_ONCE);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_DATAEVT_MAP], R*C, GUID_USER_EVENT_STICKY);
	ocrGuidRangeCreate(&constguidPTR[INDEX_OF_NEXTEVT_MAP], 1, GUID_USER_EVENT_LATCH);
#else
	for (u64 level = 0; level<MAX_LEVEL; level++) {
		for (u64 w = 0; w<R*C; w++) {
			ocrGuid_t aEDT, sEDT, dEDT;
			u64 PRM[2]; PRM[0] = w; PRM[1] = level;
			ocrEdtCreate(&aEDT, applyTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &hints[getAffinityIndex(w, R, C, af_count)], NULL);
			constguidPTR[INDEX_OF_APPLYEDT(w, level)] = aEDT;
			ocrEdtCreate(&sEDT, searchTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &hints[getAffinityIndex(w, R, C, af_count)], NULL);
			constguidPTR[INDEX_OF_SEARCHEDT(w, level)] = sEDT;
			ocrEdtCreate(&dEDT, distributeTMP, EDT_PARAM_DEF, PRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &hints[getAffinityIndex(w, R, C, af_count)], NULL);
			constguidPTR[INDEX_OF_DISTRIBUTEEDT(w, level)] = dEDT;
		}
	}
	for (u64 w = 0; w<R*C; w++) {
		ocrGuid_t dataEVT;
		ocrEventCreate(&dataEVT, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
		constguidPTR[INDEX_OF_DATAEVT(w)] = dataEVT;
	}
	for (u64 level = 0; level<MAX_LEVEL; level++) {
		ocrGuid_t nextEVT;
		ocrEventCreate(&nextEVT, OCR_EVENT_LATCH_T, EVT_PROP_NONE);
		constguidPTR[INDEX_OF_NEXTEVT(level)] = nextEVT;
		for (u64 i = 0; i<R*C; i++)
			ocrEventSatisfySlot(nextEVT, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
	}
#endif

	// SHUTDOWN EDT
	ocrGuid_t shutDownEDT;
	ocrEdtCreate(&shutDownEDT, shutDownTMP, EDT_PARAM_DEF, &NUMBER_OF_SEARCH, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, local_hint, NULL);
	myAddDependence(paramDBK, shutDownEDT, SHUTDOWN_SLOT_PARAM, PARAMDBK_MODE);
	constguidPTR[INDEX_OF_SHUTDOWNEDT] = shutDownEDT;
	ocrEdtTemplateDestroy(shutDownTMP);

	ocrGuid_t loadEndEVT;
	ocrEventCreate(&loadEndEVT, OCR_EVENT_LATCH_T, EVT_PROP_NONE);
	for (u64 w = 0; w<R*C; w++)
		ocrEventSatisfySlot(loadEndEVT, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT);
	constguidPTR[INDEX_OF_LOADENDEVT] = loadEndEVT;

	ocrGuid_t startMap = constguidPTR[INDEX_OF_START_MAP];
	ocrGuid_t loadMap = constguidPTR[INDEX_OF_LOAD_MAP];
	ocrDbRelease(constguidDBK);
	myAddDependence(constguidDBK, shutDownEDT, SHUTDOWN_SLOT_CONSTGUID, DB_MODE_CONST);

	ocrGuid_t startEDT, startEVT;
	for (u64 searchId = 0; searchId<NUMBER_OF_SEARCH; ++searchId) {
		// START EDT
		u64 PRMs[3]; PRMs[0] = (ROOT + searchId) % SIZE; PRMs[1] = searchId; PRMs[2] = NUMBER_OF_SEARCH;
#ifndef NO_MAP
		ocrGuidFromIndex(&startEVT, startMap, searchId);
		ocrEventCreate(&startEVT, OCR_EVENT_ONCE_T, GUID_PROP_IS_LABELED);
#else
		ocrEventCreate(&startEVT, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
#endif
		ocrEdtCreate(&startEDT, startTMP, EDT_PARAM_DEF, PRMs, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, local_hint, NULL);
		ocrAddDependence(startEVT, startEDT, START_SLOT_TRIGGER, DB_MODE_NULL);
		myAddDependence(paramDBK, startEDT, START_SLOT_PARAM, PARAMDBK_MODE);
		myAddDependence(timeDBK, startEDT, START_SLOT_TIME, DB_MODE_EW);
		myAddDependence(constguidDBK, startEDT, START_SLOT_CONSTGUID, DB_MODE_EW);
	}
	ocrEdtTemplateDestroy(startTMP);

	// LOAD EDTs
	u64 PRMl[1];
#ifndef NO_MAP
	ocrGuid_t firstStartEVT;
	ocrGuidFromIndex(&firstStartEVT, startMap, 0);
	ocrAddDependence(loadEndEVT, firstStartEVT, 0, DB_MODE_NULL);
#else
	ocrAddDependence(loadEndEVT, startEVT, 0, DB_MODE_NULL);
#endif
	for (u64 w = 0; w<R*C; w++) {
		ocrGuid_t lEDT;
		PRMl[0] = w; // worker

					 //create loadEDT and add its dependences
#ifndef NO_MAP
		ocrGuidFromIndex(&lEDT, loadMap, w);
		ocrEdtCreate(&lEDT, loadTMP, EDT_PARAM_DEF, PRMl, EDT_PARAM_DEF, NULL, GUID_PROP_IS_LABELED, NULL_HINT, NULL);
#else
		ocrEdtCreate(&lEDT, loadTMP, EDT_PARAM_DEF, PRMl, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &hints[getAffinityIndex(w, R, C, af_count)], NULL);
#endif
		myAddDependence(constguidDBK, lEDT, LOAD_SLOT_CONSTGUID, DB_MODE_CONST);
		myAddDependence(paramDBK, lEDT, LOAD_SLOT_PARAM, PARAMDBK_MODE);
	}

	ocrEdtTemplateDestroy(loadTMP);
	ocrDbDestroy(depv[0].guid);
	//PRINTF("main end"); fflush(0);
	return NULL_GUID;
}
