#include <ocxxr-main.hpp>
#include <cstring>

class Grid2D {
 private:
    static ocxxr::RelPtr<double> *_initGrid2D(size_t rows, size_t cols) {
        ocxxr::RelPtr<double> *grid =
                ocxxr::NewArray<ocxxr::RelPtr<double>>(rows);
        for (size_t r = 0; r < rows; r++) {
            grid[r] = ocxxr::NewArray<double>(cols);
        }
        return grid;
    }

 public:
    const size_t rows;
    const size_t cols;
    const ocxxr::RelPtr<const ocxxr::RelPtr<double>> grid;

    Grid2D(size_t rowDim = 5, size_t colDim = 10)
            : rows(rowDim), cols(colDim), grid(_initGrid2D(rowDim, colDim)) {
        int n = 0;
        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < cols; c++) {
                grid[r][c] = n++ / 10.0;
            }
        }
    }

    Grid2D(Grid2D &other)
            : rows(other.rows), cols(other.cols), grid(other.grid) {}

    inline double &at(size_t row, size_t col) const {
        assert(row < rows && col < cols && "Index out of bounds");
        return grid[row][col];
    }
};

#define ARENA_SIZE (1 << 16)

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    // Set up arena datablock
    auto arena = ocxxr::Arena<Grid2D>::Create(ARENA_SIZE);
    // Use arena as current allocator backing-datablock
    ocxxr::SetImplicitArena(arena);
    // Allocate a grid in the datablock
    Grid2D &grid = *ocxxr::New<Grid2D>(10, 10);
    // Is the grid in the datablock?
    void *gridAddr = &grid;
    void *arenaPtr = arena.data_ptr();
    void *arenaEnd = reinterpret_cast<char *>(arenaPtr) + ARENA_SIZE;
    // PRINTF("%p %p %p\n", arenaPtr, gridAddrarenaEnd);
    assert(arenaPtr <= gridAddr && gridAddr <= arenaEnd);
    // Read current grid
    double *data = &grid.at(5, 6);
    PRINTF("Item at orig (5, 6) = %.1f (@ %p)\n", *data, data);
    assert(*data == 5.6);
    // Copy current grid
    auto copy = ocxxr::Arena<Grid2D>::Create(ARENA_SIZE);
    memcpy(copy.base_ptr(), arena.base_ptr(), arena.size());
    Grid2D &grid2 = copy.data();
    // Wipe old grid
    memset(arena.base_ptr(), 0, arena.size());
    PRINTF("Wiped original: val = %.1f (@ %p)\n", *data, data);
    assert(*data == 0.0);
    // Read new grid
    data = &grid2.at(5, 6);
    PRINTF("Item at orig (5, 6) = %.1f (@ %p)\n", *data, data);
    assert(*data == 5.6);
    // Update current arena to the copy (since we clobbered the original)
    ocxxr::SetImplicitArena(copy);
    // Try a non-scalar array
    Grid2D *grids = ocxxr::NewArray<Grid2D>(5);
    PRINTF("Grid of grids (1,2,3) = %.1f\n", grids[1].at(2, 3));
    assert(grids[1].at(2, 3) == 2.3);
    PRINTF("Grid of grids (2,3,4) = %.1f\n", grids[2].at(3, 4));
    assert(grids[2].at(3, 4) == 3.4);
    // Done
    ocxxr::Shutdown();
}
