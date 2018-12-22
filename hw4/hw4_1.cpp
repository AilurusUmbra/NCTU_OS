#include <iostream>
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
               // 0 = idle, 1~3 = division, 4 = bubble_sort, 10 = detach itself
                
    int h;   //head
    int t;   //tail
};
/*}}}*/

//declare semaphores, sorted array, data structures{{{

int it;
int n;    
sem_t *sem_array;
sem_t full, empty;
sem_t job_mutex;
sem_t sort_finished;
int *unsorted;
int *unsorted2;
struct range *range_array;
int * thread_id;
deque<int> job_list; //save job_id
int job_id = 0;


/*}}}*/

//print range_array[i]{{{
void print_range(int idx){

    printf("range_array[%d]: \n", idx);
    printf("level=%d h=%d t=%d \n", range_array[idx].level , range_array[idx].h, range_array[idx].t);

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
    if(head<0 | tail>n) return;
    int tmp=0;
    for(int i=head; i<tail; ++i){
        if(i>=n)continue;
        for(int j=head; j<tail-1; ++j){
            if(j>=n-1) continue;
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
    int sem_id = *((int *)data_struct);
    
    int id; //job data save in range_array[id] 
    int level=0, h=0, t=n-1;
    int tmp_pivot = 0; 

    while(1){
        
        //this thread is waiting in pool until this sem is signalled
        sem_wait(&sem_array[sem_id]);
        while(1){
            //while(job_list.empty());
            consume(&id);
           // cout<<sem_id<<" ";
            //get job information
            level = range_array[id].level;
            h = range_array[id].h;
            t = range_array[id].t;
            
            if(level>0 && level<=3) { //division
                tmp_pivot = quick_divide(unsorted, h, t);

                produce(level, h, tmp_pivot == h ? h : (tmp_pivot-1) );
                produce(level, tmp_pivot == t ? t : (tmp_pivot+1), t);
                
            } else if(level==4) { //sort
                //bubble sort and signal thread1
                bubble(unsorted, h, t+1);

            } else if(level>=10) {
                pthread_detach(pthread_self());
             //   cout<<"\nkilljob sem_id="<<sem_id;
                pthread_exit((void*)0);
                break;
            } 

            // the thread taking the last job should signal main thread when finished and return to pool 
            if(id==14) {
             //   cout<<"\nup sem_id="<<sem_id;
                sem_post(&sort_finished);
                break;
            }
            // other threads return to pool if 15 jobs are produced and there exist no job in job_list
            if(job_id==15 && job_list.empty()){
            //   cout<<"\ndown sem_id"<<sem_id;
                //pthread_exit((void*)0);
                break;
            }

        }//inner while end
        
        if(level>=10) {
            pthread_exit((void*)0);
            break;
        }
    }//outer while end
}
/*}}}*/



int main(){

    //open input file{{{
    FILE *in;
    char input[20]="input.txt\0";
    //printf("Enter input file name:");
    //scanf("%s", input);
    
    in = fopen(input,"r");
    if(!in){
        printf("fail to open file...\n");
        exit(1);
    }/*}}}*/

    //read input file{{{
    
    //n is the number of elements waiting to sort
    fscanf(in, "%d", &n);
    unsorted = (int *)calloc(n, sizeof(int));
    //unsorted2 = (int *)calloc(n, sizeof(int));
    for(int i=0; i<n; ++i){
        fscanf(in, "%d", &unsorted[i]);
    //    unsorted2[i] = unsorted[i];
    }  
    printf("\n");
    fclose(in);
/*}}}*/
    
    //malloc & initial semaphores
    sem_array = (sem_t *)malloc(TCOUNT * sizeof(sem_t));
    for(int t=0; t<TCOUNT; ++t)
        sem_init(&sem_array[t], 0, 0);
    sem_init(&job_mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 15);
    sem_init(&sort_finished, 0, 0);

    //malloc & initial ranges
    range_array = (struct range *)malloc(JOBCOUNT * sizeof(struct range));
    for(int i=0; i<JOBCOUNT; ++i){
        range_array[i].level = 0; 
        range_array[i].h = 0;
        range_array[i].t = n-1;
    }

    //malloc & initial thread array
    pthread_t *th = (pthread_t *)malloc(TCOUNT * sizeof(pthread_t));
    void *retval;

    //malloc & initial thread_id
    thread_id = (int *)malloc(TCOUNT * sizeof(int));
    for(int i=1; i<TCOUNT; ++i){
        thread_id[i]=i;
    }

    //create threads to divide & sort
    for(int i=1; i<TCOUNT; ++i){
        pthread_create(th+i, NULL, pthread_func, (void*)&thread_id[i]);
    }

    



    struct timeval start, end;
    FILE *out, *out2;
    char filename[15]="output_3.txt";
    filename[12] = '\0';

    for(it=1; it<TCOUNT; ++it){

        //reinitialize 
        for(int i=0; i<JOBCOUNT; ++i){
            range_array[i].level = 0; 
            range_array[i].h = 0;
            range_array[i].t = n-1;
        }
        //for(int i=0; i<n; ++i){ unsorted[i] = unsorted2[i];}
        job_id = 0;

 //       printf("Working thread number:\n");
        //start timer
        gettimeofday(&start, 0);
   
        //enable threads of number it 
        for(int i=1; i<=it; ++i){
            sem_post(&sem_array[i]);
            
        }
         
        //push first job (divide unsorted array)
        produce(0, 0, n-1); 
        sem_wait(&sort_finished);

        
        //stop timer and print runtime
        gettimeofday(&end, 0);
        printf("\nSorting with pool of %d thread elapsed %f s\n", it, (end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);

         
        //open write file{{{
        filename[7]=it+'0';
        out = fopen(filename, "w");
       // out2 = fopen("output2.txt", "w");
        if(!out){
            printf("open output_n.txt failed\n");
            exit(1);
        }
        if(!out2){
            printf("open output2.txt failed\n");
            exit(1);
        }
        //}}}
   
        //write output file 
        for(int i=0; i<n-1; ++i){
            fprintf(out, "%d \n", unsorted[i]);
        } 
        fprintf(out, "%d\n", unsorted[n-1]);
       
     /*   qsort(unsorted, n, sizeof(int), cmp);
        for(int i=0; i<n-1; ++i){
            fprintf(out2, "%d \n", unsorted[i]);
        }  
        fprintf(out2, "%d\n", unsorted[n-1]);
        
        fclose(out2);*/
        fclose(out);
    }





    //push 8 detach job and signal threads to detach thread pool
        for(int i=0; i<JOBCOUNT; ++i){
            range_array[i].level = 10; 
            range_array[i].h = 0;
            range_array[i].t = n-1;
        }
    for(int i=1; i<TCOUNT; ++i){
        job_id = 1;
        produce(9, 0, n-1);
        sem_post(&sem_array[i]);
    }

    
    
     
    free(sem_array);
    free(range_array);
    free(thread_id);
    free(unsorted);

    return 0;
}
