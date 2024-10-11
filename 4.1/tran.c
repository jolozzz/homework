/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}

char trans1_desc[] = "Transpose trail 1";
void trans1(int M, int N, int A[N][M], int B[M][N])
{
    int temp1,temp2,temp3,temp4,
        temp5,temp6,temp7,temp8;

    for(int i = 0;i<N;i+=8){
        for(int j=0;j<M;j+=8){
            for(int ii=i;ii<i+8;ii++){
                temp1 = A[ii][j];
                temp2 = A[ii][j+1];
                temp3 = A[ii][j+2];
                temp4 = A[ii][j+3];
                temp5 = A[ii][j+4];
                temp6 = A[ii][j+5];
                temp7 = A[ii][j+6];
                temp8 = A[ii][j+7];
                B[j][ii] = temp1;
                B[j+1][ii] = temp2;
                B[j+2][ii] = temp3;
                B[j+3][ii] = temp4;
                B[j+4][ii] = temp5;
                B[j+5][ii] = temp6;
                B[j+6][ii] = temp7;
                B[j+7][ii] = temp8;
            }
        }
    }   
}

char trans2_desc[] = "Transpose trail 2";
void trans2(int M, int N, int A[N][M], int B[M][N])
{

    int temp1,temp2,temp3,temp4;

    for(int i = 0;i<N;i+=4){
        for(int j=0;j<M;j+=4){
            for(int ii=i;ii<i+4;ii++){
                temp1 = A[ii][j];
                temp2 = A[ii][j+1];
                temp3 = A[ii][j+2];
                temp4 = A[ii][j+3];
                B[j][ii] = temp1;
                B[j+1][ii] = temp2;
                B[j+2][ii] = temp3;
                B[j+3][ii] = temp4;
                // for(int jj=j;jj<j+4;jj++){
                //     B[jj][ii] = A[ii][jj];
                // }
            }
        }
    }   
}

char trans3_desc[] = "Transpose trail 3";
void trans3(int M, int N, int A[N][M], int B[M][N])
{
    const int blockN = 16;
    const int blockM = 16;

    for(int i=0;i<N;i+=blockN){
        for(int j=0;j<M;j+=blockM){
            for(int ii=i;ii<(N<i+blockN ? N : i+blockN);ii++){
                for(int jj=j;jj<(M<j+blockM ? M : j+blockM);jj++){
                    //printf("%d %d\n",ii,jj);
                    B[jj][ii] = A[ii][jj];
                }
            }
        }
    }
    // for(int i=0;i<N;i++){
    //     for(int j=M-M%blockM;j<M;j++){
    //         B[j][i] = A[i][j];
    //     }
    // }
    // for(int i=N-N%blockN;i<N;i++){
    //     for(int j=0;j<M;j++){
    //         B[j][i] = A[i][j];
    //     }
    // }
    // for(int i=N-N%blockN;i<N;i++){
    //     for(int j=M-M%blockM;j<M;j++){
    //         B[j][i] = A[i][j];
    //     }
    // }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
    registerTransFunction(trans1, trans1_desc);
    registerTransFunction(trans2, trans2_desc);
    registerTransFunction(trans3, trans3_desc);  

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

