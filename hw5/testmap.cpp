#include<cstdio>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<list>
#include<map>
#include<set>
#include <sys/time.h>

using namespace std;

int idx=0;
char in_array[10100000][12];
//string in_str[10100000];

int main(){

    // open file
    FILE * file;
    file = fopen("trace.txt", "r");
    if(!file){
        cout<<"trace.txt fail to open...\n";
        exit(1);    
    }

    // declaration
    set<string> searchset;
    map<string, list<string>::iterator> search;
    list<string> frame;
    char mode;
    char page_array[12];
    string page, temp;
    int hit=0, miss=0;
    int frame_num=64;

    struct timeval start, end;
    //start timer
    gettimeofday(&start, 0);
    // read file
    int in_max=0; 
    for(int i=1;i<2;++i ){
        idx=0;
        while(fscanf(file, " %c ", &mode)==1){
            fscanf(file, "%s", in_array[idx]);
            in_array[idx][5]='\0';
            //page_array[5]='\0';
            //temp=page_array;
 //           temp=in_array[idx];
            //in_str[idx]=in_array[idx];
            //cout<<"in_str[idx]="<<in_str[idx]<<endl;
            idx++;
        }
        in_max=idx;
        //fseek(file, 0, SEEK_SET);
    }
    //stop timer and print runtime
    gettimeofday(&end, 0);
    printf("elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
    

    // FIFO
	cout<<"FIFO---\n"<<setiosflags(ios::left)
        <<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";
       
    for(frame_num=64; frame_num<=512; frame_num*=2){
        fseek(file, 0, SEEK_SET);
        searchset.clear();
        frame.clear();
        hit=0;
        miss=0;
        idx=0;
        gettimeofday(&start, 0);
        while(idx<in_max){
            temp=in_array[idx++];
        /*while(fscanf(file, " %c ", &mode)==1){
            fscanf(file, "%s", page_array);
            //page=page_array;
            page_array[5]='\0';
            //temp=page.c_str();
            temp=page_array;*/
            if(searchset.find(temp)!=searchset.end()){
                ++hit;
            }
            else{
                ++miss; 
                if(searchset.size()==frame_num){
                    searchset.erase(frame.front());
                    frame.pop_front();
                }
                searchset.insert(temp);
                frame.push_back(temp);
            }
        }//while end
        gettimeofday(&end, 0);
        printf("else elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
        
        cout<<setiosflags(ios::left)<<setw(15)<<frame_num<<setiosflags(ios::left)<<setw(15)<<miss;
        cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<(double)miss/(hit+miss)<<endl;
    }
   
	// LRU
/*    
    struct timeval start2, end2;
	cout<<"LRU---\n"<<setiosflags(ios::left)
	 	<<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";

    for(frame_num=64; frame_num<=512; frame_num*=2){
        //fseek(file, 0, SEEK_SET);
        gettimeofday(&start2, 0);
        search.clear();
        frame.clear();
        hit=0;
        miss=0;
        idx=0;
        gettimeofday(&end2, 0);
        printf("else elapsed %f s\n",(end2.tv_sec-start2.tv_sec) + (end2.tv_usec-start2.tv_usec)/1000000.0);
//        while(fscanf(file, " %c ", &mode)==1){
            //fscanf(file, "%s", page_array);
            //page=page_array;
            //page_array[5]='\0';
            //temp=page.c_str();
            //temp=page_array;
        gettimeofday(&start, 0);
        while(idx<in_max){
            temp=in_array[idx++];
            if(search.find(temp)!=search.end()){
                ++hit;
                if(frame.back()==temp) continue;
                frame.erase(search[temp]);
                frame.push_back(temp);
                search[temp]=--frame.end();
            }
            else{
                ++miss; 
                if(search.size()==frame_num){
                    search.erase(frame.front());
                    frame.pop_front();
                }
                frame.push_back(temp);
                search[temp]=--frame.end();
            }
            //++idx;
        }//while end
        
        gettimeofday(&end, 0);
        printf("if elapsed %f s\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
        cout<<setiosflags(ios::left)<<setw(15)<<frame_num<<setiosflags(ios::left)<<setw(15)<<miss;
        cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<(double)miss/(hit+miss)<<endl;
    }
*/
       // gettimeofday(&start2, 0);
       // gettimeofday(&end2, 0);
       // printf("else elapsed %f s\n",(end2.tv_sec-start2.tv_sec) + (end2.tv_usec-start2.tv_usec)/1000000.0);
    fclose(file);
    return 0;
}
