#ifndef __SHAPE_H__
#define __SHAPE_H__
///We call the size of the region and the dimension
///of the region collectively as its shape. shape can be viewed as a logical structure with two
///values, the dimension and the size of the region.

// Shape (Region) is a collection of N triplets 
struct shape {
    int dim;        // dimension
    Triplet *dims_;   // pointer to an array of Triplet  
};

typedef struct shape* Shape;

// create a shape (heap allocation)
Shape Shape_create(int dim, ...);

// free a shape
void Shape_destroy();

// make a copy of the shape
Shape Shape_copy(const Shape s);

#endif
