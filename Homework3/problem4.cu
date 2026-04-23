#include <stdio.h>
#include <stdlib.h>

__global__ void vectorMatrixAddition(int * vector, int * matrix, int * result)
{
    result[blockIdx.x * 3 + threadIdx.x] = matrix[blockIdx.x * 3 + threadIdx.x] + vector[blockIdx.x];
}

void initMatrix(int * matrix)
{

    matrix[0] = 130;
    matrix[1] = 224;
    matrix[2] =  54;
    matrix[3] = 147;
    matrix[4] = 158;
    matrix[5] = 158;
    matrix[6] = 115;
    matrix[7] = 187;
    matrix[8] = 120;
}

void printMatrix(int * matrix)
{
    int i, j;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            printf("%3d ", matrix[j * 3 + i]);
        }
        printf("\n");
    }
}

void initVector(int * vector)
{
    vector[0] = 221;
    vector[1] =  12;
    vector[2] = 157;
}

int main(int argc, char ** argv)
{
    // Init program variables
    int * hostMatrix,
        * hostVector,
        * hostResult,
        * deviceMatrix,
        * deviceVector,
        * deviceResult,
        matrixMemSize,
        vectorMemSize;
    matrixMemSize = 9 * sizeof(int);
    vectorMemSize = 3 * sizeof(int);
    
    // Alloc and init host matrices
    hostMatrix = (int *)calloc(9, sizeof(int));
    hostVector = (int *)calloc(3, sizeof(int));
    hostResult = (int *)calloc(9, sizeof(int));
    initMatrix(hostMatrix);
    initVector(hostVector);
    printf("Input Vector:\n");
    printMatrix(hostMatrix);

    // Copy data to device memory
    cudaMalloc((void **)&deviceMatrix, matrixMemSize);
    cudaMalloc((void **)&deviceVector, vectorMemSize);
    cudaMalloc((void **)&deviceResult, matrixMemSize);
    cudaMemcpy(deviceMatrix, hostMatrix, matrixMemSize, cudaMemcpyHostToDevice);
    cudaMemcpy(deviceVector, hostVector, vectorMemSize, cudaMemcpyHostToDevice);

    // Perform additional calculation
    vectorMatrixAddition<<<3, 3>>>(deviceVector, deviceMatrix, deviceResult);

    // Copy result back to host memory
    cudaMemcpy(hostResult, deviceResult, matrixMemSize, cudaMemcpyDeviceToHost);
    printf("Output Vector:\n");
    printMatrix(hostResult);
    
    // Clean up
    free(hostMatrix);
    free(hostVector);
    cudaFree(deviceMatrix);
    cudaFree(deviceVector);
    return 0;
}
