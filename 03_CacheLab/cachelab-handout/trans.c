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
#define min(a, b) (((a) < (b)) ? (a) : (b))

void subblock_routine(int i, int j, int M, int N, int A[N][M], int B[M][N]);
void edge_routine(int i, int j, int M, int N, int A[N][M] ,int B[M][N]);
void symmetric_routine(int M, int N, int A[N][M] ,int B[M][N]);
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_simple_buffer_desc[] = "Simple 8x8 with register buffer transpose";
void transpose_simple_buffer(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int ii;
    int v0, v1, v2, v3, v4, v5, v6, v7;


    //We are doing blocks of 8x8


    for (i = 0; i < N; i+=8) {
        for (j = 0; j < M; j+=8) {

            if (((N-i) >= 8) && ((M-j) >= 8)) {
                for (ii = 0; ii < 8; ii++) {
                    v0 = A[i + ii][j + 0];
                    v1 = A[i + ii][j + 1];
                    v2 = A[i + ii][j + 2];
                    v3 = A[i + ii][j + 3];
                    v4 = A[i + ii][j + 4];
                    v5 = A[i + ii][j + 5];
                    v6 = A[i + ii][j + 6];
                    v7 = A[i + ii][j + 7];

                    B[j + 0][i + ii] = v0;
                    B[j + 1][i + ii] = v1;
                    B[j + 2][i + ii] = v2;
                    B[j + 3][i + ii] = v3;
                    B[j + 4][i + ii] = v4;
                    B[j + 5][i + ii] = v5;
                    B[j + 6][i + ii] = v6;
                    B[j + 7][i + ii] = v7;
                }
            }
            else {
                int tmp;
                for (int ii=i; ii < min(N, i+8); ii++) {
                    for (int jj=j; jj < min(M, j+8); jj++) {
                        tmp = A[ii][jj];
                        B[jj][ii] = tmp;
                    }
                }
            }

        }
    }

}

char transpose_unblocked_desc[] = "Transpose unblocked";
void transpose_unblocked(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    int ii, jj;



    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            if (((N-i) >= 8) && ((M-j) >= 8)) {
                for (ii = 0; ii < 8; ii++) {
                    for (jj = 0; jj < 8; jj++) {
                        tmp = A[i + ii][j + jj];
                        B[j + jj][i + ii] = tmp;
                    }
                }
            }
            else {
                int tmp;
                for (int ii=i; ii < min(N, i+8); ii++) {
                    for (int jj=j; jj < min(M, j+8); jj++) {
                        tmp = A[ii][jj];
                        B[jj][ii] = tmp;
                    }
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


    for (i = 0; i < N; i += 4) {
        for (j = 0; j < M; j+= 4) {

            if (((N-i) >= 4) && ((M-j) >= 4)) {
                for (ii = 0; ii < 4; ii++) {
                    for (jj = 0; jj < 4; jj++) {
                        tmp = A[i + ii][j + jj];
                        B[j + jj][i + ii] = tmp;
                    }
                }
            }
            else {
                for (int ii=i; ii < min(N, i+4); ii++) {
                    for (int jj=j; jj < min(M, j+4); jj++) {
                        tmp = A[ii][jj];
                        B[jj][ii] = tmp;
                    }
                }
            }

        }
    }

}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (N == 32 && M == 32) {
        transpose_simple_buffer(M, N, A, B);
    }
    else if (N == M) {
        symmetric_routine(M, N, A, B);
    }
    else {
        //When not symmetric, the offsets can protect us from too many conflicts)
        int i,j;
        for (i=0; i < N; i += 16) {
            for (j=0; j < M; j += 16) {
                edge_routine(i,j, M, N, A, B);
                edge_routine(i+8,j, M, N, A, B);
                edge_routine(i,j+8, M, N, A, B);
                edge_routine(i+8,j+8, M, N, A, B);
            }
        }

    }

}

void symmetric_routine(int M, int N, int A[N][M] ,int B[M][N]){
    int i, j;

    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            if (((N-i) >= 8) && ((M-j) >= 8)) {
                subblock_routine(i,j, M,N, A, B);
            }
            else {
                edge_routine(i,j, M,N, A, B);
            }
        }
    }
}

void edge_routine(int i, int j, int M, int N, int A[N][M] ,int B[M][N]) {
    if ((i > N) || (j > M)) {
        return;
    }
    int v0, v1, v2, v3, v4, v5, v6,v7;
    for (int ii=i; ii < min(N, i+8); ii++) {
        int remaining_width = min(M, j+8) - j;

        if (remaining_width >= 1) v0 = A[ii][j+0];
        if (remaining_width >= 2) v1 = A[ii][j+1];
        if (remaining_width >= 3) v2 = A[ii][j+2];
        if (remaining_width >= 4) v3 = A[ii][j+3];
        if (remaining_width >= 5) v4 = A[ii][j+4];
        if (remaining_width >= 6) v5 = A[ii][j+5];
        if (remaining_width >= 7) v6 = A[ii][j+6];
        if (remaining_width >= 8) v7 = A[ii][j+7];

        if (remaining_width >= 1) B[j + 0][ii] = v0;
        if (remaining_width >= 2) B[j + 1][ii] = v1;
        if (remaining_width >= 3) B[j + 2][ii] = v2;
        if (remaining_width >= 4) B[j + 3][ii] = v3;
        if (remaining_width >= 5) B[j + 4][ii] = v4;
        if (remaining_width >= 6) B[j + 5][ii] = v5;
        if (remaining_width >= 7) B[j + 6][ii] = v6;
        if (remaining_width >= 8) B[j + 7][ii] = v7;
    }

}

void subblock_routine(int i, int j, int M, int N, int A[N][M] ,int B[M][N]) {

    int v0, v1, v2, v3, v4, v5, v6, v7;
    int k;

    //Flip Q1 and store extra data in Q2 (as a buffer)
    for (k = 0; k < 4; k++ ){
        v0 = A[i + k][j + 0];
        v1 = A[i + k][j + 1];
        v2 = A[i + k][j + 2];
        v3 = A[i + k][j + 3];
        v4 = A[i + k][j + 4];
        v5 = A[i + k][j + 5];
        v6 = A[i + k][j + 6];
        v7 = A[i + k][j + 7];

        //Transpose top left
        B[j + 0][i + k] = v0;
        B[j + 1][i + k] = v1;
        B[j + 2][i + k] = v2;
        B[j + 3][i + k] = v3;

        //Store extra data
        B[j + 0][i + k + 4] = v4;
        B[j + 1][i + k + 4] = v5;
        B[j + 2][i + k + 4] = v6;
        B[j + 3][i + k + 4] = v7;
    }

    //Flip Q2 and Q3
    //Average of 2 misses per loop
    for (k = 0; k < 4; k++ ){
        //Load row0 of Q2 into registers (single set!)
        v0 = B[j + k][i + 4];
        v1 = B[j + k][i + 5];
        v2 = B[j + k][i + 6];
        v3 = B[j + k][i + 7];

        //Q3: Load the column of A (to replace data above in Q2)
        //4 sets
        v4 = A[i + 4][j + k];
        v5 = A[i + 5][j + k];
        v6 = A[i + 6][j + k];
        v7 = A[i + 7][j + k];

        //Write to Q2 (miss)
        B[j + k][i + 4] = v4;
        B[j + k][i + 5] = v5;
        B[j + k][i + 6] = v6;
        B[j + k][i + 7] = v7;

        //Write to Q3 (miss)
        B[j + k + 4][i + 0] = v0;
        B[j + k + 4][i + 1] = v1;
        B[j + k + 4][i + 2] = v2;
        B[j + k + 4][i + 3] = v3;
    }

    //Q4
    for (k = 4; k < 8; k++ ){
        v4 = A[i + k][j + 4];
        v5 = A[i + k][j + 5];
        v6 = A[i + k][j + 6];
        v7 = A[i + k][j + 7];

        //Transpose top left
        B[j + 4][i + k] = v4;
        B[j + 5][i + k] = v5;
        B[j + 6][i + k] = v6;
        B[j + 7][i + k] = v7;
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

    registerTransFunction(transpose_simple_buffer, transpose_simple_buffer_desc);
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

