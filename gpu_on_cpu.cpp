
#include <iostream>
#include <vector>

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
    int* tabInOut)
{
    std::cout << "Id: " << threadIdx.x << std::endl;
    tabInOut[threadIdx.x] *= 2;
    std::cout << tabInOut[threadIdx.x] << std::endl;
}

void wrapperFct(
    /*block:*/ dim3 blockDim,
    /*grid: */ dim3 gridDim,
    // Real parameter(s) from here
    int* tabInOut)
{
		// Emulate wrap to 32 threads issues
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
                    tabInOut);
        }
				// Emulate wrap to 32 threads issues
        if (restTo32 != 0)
        {
            for (int missingThread = 0; missingThread < 32 - restTo32; ++missingThread)
            {
            fct_gpu({blockDim.x + missingThread, blockDim.y - 1, blockDim.z - 1}, blockDim,
                    {blockx, blocky, blockz}, gridDim,
                    // Real parameter(s) from here
                    tabInOut);

            }
        }
    }
}

int main()
{
    std::vector<int> tab = { 0, 1, 2, 3};
    wrapperFct(
        /*blockDim=*/{4},
        /*gridDim=*/{},
        // Real parameter(s) from here
        tab.data());

	  for (const auto& val : tab)
    {
        std::cout << "Res: " << val << std::endl;
    }

		return 0;
}
