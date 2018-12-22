#include <deque>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
using namespace std;
#define TCOUNT 9
#define JOBCOUNT 16

//declare data structure for thread {{{
//it descibe the range of sorting
struct range {
    int level; //operation level
               // 0 = idle, 1~3 = division, 4 = bubble_sort, 5 = detach itself
               // 
    int h;   //head
    int t;   //tail
    int id;  //thread id 
};
/*}}}*/

//declare semaphores, sorted array, data structures{{{

int it;
int n;    
sem_t *sem_array;
sem_t job_flag;
sem_t full, empty;
sem_t job_mutex;
int *unsorted, *unsorted2; //for multi-thread and single-thread
struct range *range_array;
deque<int> job_list; //save job_id
int job_id = 0;


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

//push job to list
void produce(int level, int h, int t){

    // produce item
    sem_wait(&empty);
    sem_wait(&job_mutex);
    // add the item to the buffer
    job_list.push_back(job_id);
    range_array[job_id].level = level+1;
    range_array[job_id].h = h;
    range_array[job_id].t = t;
    ++job_id;

    sem_post(&job_mutex);
    sem_post(&full);
    return;
}

//pop job from list
void consume(int * id){

    sem_wait(&full);
    sem_wait(&job_mutex);
    // remove an item from buffer
    *id = job_list.front();
    job_list.pop_front();
    
    sem_post(&job_mutex);
    sem_post(&empty);
    // consume the item   
    return;
}


void * pthread_func(void * data_struct){ /*{{{*/
    int * sem_id = (int *)data_struct;
    
    int id; //job data save in range_array[id] 
    int level=0, h=0, t=0;
    int tmp_pivot = 0; 

    while(1){
    
        //this thread is waiting in pool until this sem is signalled
        sem_wait(&sem_array[(*sem_id)]);
    
        while(1){

            consume(&id);
            //get job information
            level = range_array[id].level;
            h = range_array[id].h;
            t = range_array[id].t;
            
            if(level>0 && level<=3) { //division
                tmp_pivot = quick_divide(unsorted, h, t);
           
                produce(level, h, tmp_pivot == h ? h : (tmp_pivot-1) );
                produce(level, tmp_pivot == t ? t : (tmp_pivot+1), t);
                
            } else if(range_array[id].level==4) { //sort
            
                //bubble sort and signal thread1
                bubble(unsorted, range_array[id].h, range_array[id].t+1);
                //sem_post(&sem_array[1]);

            } else if(range_array[id].level==5) { //detach
                pthread_detach(pthread_self());
                break;
            }


        }//inner while end
    }//outer while end
}
/*}}}*/

void * singlethread_func(void * data_struct){ {{{
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
}/*}}}*/


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
    
    //n is the number of elements waiting to sort
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
    sem_init(&job_mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 15);

    //malloc & initial ranges
    range_array = malloc(JOBCOUNT * sizeof(struct range));
    for(int i=0; i<JOBCOUNT; ++i){
        range_array[i].level = 0; 
        range_array[i].h = 0;
        range_array[i].t = n-1;
        range_array[i].id = i;
    }

    //malloc & initial thread array
    pthread_t *th = malloc(TCOUNT * sizeof(pthread_t));
    void *retval;


    //signal thread1
    //sem_post(&sem_array[1]);

    //start timer
    struct timeval start, end;
//    gettimeofday(&start, 0);
    
    //create threads to divide & sort
    for(int i=1; i<TCOUNT; ++i){
        pthread_create(th+i, NULL, pthread_func, (void*)i);
    }





    for(it=1; it<TCOUNT; ++it){

        job_id = 0;
        //start timer
    
        //enable #it 個 threads 
        for(int i=1; i<=it; ++i){
            sem_post(&sem_array[i]);
        }
        //push first job (divide unsorted)
        
        
        //disable threads (return to pool)
        for(int i=1; i<=it; ++i){
            sem_wait(&sem_array[i]);
        }
        //stop timer 
    }





    //join threads & destory semaphores
    for(int i=1; i<TCOUNT; ++i){
        pthread_join(th[i], &retval);
    }

    //stop timer and print runtime
//    gettimeofday(&end, 0);
//    printf("Multi-thread sorting elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
   
    //start timer
//    gettimeofday(&start, 0);

    //use th[0] to do single thread sorting
    pthread_create(th, NULL, singlethread_func, (void*)&range_array[0]);
    pthread_join(th[0], &retval);
    
    //stop timer and print runtime
//    gettimeofday(&end, 0);
//    printf("Single-thread sorting elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
    
    //open write file{{{
    FILE *out3, *out2;
    out3 = fopen("output3.txt", "w");
    out2 = fopen("output2.txt", "w");
    if(!out1){
        printf("open output3.txt failed\n");
        exit(1);
    }
    if(!out2){
        printf("open output2.txt failed\n");
        exit(1);
    }
    //}}}
   

    //write output file 
    for(int i=0; i<n-1; ++i){
        fprintf(out3, "%d ", unsorted[i]);
    } 
    fprintf(out3, "%d", unsorted[n-1]);
   
    qsort(unsorted, n, sizeof(int), cmp);
    for(int i=0; i<n-1; ++i){
        fprintf(out2, "%d ", unsorted[i]);
    }  
    fprintf(out2, "%d", unsorted[n-1]);
    
        
    free(sem_array);
    free(range_array);
    free(unsorted);

    fclose(out2);
    fclose(out3);
    return 0;
}
