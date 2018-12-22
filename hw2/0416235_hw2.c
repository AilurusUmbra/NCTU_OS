#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>

unsigned int dot(int *u, int *v, int length){/*{{{*/
    unsigned int rst=0;
    for(short i=0; i < length; ++i)
      rst+=u[i]*v[i];
    return rst;
}/*}}}*/

int main(void){

    unsigned int checksum=0;
    int dim=0, size=0;
    scanf("%d", &dim);
    size = dim * dim;
    struct timeval start, end;

    int shm_id;
    shm_id = shmget(IPC_PRIVATE, 4 * sizeof(int) * size, IPC_CREAT | 0660);
//    printf("%d",shm_id);
    
    if(shm_id<0){
        printf("shm_id error, %d\n",shm_id);
        return 1; 
    }

    unsigned int *A = (unsigned int *)shmat(shm_id, NULL, 0);
    unsigned int *B = A + size;
    //unsigned int *B = (unsigned int *)shmat(shm_id, NULL, 0);
    unsigned int *C = B + size ;
    
    
    if(A == (void*) -1 )
    {  
      perror("shmat");
      exit(1);
    }

   //init A, B 
   for(int i = 0; i < dim; ++i){
        for(int j = 0; j < dim; ++j){
    	    A[i*dim+j] = i*dim + j ;
            B[i*dim+j] = i + j*dim ;
    	    
        }
    }

//   for(int i=0;i<size;++i) printf("%d ",A[i]);
//   for(int i=0;i<size;++i) printf("%d ",B[i]);
   
   
    int quotient = 0;
    int atama = 0, shippo = size; 
    int proc_idx = 0;
    
    
pid_t allpid[17];
allpid[0] = 1;
int shm_cid = shmget(IPC_PRIVATE, 3 * sizeof(int) * size, IPC_CREAT | 0660);


for(int n=0; n<16; ++n){

    
    printf("Multiply matrix by using %d process:\n", n+1);
    
    //init proc_idx & pid array
    proc_idx = 0;
    allpid[0]=1;
    for(int f=1; f<17; ++f) 
        allpid[f] = 0;

    //fork n process
    for(int f=1; f<=n; ++f){ 
        //only parent process call fork()
        if(allpid[f-1]!=0){
            allpid[f]=fork();
            allpid[0]=0;
        }
    }

    C = (unsigned int*)shmat(shm_cid, NULL, 0);

    for(int f=0; f<=n; ++f) {
        if(allpid[f]==0) proc_idx = f;

    }

    checksum = 0;
    
    quotient = dim/(n+1);

    atama = proc_idx * quotient;
    shippo = (proc_idx==(n)) ? dim : ((proc_idx+1)*quotient);

    gettimeofday(&start, 0);
    for(int i=atama; i<shippo; ++i){
        for(int j=0; j<dim; ++j){
            C[i*dim+j] = dot(A+i*dim, B+j*dim, dim);
            //printf("c[%u]=%u ",i*dim+j,C[i*dim+j]);
        }
      //  printf("\n");
    }


    // multiplication finished, wait process
    if(proc_idx != 0) {
        exit(0);
        shmdt(C);
    }
    for(int f=1; f<=n; ++f){ 
        waitpid(allpid[f], NULL, 0);
    } 

    gettimeofday(&end, 0);


    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    printf("elapsed time:%f s\n", sec*100 + usec/10000.0); 


    for(int i=0;i<size;++i)
        checksum += C[i];
    printf("checksum=%u\n", checksum);

}// for loop end

    shmdt(C);
    shmdt(B);
    shmdt(A);

    shmctl(shm_id,IPC_RMID,0);
    
    return 0;
}

