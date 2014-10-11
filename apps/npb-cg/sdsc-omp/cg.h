#include <inttypes.h>

typedef struct {
    char c;
    uint32_t na;
    uint32_t nonzer;
    uint32_t niter;
    unsigned char on;
    double shift;
    double zvv;
} class_t;

enum {class_S='S', class_W='W', class_A='A', class_B='B', class_C='C', class_D='D', class_E='E', class_U='U'};

void select_class(class_t* c, char class);

double time();

void init_timer(class_t* class, unsigned char on);

void timer_start();

void timer_stop(unsigned char tr);

double timer_read(unsigned char tr);

void free_timer();

double randd();

int randi(int i);

double conj_grad(uint32_t* colidx, uint64_t* rowstr, double* x, double* z, double* a, uint32_t n);

void init_cg(uint32_t n);

void free_cg();

int makea(class_t* class, double** a, uint32_t** colidx, uint64_t* rowstr);

void print_results(class_t* class, double t, double mops);
