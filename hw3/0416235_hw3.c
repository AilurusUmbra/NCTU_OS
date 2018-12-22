#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#define TCOUNT 16

//declare data structure for thread {{{
//it descibe the range of sorting
struct range {
    int h;  //head
    int t;  //tail
    int id; //thread id 
};
/*}}}*/

//declare semaphores, sorted array, data structures{{{
int n;    
sem_t *sem_array;
int *unsorted, *unsorted2; //for multi-thread and single-thread
struct range *range_array;
/*}}}*/

//print range_array[i]{{{
void print_range(int idx){

    printf("range_array[%d]: \n", idx);
    printf("h=%d t=%d id=%d\n", range_array[idx].h, range_array[idx].t, range_array[idx].id );

}
/*}}}*/

int cmp (const void * a, const void * b){
  return ( *(int*)a - *(int*)b );
} 

int quick_divide(int *sort, int head, int tail){/*{{{*/

    if(head==tail) return head;
    int i = head-1;
    int j = head;
    int tmp = 0;

    for(j = head; j < tail; ++j) { 
        if(sort[j] <= sort[tail]) { 
            i++; 
            tmp = sort[i];
            sort[i] = sort[j];
            sort[j] = tmp; 
            
        } 
    } 

    tmp = sort[i+1];
    sort[i+1] = sort[tail];
    sort[tail] = tmp;
    return i+1;
}/*}}}*/

void bubble(int *sort, int head, int tail){/*{{{*/

    if(head>=tail) return;

    int tmp=0;
    for(int i=head; i<tail; ++i){
        for(int j=head; j<tail-1; ++j){
            if(sort[j]>sort[j+1]){
                tmp = sort[j];
                sort[j] = sort[j+1];
                sort[j+1] = tmp;
            }
        }
    }
}
/*}}}*/

void * pthread_func(void * data_struct){ /*{{{*/
    struct range * data = (struct range *)data_struct;
    sem_wait(&sem_array[data->id]);

    int tmp_pivot = 0; 
    if(data->id<8){
        //divide and assign range of next level division 
        tmp_pivot = quick_divide(unsorted, data->h, data->t);
        range_array[(data->id)*2].h = data->h;
        range_array[(data->id)*2].t = tmp_pivot==(data->h) ? (data->h) : (tmp_pivot-1);       
        range_array[(data->id)*2+1].h = tmp_pivot==(data->t) ? (data->t) : (tmp_pivot+1);
        range_array[(data->id)*2+1].t = data->t;
       
        //signal next level thread 
        sem_post(&sem_array[(data->id)*2]);
        sem_post(&sem_array[(data->id)*2+1]);
    }
    else{
        //bubble sort and signal thread1
        bubble(unsorted, data->h, data->t+1);
        //sem_post(&sem_array[1]);
    }

    pthread_exit((void*)0);
}
/*}}}*/


void * singlethread_func(void * data_struct){ 
    struct range * data = (struct range *)data_struct;
    int tmp_pivot = 0;
    int h = 0;
    int t = n-1;
    for(data->id=1; data->id<TCOUNT; ++(data->id)){ 
        h = range_array[(data->id)].h;
        t = range_array[(data->id)].t;
        if(data->id<8){
            //divide and assign range of next level division 
            tmp_pivot = quick_divide(unsorted2, h, t);
            range_array[(data->id)*2].h = h;
            range_array[(data->id)*2].t = tmp_pivot==h ? h : (tmp_pivot-1);       
            range_array[(data->id)*2+1].h = tmp_pivot==t ? t : (tmp_pivot+1);
            range_array[(data->id)*2+1].t = t;
            
        }
        else{
            //bubble sort 
            bubble(unsorted2, h, t+1);
        }
    }
    pthread_exit((void*)0);
}

int main(){

    //open input file{{{
    FILE *in;
    char input[1024]={0};
    printf("Enter input file name:");
    scanf("%s", input);
    
    in = fopen(input,"r");
    if(!in){
        printf("fail to open file...\n");
        exit(1);
    }/*}}}*/

    //read input file{{{
    //int n;
    fscanf(in, "%d", &n);
    unsorted = calloc(n, sizeof(int));
    unsorted2 = calloc(n, sizeof(int));
    for(int i=0; i<n; ++i){
        fscanf(in, "%d", &unsorted[i]);
        unsorted2[i] = unsorted[i];
        //printf("%d ",unsorted[i]);
    }  
    printf("\n");
    fclose(in);
/*}}}*/

    //malloc & initial semaphores
    sem_array = malloc(TCOUNT * sizeof(sem_t));
    for(int t=0; t<TCOUNT; ++t)
        sem_init(&sem_array[t], 0, 0);

    //malloc & initial ranges
    range_array = malloc(TCOUNT * sizeof(struct range));
    for(int i=0; i<TCOUNT; ++i){
        range_array[i].h = 0;
        range_array[i].t = n-1;
        range_array[i].id = i;
    }

    //malloc & initial thread array
    pthread_t *th = malloc(TCOUNT * sizeof(pthread_t));
    void *retval;


    //signal thread1
    sem_post(&sem_array[1]);

    //start timer
    struct timeval start, end;
    gettimeofday(&start, 0);
    
    //create threads to divide & sort
    for(int i=1;i<TCOUNT;++i){
        range_array[i].id = i;  
        pthread_create(th+i, NULL, pthread_func, (void*)&range_array[i]);
    }

    //join threads & destory semaphores
    for(int i=1; i<TCOUNT; ++i){
        pthread_join(th[i], &retval);
    }

    //stop timer and print runtime
    gettimeofday(&end, 0);
    printf("Multi-thread sorting elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
   
    //start timer
    gettimeofday(&start, 0);

    //use th[0] to do single thread sorting
    pthread_create(th, NULL, singlethread_func, (void*)&range_array[0]);
    pthread_join(th[0], &retval);
    
    //stop timer and print runtime
    gettimeofday(&end, 0);
    printf("Single-thread sorting elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
    
    //open write file{{{
    FILE *out1, *out2;
//    FILE *out3;
    out1 = fopen("output1.txt", "w");
    out2 = fopen("output2.txt", "w");
//    out3 = fopen("output3.txt", "w");
    if(!out1){
        printf("open output1.txt failed\n");
        exit(1);
    }
    if(!out2){
        printf("open output2.txt failed\n");
        exit(1);
    }
    //}}}
   

    //write output file 
    for(int i=0; i<n-1; ++i){
        fprintf(out1, "%d ", unsorted[i]);
    } 
    fprintf(out1, "%d", unsorted[n-1]);
   
 /*   qsort(unsorted, n, sizeof(int), cmp);
    for(int i=0; i<n-1; ++i){
        fprintf(out3, "%d ", unsorted[i]);
    }  
    fprintf(out3, "%d", unsorted[n-1]);*/
    
    for(int i=0; i<n-1; ++i){
        fprintf(out2, "%d ", unsorted2[i]);
    }  
    fprintf(out2, "%d", unsorted2[n-1]);
    
        
    free(sem_array);
    free(range_array);
    free(unsorted);
    free(unsorted2);

    fclose(out1);
    fclose(out2);
 //   fclose(out3);
    return 0;
}
