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

void subblock_routine(int* i, int* j, int* M, int* N, int* A[N][M] ,int* B[M][N]) {

    int v0, v1, v2, v3, v4, v5, v6, v7;
    int ii;

    /////
    //Top left quadrant
    /////
    for (ii = 0; ii < 4; ii++ ){
        v0 = A[*i*8 + ii][*j*8 + 0];
        v1 = A[*i*8 + ii][*j*8 + 1];
        v2 = A[*i*8 + ii][*j*8 + 2];
        v3 = A[*i*8 + ii][*j*8 + 3];
        v4 = A[*i*8 + ii][*j*8 + 4];
        v5 = A[*i*8 + ii][*j*8 + 5];
        v6 = A[*i*8 + ii][*j*8 + 6];
        v7 = A[*i*8 + ii][*j*8 + 7];

        //Transpose top left
        B[*j*8 + 0][*i*8 + ii] = v0;
        B[*j*8 + 1][*i*8 + ii] = v1;
        B[*j*8 + 2][*i*8 + ii] = v2;
        B[*j*8 + 3][*i*8 + ii] = v3;

        //Store extra data
        B[*j*8 + 0][*i*8 + ii + 4] = v4;
        B[*j*8 + 1][*i*8 + ii + 4] = v5;
        B[*j*8 + 2][*i*8 + ii + 4] = v6;
        B[*j*8 + 3][*i*8 + ii + 4] = v7;

    }

    // TODO

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

