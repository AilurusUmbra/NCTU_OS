#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>

int dot(int *u, int *v, int size){
    int rst=0;
    for(short i=0; i < size; ++i)
      rst+=u[i]*v[i];
    return rst;
}


int main(void){

    int checksum=0, size=0;
    scanf("%d", &size);
    struct timeval start, end;


    int **A = (int **)malloc(size * sizeof(int*));{{{
    int **B = (int **)malloc(size * sizeof(int*));
    int **C = (int **)malloc(size * sizeof(int*));
    for(int i = 0; i < size; ++i){/*}}}*/
        A[i] = (int *)malloc(size * sizeof(int));
        B[i] = (int *)malloc(size * sizeof(int));
	C[i] = (int *)calloc(size , sizeof(int));
    }
    for(int i = 0; i < size; ++i){
        for(int j = 0; j < size; ++j){
	    A[i][j] = i*size + j ;
            B[i][j] = i + j*size ;	    
	 //   C[i][j] = A[i][j]+B[i][j];
	   // printf("%d\n", C[i][j]);
	}
    }
/*
    //print
    for(int i=0;i<size;++i){
	for(int j=0;j<size;++j){
	printf("%d ",A[i][j]);
	}
	printf("\n");
    }
    printf("\n\n");

    for(int i=0;i<size;++i){
	for(int j=0;j<size;++j){
	printf("%d ",B[i][j]);
	}
	printf("\n");
    }*/
    
    gettimeofday(&start, 0);

    printf("\n\n");
    for(int i=0;i<size;++i){
	for(int j=0;j<size;++j){
	C[i][j] = dot(A[i],B[j],size);
	}
    }

    gettimeofday(&end, 0);
     
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_sec - start.tv_usec;
    printf("elapsed time:%f ms\n", sec*1000+(usec/1000.0)); 


    for(int i=0;i<size;++i){
	for(int j=0;j<size;++j){
	    checksum += C[i][j];
	}
    }


    printf("checksum=%d\n", checksum);

    return 0;
}

