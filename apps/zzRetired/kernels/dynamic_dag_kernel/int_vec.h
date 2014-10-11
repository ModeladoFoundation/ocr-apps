
struct IntVec
{

  int* arr; //the dynamic array;
  int num_elements;
  int num_allocated;
}IntVec;

/* vector of intvec */

struct Vec_IntVec
{
  struct IntVec** arr;
  int num_elements;
  int num_allocated;

};

struct IntVec* construct_IntVec();

void add_int(struct IntVec* a, int b);

void add_intvec(struct Vec_IntVec* a, struct IntVec* b);

void free_Vec_IntVec(struct Vec_IntVec* a);
