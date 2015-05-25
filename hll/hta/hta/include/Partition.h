#ifndef __PARTITION_H__
#define __PARTITION_H__

#ifndef HTA_MAX_NUM_PART
#define HTA_MAX_NUM_PART (8)
#endif

#define PART_NO_CHANGE (0)
#define PART_INC  (1)
#define PART_SPLIT (-1)
#define PART_MERGE (-2)
#define PART_NONE (-1)

typedef struct part {
    int num_parts;
    int values[HTA_MAX_NUM_PART];
} Partition;

Partition Partition_create(int num_parts, ...);
// ------------------------------
// Partitioning
// ------------------------------
/// The dynamic partitioning function is applied to one level only.
/// It can change the distribution of tiles if it is applied to the
/// top level. If applied to other levels, it only changes the shape
/// but not the distribution.
HTA* HTA_part(HTA* h, const Partition* src_partition, const Partition* offset);
HTA* HTA_rmpart(HTA *h, const Partition* partition);
/// This function partitions an existing matrix and creates an HTA from it
/// Assume the number of Partitions is the same as dim
/// Assume sorted numbers for partition and dense HTA
HTA* HTA_part_matrix(int dim, void *matrix, Tuple* matrix_size, HTA_SCALAR_TYPE scalar_type, Partition* partitions);
#endif
