#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<list>
#include<map>
#include<set>
#include<sys/time.h>

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
    int hit=0, miss=0;
    int idx=0, in_max=0, frame_num=64;
    struct timeval start, end;

	// read file
	while(fscanf(file, " %c ", &mode)==1){
		fscanf(file, "%s", in_array[idx]);
		in_array[idx][5]='\0';
		idx++;
	}
	in_max=idx;

    //start timer
    gettimeofday(&start, 0);	
	
    // FIFO
	cout<<"FIFO---\n"<<setiosflags(ios::left)
        <<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";
       
    for(frame_num=64; frame_num<=512; frame_num*=2){
        searchset.clear();
        frame.clear();
        hit=0;
        miss=0;
		idx=0;
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

        }//while end
        
        cout<<setiosflags(ios::left)<<setw(15)<<frame_num<<setiosflags(ios::left)<<setw(15)<<miss;
        cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<fixed<<setprecision(9)<<(double)miss/(hit+miss)<<endl;
    }
    
    // stop timer and print runtime
    gettimeofday(&end, 0);
    printf("\n FIFO elapsed %f s\n\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);

	// LRU 
    //start timer
    gettimeofday(&start, 0);	
	cout<<"LRU---\n"<<setiosflags(ios::left)
	 	<<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";

    for(frame_num=64; frame_num<=512; frame_num*=2){
            search.clear();
            frame.clear();
            hit=0;
            miss=0;
            idx=0;
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
                    cout<<"\nmiss before insert:\n";
                    for(list<string>::iterator it1 = frame.begin(); it1 != frame.end(); ++it1){
                        cout<<*it1<<" ";
                        getchar();
                    }
                    if(search.size()==frame_num){
                        search.erase(frame.front());
                        frame.pop_front();
                    }
                    frame.push_back(temp);
                    search[temp]=--frame.end();
                }
            }//while end
            
            cout<<setiosflags(ios::left)<<setw(15)<<frame_num<<setiosflags(ios::left)<<setw(15)<<miss;
            cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<(double)miss/(hit+miss)<<endl;
        }

    // stop timer and print runtime
    gettimeofday(&end, 0);
    printf("\n LRU elapsed %f s\n\n",(end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);
    
    fclose(file);
    return 0;
}
