typedef   long  int  nref;
uint32_t genRand(uint32_t M);
void grow_dag_init(int d);
//int addNode_c(void (*edge_adder)(int, int), void (*depth_adder)(int, int), int, int);
nref findMatch_c();
void addNewNode(int (*edge_adder)(int, int), void (*depth_setter)(int,int) );
void free_dag_generator();



