namespace shuffle_ds
{
  int num_cores;
  int scale;
  int64_t num_nodes;
  int64_t bucket_extant;
  int64_t bucket_sz;
  int64_t bin_extant; /* space reserved per bin */
  int64_t bin_sz;
  int64_t num_bins;
  int64_t num_bins_per_core;
  int64_t nugget_sz;
  int64_t nugget_extant;
  int rounds;
  char bucket_fn[500] = "bucket.dat";
  char cbucket_fn[500] = "cbucket.dat";
}
