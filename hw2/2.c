#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>

int dot(int *u, int *v, int length){
    int rst=0;
    for(short i=0; i < length; ++i)
      rst+=u[i]*v[i];
    return rst;
}

pid_t allpid[17];



int main(void){

    int checksum=0, dim=0, size=0;
    scanf("%d", &dim);
    size = dim * dim;
    struct timeval start, end;
printf("hey");
    int shm_id;
    shm_id = shmget(IPC_PRIVATE, 3 * sizeof(int) * size, IPC_CREAT | 0660);
    if(shm_id<0) {
	fprintf(stderr, "shm failed\n");
        exit(1);
    }

    int *A = (int *)shmat(shm_id, NULL, 0);
    int *B = A + size ;
    int *C = B + size ;

    //int **A = (int **)malloc(size * sizeof(int*));
    //int **B = (int **)malloc(size * sizeof(int*));
   // int **C = (int **)malloc(size * sizeof(int*));
  /*  for(int i = 0; i < dim; ++i){
        A[i] = (int *)malloc(dim * sizeof(int));
        B[i] = (int *)malloc(dim * sizeof(int));
	C[i] = (int *)calloc(dim , sizeof(int));
    }*/

    // set matrix A & B's value
    // use A[i*dim+j] stand for A[i][j]
    for(int i = 0; i < dim; ++i){
        for(int j = 0; j < dim; ++j){
	    A[i*dim + j] = i*dim + j ;
            B[i*dim + j] = i + j*dim ;	    
	}
    }
printf("ABis set");
    
    int quotient = 0;
    int atama = 0, shippo = size; 
    pid_t pid[5]={0, 0, 0, 0, 0};
    int proc_idx = 0;
    
    
/*    pid[1]=fork();
    pid[2]=fork();
    pid[3]=fork();
    pid[4]=fork();

    
    proc_idx = (pid[1]!=0) * 8 + (pid[2]!=0) * 4 + (pid[3]!=0) * 2 + (pid[4]!=0);
    
    switch(proc_idx){

    case 15:
	allpid[2] = pid[1];
	allpid[3] = pid[2];
	allpid[5] = pid[3];
	allpid[9] = pid[4];
	break;
	
	case 7:
	allpid[4] = pid[2];
	allpid[6] = pid[3];
	allpid[10] = pid[4];
	break;

	case 11:
	allpid[7] = pid[3];
	allpid[11] = pid[4];
	break;

	case 3:
	allpid[8] = pid[3];
	allpid[12] = pid[4];
	break;
	
	case 13:
	allpid[13] = pid[4];
	break;
	
	case 5:
	allpid[14] = pid[4];
	break;
	
	case 9:
	allpid[15] = pid[4];
	break;
	
	case 1:
	allpid[16] = pid[4];
	break;

	default: 
	break;
	}
  */  
     // if(proc_idx==15)
     
    //printf("proc_idx=%d %d %d %d %d\n ",proc_idx, pid[1],pid[2],pid[3],pid[4]);

    
    
proc_idx=0;
    // n stands for number of processes used
    for(int n=0; n<1; ++n){
        checksum = 0;
	if(n>0)
	quotient = dim/n;

	if(1){        
	//if(proc_idx<=n){

            atama = proc_idx * quotient;
	    shippo = (proc_idx==n) ? size : ((proc_idx+1)*quotient - 1);

            gettimeofday(&start, 0);
                for(int i=atama;i<shippo;++i){
	             C[i] += A[i]*B[i];
             
                }
            gettimeofday(&end, 0);
	}
        else 
	    continue;	
//wait 
     
       int sec = end.tv_sec - start.tv_sec;
       int usec = end.tv_usec - start.tv_usec;
//	printf("%f = %f - %f\n", sec, end.tv_sec, start.tv_sec);
//	printf("%f = %f - %f\n", usec, end.tv_usec, start.tv_usec);
        printf("elapsed time:%f s\n", sec*100 + usec/10000.0); 


        for(int i=0;i<size;++i)
	        checksum += C[i];
        printf("checksum=%d\n", checksum);
    }


/*    if(proc_idx==15) {
	waitpid(pid[1], NULL, 0);
    	waitpid(pid[2], NULL, 0);
	waitpid(pid[3], NULL, 0);
	waitpid(pid[4], NULL, 0);
    }
    else exit(0);
*/    
    shmdt(A);
    shmctl(shm_id,IPC_RMID,0);
    
/*    for(int a=0;a<17;++a) printf("allpid[%d]=%d\n",a,allpid[a]);
    wait(0);
    wait(0);
    wait(0);
    
    wait(0);
    wait(0);
    wait(0);
    wait(0);
    wait(0);
    wait(0);
    wait(0);
    waitpid(-1, NULL, 0);
*/
    return 0;
}

