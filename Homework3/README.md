

## A Note on Problem G2
During my transferring of the Game of Life problem from the first homework to this one, there's been some break in the functionality. I _think_ it's from where my original code was C, but the new code uses C++ for Kokkos. As a result, there's a rank error at run time, breaking the code. I've done my best to go ahead and implement the Kokkos changes, which are from line 290 to line 308 in [problem2g.cpp](./problemg2.cpp).
