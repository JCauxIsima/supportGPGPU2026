// https://godbolt.org/z/8xvh5eKo6

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
    int*** image, 
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

    image[y][x][0] = 0.299 * image[y][x][0] + 0.587 * image[y][x][1] + 0.114 * image[y][x][2];
}

void wrapperFct(
    /*block:*/ dim3 blockDim,
    /*grid: */ dim3 gridDim,
    // Real parameter(s) from here
    int*** image, 
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
    int *** image = new int**[tailleY];
    for (int y = 0; y < tailleY; ++y)
    {
        image[y] = new int*[tailleX];
        for (int x = 0; x < tailleX; ++x)
        {
            image[y][x] = new int[3];
            for (int rgb = 0; rgb < 3; ++rgb)
            {
                image[y][x][rgb] = (x + y * tailleX + rgb * 50 ) % 256;
            }
        }
    }
    std::cout << "image[10][9]-R: " << image[10][9][0] << std::endl;
    std::cout << "image[10][9]-G: " << image[10][9][1] << std::endl;
    std::cout << "image[10][9]-B: " << image[10][9][2] << std::endl;

    const auto blockDim = computeBlockDim(tailleX, tailleY);
    const auto gridDim = computeGridDim(tailleX, tailleY, blockDim);
    wrapperFct(
        blockDim, gridDim,
        // Real parameter(s) from here
        image, tailleX, tailleY);
    std::cout << "image[10][9]-Grey: " << image[10][9][0] << std::endl;
    return 0;
}

