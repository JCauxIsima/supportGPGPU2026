// https://godbolt.org/z/c4Tnhqe9E

#include <iostream>
#include <vector>

// Godbolt intermediaire:
// https://godbolt.org/z/vG8GMsTzx
// https://godbolt.org/z/4bo4Mqodn
// https://godbolt.org/z/h5qvx5Wrq

struct dim3
{
    int x =1;
    int y = 1;
    int z = 1;
};

void fct_gpu(
    /*block:*/ dim3 threadIdx, dim3 blockDim,
    /*grid: */ dim3 blockIdx, dim3 gridDim,
    // Real parameter(s) from here
    int* image, 
    int tailleX, int tailleY)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    if (x >= tailleX)
    {
        return;
    }
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    if (y >= tailleY)
    {
        return;
    }

    image[y * tailleX * 3 + x * 3]
        = 0.299 * image[y * tailleX * 3 + x * 3]
            + 0.587 * image[y * tailleX * 3 + x * 3 + 1]
            + 0.114 * image[y * tailleX * 3 + x * 3 + 2];
}

void wrapperFct(
    /*block:*/ dim3 blockDim,
    /*grid: */ dim3 gridDim,
    // Real parameter(s) from here
    int* image, 
    int tailleX, int tailleY)
{
    int restTo32 = (blockDim.z * blockDim.y * blockDim.x) % 32;
    for (int blockz = 0; blockz < gridDim.z; ++blockz)
        for (int blocky = 0; blocky < gridDim.y; ++blocky)
            for (int blockx = 0; blockx < gridDim.x; ++blockx)
    {
        for (int threadz = 0; threadz < blockDim.z; ++threadz)
            for (int thready = 0; thready < blockDim.y; ++thready)
                for (int threadx = 0; threadx < blockDim.x; ++threadx)
        {
            fct_gpu({threadx, thready, threadz}, blockDim,
                    {blockx, blocky, blockz}, gridDim,
                    // Real parameter(s) from here
                    image,
                    tailleX, tailleY);
        }
    }
}

dim3 computeBlockDim(int tailleX, int tailleY)
{
    return {
        /* x: */ std::min(32, tailleX),
        /* y: */ std::min(32, tailleY)
        };
}

dim3 computeGridDim(int tailleX, int tailleY, dim3 blockDim)
{
    return {
       /* x: */ tailleX / blockDim.x
                    + (tailleX % blockDim.x == 0 ? 0 : 1),
       /* y: */ tailleY / blockDim.y
                    + (tailleY % blockDim.y == 0 ? 0 : 1),
    };
}

int main()
{
    int tailleX = 1000;
    int tailleY = 1000;
    int* image = new int[tailleX * tailleY * 3];
    for (int y = 0; y < tailleY; ++y)
    {
        for (int x = 0; x < tailleX; ++x)
        {
            for (int rgb = 0; rgb < 3; ++rgb)
            {
                image[y * tailleX * 3 + x * 3 + rgb] = (x + y * tailleX + rgb * 50 ) % 256;
            }
        }
    }
    std::cout << "image[10][9]-R: " << image[10 * tailleX * 3 + 9 * 3] << std::endl;
    std::cout << "image[10][9]-G: " << image[10 * tailleX * 3 + 9 * 3 + 1] << std::endl;
    std::cout << "image[10][9]-B: " << image[10 * tailleX * 3 + 9 * 3 + 2] << std::endl;

    const auto blockDim = computeBlockDim(tailleX, tailleY);
    const auto gridDim = computeGridDim(tailleX, tailleY, blockDim);
    wrapperFct(
        blockDim, gridDim,
        // Real parameter(s) from here
        image, tailleX, tailleY);
    std::cout << "image[10][9]-Grey: " << image[10 * tailleX * 3 + 9 * 3] << std::endl;
    return 0;
}

