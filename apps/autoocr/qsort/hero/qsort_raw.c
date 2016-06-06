__task void quickSort( int[], int, int);
int partition( int[], int, int);

void main()
{
  __data __attribute__((level(4))) int a[LEN];

  int i;
  for(i = 0; i<LEN; i++) a[i] = rand();
  quickSort( a, 0, LEN-1);
}

void quickSort( int a[], int l, int r)
{
  int j;

  if( l < r )
    {
      // divide and conquer
      j = partition( a, l, r);
      quickSort( a, l, j-1);
      quickSort( a, j+1, r);
    }

}

int partition( int a[], int l, int r) {
  int pivot, i, j, t;
  pivot = a[l];
  i = l; j = r+1;

  while( 1)
    {
      do ++i; while( a[i] <= pivot && i <= r );
      do --j; while( a[j] > pivot );
      if( i >= j ) break;
      t = a[i]; a[i] = a[j]; a[j] = t;
    }
  t = a[l]; a[l] = a[j]; a[j] = t;
  return j;
}
