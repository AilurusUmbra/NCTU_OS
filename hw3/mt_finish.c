#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#define TCOUNT 16

struct range {
    int h;  //head
    int t;  //tail
    int id; //thread id 
};

int n;    

sem_t *sem_array;
int *unsorted;
struct range *range_array;

void print_range(int idx){

    printf("range_array[%d]: \n", idx);
    printf("h=%d t=%d id=%d\n", range_array[idx].h, range_array[idx].t, range_array[idx].id );

}

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

void * pthread_func(void * data_struct){
    struct range * data = (struct range *)data_struct;
    sem_wait(&sem_array[data->id]);
//    printf("thread %d\n",data->id);

    int tmp_pivot = 0; 
    if(data->id<8){
//        print_range(data->id);
        tmp_pivot = quick_divide(unsorted, data->h, data->t);
//        printf("tmp_pivot=%d\n",tmp_pivot); 
//        for(int i=0; i<n; ++i){printf("%d ",unsorted[i]);}   
        printf("\n");
        range_array[(data->id)*2].h = data->h;
        range_array[(data->id)*2].t = tmp_pivot==(data->h) ? (data->h) : (tmp_pivot-1);       
        range_array[(data->id)*2+1].h = tmp_pivot==(data->t) ? (data->t) : (tmp_pivot+1);
        range_array[(data->id)*2+1].t = data->t;
//        print_range((data->id)*2);
//        print_range((data->id)*2+1);
        sem_post(&sem_array[(data->id)*2]);
        sem_post(&sem_array[(data->id)*2+1]);
    }
    else{
//        print_range(data->id);
        bubble(unsorted, data->h, data->t+1);
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
    for(int i=0; i<n; ++i){
        fscanf(in, "%d", &unsorted[i]);
        printf("%d ",unsorted[i]);
    }  
    printf("\n");
    fclose(in);
/*}}}*/

    //initial semaphores
    sem_array = malloc(TCOUNT * sizeof(sem_t));
    for(int t=0; t<TCOUNT; ++t)
        sem_init(&sem_array[t], 0, 0);

    //initial ranges
    int pivot = 0;
    range_array = malloc(TCOUNT * sizeof(struct range));
    for(int i=0; i<TCOUNT; ++i){
        range_array[i].h = 0;
        range_array[i].t = n-1;
        range_array[i].id = i;
    }

    //construct thread array
    pthread_t *th = malloc(TCOUNT * sizeof(pthread_t));
    void *retval;

    sem_post(&sem_array[1]);

    for(int i=1;i<TCOUNT;++i){
        range_array[i].id = i;  
//        printf("hi %d id= %d\n",i, range_array[i].id); 
        pthread_create(th+i, NULL, pthread_func, (void*)&range_array[i]);
    }
/*    for(int i=1; i<4; ++i){
        range_array[i].h = 0;
        range_array[i].t = n-1???????????????;
        pthread_create(th+i, NULL, thread_divide, (void *)&range_array[i]);
    }

    for(int i=4; i<TCOUNT; ++i){
        pthread_create(th+i, NULL, thread_bubble, (void *)&range_array[i]);
    }
*/
    for(int i=1; i<TCOUNT; ++i){
        pthread_join(th[i], &retval);
 //       printf("return value from thread%d = %d\n", i, retval);
    }

    //quick division
    //pivot = quick_divide(unsorted, 0, n-1);

    //printf("pivot = %d\n", pivot);

    //for(int i=0; i<n; ++i){ printf("%d ",unsorted[i]);} printf("\n");
    
    //bubble sort
//    bubble(unsorted, 0, n); 

    //open write file{{{
    FILE *out1, *out2;
    out1 = fopen("output1.txt", "w");
    out2 = fopen("output2.txt", "w");

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
   
    qsort(unsorted, n, sizeof(int), cmp);
    for(int i=0; i<n-1; ++i){
        fprintf(out2, "%d ", unsorted[i]);
    }  
    fprintf(out2, "%d", unsorted[n-1]);
    
    
    
    free(unsorted);

    fclose(out1);
    fclose(out2);
    return 0;
}
