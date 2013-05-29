#define HAB_C_BLK_SIZE (16)

#define FINISH \
  finish {

#define END_FINISH \
  } // finish

#define FOR_OUTER_0xNx1(index,len) \
    Index_t index ## _len = (len); \
    Index_t index ## _blk = (HAB_C_BLK_SIZE); \
    for (Index_t index ## _out=0; index ## _out < index ## _len; index ## _out+= index ## _blk) { \
      Index_t index ## _end = (index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len;

#define END_FOR_OUTER(index) \
  } // for index ## _out

#define FOR_INNER(index) \
  for (Index_t index = index ## _out; index < index ## _end; index += 1) {

#define END_FOR_INNER(index) \
  } // for index

#define ASYNC_IN_3(three,two,one, ...) \
  async IN( three ## _out, three ## _end, two ## _out, two ## _end, one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_3(three,two,one) \
  }

#define ASYNC_IN_2(two, one, ...) \
  async IN( two ## _out, two ## _end, one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_2(two,one) \
  }

#define ASYNC_IN_1(one, ...) \
  async IN( one ## _out, one ## _end, __VA_ARGS__ ) {

#define END_ASYNC_IN_1(one) \
  }

#define ASYNC_IN_1_INOUT(out, one, ... ) \
  async IN( one ## _out, one ## _end, __VA_ARGS__ ) INOUT out {

#define END_ASYNC_IN_1_INOUT(one) \
  }
