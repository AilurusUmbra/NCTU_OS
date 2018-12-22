#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<iomanip>
#include<string>
#include<list>
#include<map>
#include<set>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

using namespace std;

char in_array[10100000][12];

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
    string temp;
    int hit=0, miss=0, iteration=0;
    int idx=0, in_max=0, frame_num=64;
    struct timeval start, end;
    int hit_ans[9], miss_ans[9];
    int frame_ans[5]={0, 64, 128, 256, 512};
    pid_t pid;
	
    // read file
	while(fscanf(file, " %c ", &mode)==1){
		fscanf(file, "%s", in_array[idx]);
		in_array[idx][5]='\0';
		idx++;
	}
	in_max=idx;

    // fork 2 proc to do FIFO & LRU
    if((pid = fork())<0) {
        fprintf(stderr, "simple fork failed\n");
        exit(-1);
    } else if(pid==0) { // FIFO 
        for(frame_num=64; frame_num<=512; frame_num*=2){/*{{{*/
            searchset.clear();
            frame.clear();
            hit=0;
            miss=0;
            idx=0;
            iteration++;
            while(idx<in_max){
                temp=in_array[idx++];
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

            hit_ans[iteration]=hit;
            miss_ans[iteration]=miss;
            }//while end
        } /*}}}*/
        
        cout<<"FIFO---\n"<<setiosflags(ios::left)
            <<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";
        for(int i=1; i<5; ++i){/*{{{*/
            cout<<setiosflags(ios::left)<<setw(15)<<frame_ans[i]<<setiosflags(ios::left)<<setw(15)<<miss_ans[i];
            cout<<setiosflags(ios::left)<<setw(15)<<hit_ans[i]
                <<setw(23)<<fixed<<setprecision(9)<<(double)miss_ans[i]/(hit_ans[i]+miss_ans[i])<<endl;
        }/*}}}*/
/*{{{*/
        cout<<"LRU ---\n"<<setiosflags(ios::left)
            <<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";
      //  for(frame_num=256; frame_num<=512; frame_num*=2){
        frame_num=64;
        search.clear();
        frame.clear();
        hit=0;
        miss=0;
        idx=0;
        iteration++;
        while(idx<in_max){
            temp=in_array[idx++];
            if(search.find(temp)!=search.end()){
                ++hit;
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
        }//while end
        
        hit_ans[iteration]=hit;
        miss_ans[iteration]=miss;
        //}
        int i = 5;
        cout<<setiosflags(ios::left)<<setw(15)<<frame_ans[i-4]<<setiosflags(ios::left)<<setw(15)<<miss_ans[i];
        cout<<setiosflags(ios::left)<<setw(15)<<hit_ans[i]
            <<setw(23)<<fixed<<setprecision(9)<<(double)miss_ans[i]/(hit_ans[i]+miss_ans[i])<<endl;
/*}}}*/        
        exit(1);
    
    } else { // LRU
       iteration=1; 
        for(frame_num=128; frame_num<=512; frame_num*=2){
            search.clear();
            frame.clear();
            hit=0;
            miss=0;
            idx=0;
			iteration++;
            while(idx<in_max){
				temp=in_array[idx++];
                if(search.find(temp)!=search.end()){
                    ++hit;
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
            }//while end
            
            hit_ans[iteration+4]=hit;
            miss_ans[iteration+4]=miss;
        }
        for(int i=6; i<9; ++i){
            cout<<setiosflags(ios::left)<<setw(15)<<frame_ans[i-4]<<setiosflags(ios::left)<<setw(15)<<miss_ans[i];
            cout<<setiosflags(ios::left)<<setw(15)<<hit_ans[i]
                <<setw(23)<<fixed<<setprecision(9)<<(double)miss_ans[i]/(hit_ans[i]+miss_ans[i])<<endl;
    }
} 
    
    wait(0);

    fclose(file);
    return 0;
}
