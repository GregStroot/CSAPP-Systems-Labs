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

void subblock_routine(int i, int j, int M, int N, int A[N][M], int B[M][N]);
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
    int i, j;
    int ii;
    int v0, v1, v2, v3, v4, v5, v6, v7;


    //We are doing blocks of 8x8
    int nSteps = N/8;
    int mSteps = M/8;


    for (i = 0; i < nSteps; i++) {
        for (j = 0; j < mSteps; j++) {

            for (ii = 0; ii < 8; ii++) {
                v0 = A[i*8 + ii][j*8 + 0];
                v1 = A[i*8 + ii][j*8 + 1];
                v2 = A[i*8 + ii][j*8 + 2];
                v3 = A[i*8 + ii][j*8 + 3];
                v4 = A[i*8 + ii][j*8 + 4];
                v5 = A[i*8 + ii][j*8 + 5];
                v6 = A[i*8 + ii][j*8 + 6];
                v7 = A[i*8 + ii][j*8 + 7];

                B[j*8 + 0][i*8 + ii] = v0;
                B[j*8 + 1][i*8 + ii] = v1;
                B[j*8 + 2][i*8 + ii] = v2;
                B[j*8 + 3][i*8 + ii] = v3;
                B[j*8 + 4][i*8 + ii] = v4;
                B[j*8 + 5][i*8 + ii] = v5;
                B[j*8 + 6][i*8 + ii] = v6;
                B[j*8 + 7][i*8 + ii] = v7;
            }

        }
    }

}

char transpose_unblocked_desc[] = "Transpose unblocked";
void transpose_unblocked(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    int ii, jj;


    //We are doing blocks of 8x8
    int nSteps = N/8;
    int mSteps = M/8;


    for (i = 0; i < nSteps; i++) {
        for (j = 0; j < mSteps; j++) {

            for (ii = 0; ii < 8; ii++) {
                for (jj = 0; jj < 8; jj++) {
                    tmp = A[i*8 + ii][j*8 + jj];
                    B[j*8 + jj][i*8 + ii] = tmp;
                }
            }

        }
    }

}

char transpose_unblocked_wa_desc[] = "Transpose unblocked wrap-around val";
void transpose_unblocked_wa(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    int ii, jj;


    //We are doing blocks of 4x4
    int nSteps = N/4;
    int mSteps = M/4;


    for (i = 0; i < nSteps; i++) {
        for (j = 0; j < mSteps; j++) {

            for (ii = 0; ii < 4; ii++) {
                for (jj = 0; jj < 4; jj++) {
                    tmp = A[i*4 + ii][j*4 + jj];
                    B[j*4 + jj][i*4 + ii] = tmp;
                }
            }

        }
    }

}

char transpose_subroutine_desc[] = "Transpose unblocked 8x8 sub-block routine";
void transpose_subroutine(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;


    //We are doing blocks of 4x4
    int nSteps = N/8;
    int mSteps = M/8;


    for (i = 0; i < nSteps; i++) {
        for (j = 0; j < mSteps; j++) {
            subblock_routine(i,j, M,N, A, B);
        }
    }

}

void subblock_routine(int i, int j, int M, int N, int A[N][M] ,int B[M][N]) {

    int v0, v1, v2, v3, v4, v5, v6, v7;
    int k;

    //Flip Q1 and store extra data in Q2 (as a buffer)
    for (k = 0; k < 4; k++ ){
        v0 = A[i*8 + k][j*8 + 0];
        v1 = A[i*8 + k][j*8 + 1];
        v2 = A[i*8 + k][j*8 + 2];
        v3 = A[i*8 + k][j*8 + 3];
        v4 = A[i*8 + k][j*8 + 4];
        v5 = A[i*8 + k][j*8 + 5];
        v6 = A[i*8 + k][j*8 + 6];
        v7 = A[i*8 + k][j*8 + 7];

        //Transpose top left
        B[j*8 + 0][i*8 + k] = v0;
        B[j*8 + 1][i*8 + k] = v1;
        B[j*8 + 2][i*8 + k] = v2;
        B[j*8 + 3][i*8 + k] = v3;

        //Store extra data
        B[j*8 + 0][i*8 + k + 4] = v4;
        B[j*8 + 1][i*8 + k + 4] = v5;
        B[j*8 + 2][i*8 + k + 4] = v6;
        B[j*8 + 3][i*8 + k + 4] = v7;
    }

    //Flip Q2 and Q3
    //Average of 2 misses per loop
    for (k = 0; k < 4; k++ ){
        //Load row0 of Q2 into registers (single set!)
        v0 = B[j*8 + k][i*8 + 4];
        v1 = B[j*8 + k][i*8 + 5];
        v2 = B[j*8 + k][i*8 + 6];
        v3 = B[j*8 + k][i*8 + 7];

        //Q3: Load the column of A (to replace data above in Q2)
        //4 sets
        v4 = A[i*8 + 4][j*8 + k];
        v5 = A[i*8 + 5][j*8 + k];
        v6 = A[i*8 + 6][j*8 + k];
        v7 = A[i*8 + 7][j*8 + k];

        //Write to Q2 (miss)
        B[j*8 + k][i*8 + 4] = v4;
        B[j*8 + k][i*8 + 5] = v5;
        B[j*8 + k][i*8 + 6] = v6;
        B[j*8 + k][i*8 + 7] = v7;

        //Write to Q3 (miss)
        B[j*8 + k + 4][i*8 + 0] = v0;
        B[j*8 + k + 4][i*8 + 1] = v1;
        B[j*8 + k + 4][i*8 + 2] = v2;
        B[j*8 + k + 4][i*8 + 3] = v3;
    }

    //Q4
    for (k = 4; k < 8; k++ ){
        v4 = A[i*8 + k][j*8 + 4];
        v5 = A[i*8 + k][j*8 + 5];
        v6 = A[i*8 + k][j*8 + 6];
        v7 = A[i*8 + k][j*8 + 7];

        //Transpose top left
        B[j*8 + 4][i*8 + k] = v4;
        B[j*8 + 5][i*8 + k] = v5;
        B[j*8 + 6][i*8 + k] = v6;
        B[j*8 + 7][i*8 + k] = v7;
    }


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
        //Stride 1 on A
        //Stide N on B
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

    registerTransFunction(transpose_unblocked, transpose_unblocked_desc);
    registerTransFunction(transpose_unblocked_wa, transpose_unblocked_wa_desc);
    registerTransFunction(transpose_subroutine, transpose_subroutine_desc);

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

