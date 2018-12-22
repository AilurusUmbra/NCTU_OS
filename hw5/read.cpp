#include<cstdio>
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<list>
#include<map>
#include<set>

using namespace std;

int main(){

    // open file
    FILE * file;
    file = fopen("trace3.txt", "r");
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


    fscanf(file," %c ", &mode);
    cout<<"mode="<<mode;
    printf("mode=%c!\n", mode);
    // FIFO
	cout<<"FIFO---\n"<<setiosflags(ios::left)
        <<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";
       
    for(frame_num=64; frame_num<=64; frame_num*=2){
        searchset.clear();
        frame.clear();
        hit=0;
        miss=0;
        
        while(fscanf(file, "%c", mode)){
            fscanf(file, "%s", page_array);
            printf("mode: %c | ");
            cout<<page_array;
            page[5]='\0';
            temp=page.c_str();
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
        cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<(double)miss/(hit+miss)<<endl;
    }
    
	// LRU
    /* 
	cout<<"LRU---\n"<<setiosflags(ios::left)
	 	<<setw(15)<<"size"<<setw(15)<<"miss"<<setw(15)<<"hit"<<"page fault ratio\n";

    for(frame_num=64; frame_num<=512; frame_num*=2){
            fs.clear();
            fs.seekg(0, ios::beg);
            search.clear();
            frame.clear();
            hit=0;
            miss=0;
            while(fs>>mode>>page){
                page[5]='\0';
                temp=page.c_str();
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
            
            cout<<setiosflags(ios::left)<<setw(15)<<frame_num<<setiosflags(ios::left)<<setw(15)<<miss;
            cout<<setiosflags(ios::left)<<setw(15)<<hit<<setw(23)<<(double)miss/(hit+miss)<<endl;
        }*/

    fclose(file);
    return 0;
}
