#define _XOPEN_SOURCE 600 //to remove error: pthread_barrier_t type is undefined
#include <pthread.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct{
    int A_start_row;
    int A_num_blocks;
    int C_start_row;
    int C_num_blocks;
    int thread_id;
} t_arguments;


int M, N, P, NUM_THREADS;
float **A, **B, **C, **R, **D;
pthread_barrier_t barrier;


void fillMatrix(int n_rows, int n_cols, float **matrix){

    for(int i =0; i<n_rows; ++i){
        matrix[i] = (float *)malloc(n_cols * sizeof(float));
        for(int j=0; j<n_cols; ++j){
            matrix[i][j] = rand()%10;
        }
    }
}


void printMatrix(int n_rows, int n_cols,float **matrix){

    for(int i =0; i<n_rows; ++i){
        for(int j=0; j<n_cols; ++j){
            printf("%.2f ",matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}


void freeMatrix(int n_rows, float **matrix){

    for(int i =0; i<n_rows; ++i){
        free(matrix[i]);
    }

    free(matrix);
}


void addElement(int row, int col, float element, float **matrix){
    matrix[row][col]=element;
}


void vector_mul(
    float **m1,
    int m2_cols, 
    float **m2, 
    int row, 
    int col,
    float **m3
){
    float sum=0;

    for(int i=0; i<m2_cols; ++i){
        sum += m1[row][i] * m2[i][col];
    }

    addElement(row,col,sum,m3);
}


void decomposition(t_arguments *args){
    //calculate A*B
    for(int j=0; j<args->A_num_blocks; ++j){
        for(int i=0; i<P; ++i){
            vector_mul(A, N, B, args->A_start_row+j, i, R);
        }
    }
    
    //Wait threads with barrier
    pthread_barrier_wait(&barrier);

    //Kill threads useless
    if(args->thread_id>=P){
        pthread_exit(0);
    }
    
    //calculate C*R
    for(int j=0; j<args->C_num_blocks; ++j){
        for(int i=0; i<P;++i){
            vector_mul(C, M, R,args->C_start_row+j, i, D);
        }
    }
}


t_arguments* initialize_thread_args(
    int A_start_row,
    int A_num_blocks,
    int C_start_row,
    int C_num_blocks,
    int thread_id
){
    t_arguments *args = (t_arguments *)malloc(sizeof(t_arguments));
    args->A_start_row=A_start_row;
    args->A_num_blocks=A_num_blocks;
    args->C_start_row=C_start_row;
    args->C_num_blocks=C_num_blocks;
    args->thread_id = thread_id;

    return args;
}


int main(int argc, char ** argv){
    printf("A is a matrix MxN\nB is a matrix NxP\nC is a matrix PxM\n");
    printf("Insert M = ");
    scanf("%i",&M);

    printf("Insert N = ");
    scanf("%i",&N);

    printf("Insert P = ");
    scanf("%i",&P);

    printf("Insert NUM_THREADS = ");
    scanf("%i",&NUM_THREADS);

    if(M!=0 & N!=0 & P!=0 & NUM_THREADS!=0){

        struct timespec start, end;
        float time_res;
        clock_gettime(CLOCK_MONOTONIC, &start);

        A = (float **)malloc(M * sizeof(float *));
        B = (float **)malloc(N * sizeof(float *));
        C = (float **)malloc(P * sizeof(float *));
        R = (float **)malloc(M * sizeof(float *));
        D = (float **)malloc(P * sizeof(float *));
        int count = 0, blocksPlus = 0;
        srand(time(0));

        //initialize matrixes
        fillMatrix(M, N, A);
        fillMatrix(N, P, B);
        fillMatrix(P, M, C);
        fillMatrix(M,P,R);
        fillMatrix(P,P,D);


        //print matrix A B C
        printf("\nA:\n");
        printMatrix(M, N, A);

        printf("\nB:\n");
        printMatrix(N, P, B);

        printf("\nC:\n");
        printMatrix(P, M, C);

        //check on thread num and initialize threads' arguments

        if(NUM_THREADS>M){
            NUM_THREADS = M;
        }

        printf("NUM_THREADS=%i\n",NUM_THREADS);

        t_arguments* args[NUM_THREADS];
        pthread_t threads[NUM_THREADS];
        int errorCode[NUM_THREADS];

        //initialize barrier
        pthread_barrier_init(&barrier, NULL, NUM_THREADS);

        //initialize thread args
        for(int i=0; i<NUM_THREADS; ++i){
            args[i] = initialize_thread_args(0,1,0,1,i);
        }

        //assign blocks and starting rows for matrix A
        if(NUM_THREADS<M){
            blocksPlus = M-NUM_THREADS;

            count = NUM_THREADS-1;

            for(int i=0; i<blocksPlus; ++i){
                args[count]->A_num_blocks += 1;
                count--;

                if(count==-1){
                    count=NUM_THREADS-1;
                }
            }

            for(int i=0;i<NUM_THREADS; ++i){
                if(i>0){
                    args[i]->A_start_row += args[i-1]->A_num_blocks + args[i-1]->A_start_row;
                }
            }
        }
        else{
            for(int i=0;i<NUM_THREADS; ++i){
                args[i]->A_start_row = i;
            }
        }

        //assign blocks and starting rows for matrix C
        blocksPlus=0;

        if(NUM_THREADS<P){
            blocksPlus = P-NUM_THREADS;

            count = NUM_THREADS-1;

            for(int i=0; i<blocksPlus; ++i){
                args[count]->C_num_blocks += 1;
                count--;

                if(count==-1){
                    count=NUM_THREADS-1;
                }
            }

            for(int i=0;i<NUM_THREADS; ++i){
                if(i>0){
                    args[i]->C_start_row += args[i-1]->C_num_blocks + args[i-1]->C_start_row;
                }
            }
        }
        else{
            for(int i=0;i<NUM_THREADS; ++i){
                args[i]->C_start_row = i;
            }
        }

        
        //print blocks num and starting rows
        for(int i=0; i<NUM_THREADS; ++i){
            printf("thread %i blocks num A=%i and starting row A=%i \n",args[i]->thread_id, args[i]->A_num_blocks, args[i]->A_start_row);
            printf("thread %i blocks num C=%i and starting row C=%i \n",args[i]->thread_id, args[i]->C_num_blocks, args[i]->C_start_row);
        }

        

        // Start the threads
        for(int i=0; i<NUM_THREADS; ++i){
            errorCode[i] = pthread_create(&threads[i], NULL, (void*)decomposition, args[i]);

            
            if(errorCode[i] != 0)
                perror("Thread not created\n");
        }

        // Wait for the threads to end
        for(int i=0; i<NUM_THREADS; ++i){

            if(errorCode[i] == 0){
                errorCode[i] = pthread_join(threads[i], NULL);

                if(errorCode[i] != 0)
                    perror("Error joining thread\n");
            }
        }

        //print R and D matrixes
        printf("\nR:\n");
        printMatrix(M,P,R);

        printf("\nD:\n");
        printMatrix(P,P,D);

        //calculate execution time
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_res = ((end.tv_sec - start.tv_sec)*1e9 + (end.tv_nsec - start.tv_nsec))*1e-9;

        printf("Execution time=%f\n",time_res);

        //free memory

        for(int i=0; i<NUM_THREADS; ++i){
            free(args[i]);
        }

        pthread_barrier_destroy(&barrier);
        freeMatrix(M,A);
        freeMatrix(N,B);
        freeMatrix(P,C);
        freeMatrix(M,R);
        freeMatrix(P,D);
    }

    return 0;
}