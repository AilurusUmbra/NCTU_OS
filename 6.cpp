#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <map>
using namespace std;

#define TWOKILO 2047
#define MEGA 1048576
#define MAXIMUM 2147483647
string op[4] = {"-inode", "-name", "-size_min", "-size_max"};

void check_property(string& path, map<string, string>& table ){

    struct stat buf;
    stat(path.c_str(), &buf);

    if(buf.st_ino==0) {
        cout<<"not found..\n";
        return;
    } else {
    
        // -inode 
        if(table.find(op[0])!=table.end()) {
            if(buf.st_ino!=stoi(table[op[0]], nullptr, 0))
                return;
        }
        // -name
        if(table.find(op[1])!=table.end()) {

            if(table[op[1]]==".." | table[op[1]]==".")
                return;
            int cmp_leng = table[op[1]].length();
            if(path.length()<cmp_leng) 
                return;
            if(path.compare(path.size()-cmp_leng, cmp_leng, table[op[1]])!=0)
                return;
        }
        // -size_min
        if(table.find(op[2])!=table.end()) {
            
            int size_min;
            // overflow handling
            try{
                size_min = stoi(table[op[2]], nullptr, 0);
            }
            catch(...){
                size_min = MAXIMUM;
            }
            
            // >2047MB, ignore error of float 
            if (size_min>TWOKILO & (double)(buf.st_size/MEGA)<size_min)
                return;
            
            else if (size_min<=TWOKILO & buf.st_size<size_min*MEGA)
                return;
            
        }

        // -size_max
        if(table.find(op[3])!=table.end()) {
            
            int size_max;
            // overflow handling
            try {
                size_max = stoi(table[op[3]], nullptr, 0);
            } catch(...) {
                size_max = MAXIMUM;
            }

            // >2047MB, ignore error of float 
            if(size_max>TWOKILO) {
                if((double)(buf.st_size/MEGA)>size_max)
                    return;
            }
            else {
                if(buf.st_size>size_max*MEGA)
                    return;
            }
         }
        // ignore . and ..
        string subpath(path.end()-3, path.end());
        if (subpath.back()!='/') 
            subpath.push_back('/');
        subpath[0]=' ';
        if(subpath.find("./")==string::npos & subpath.find("../")==string::npos )
            cout<<path<<" "<<buf.st_ino<<" "<<(double)buf.st_size/MEGA<<"MB"<<endl;

    }
}

void dir_recursive(string& path, map<string, string>& table) { 
    
    // try to open directory
    DIR * dp = opendir(path.c_str());
    
    // check file properties
    if (!dp) {
        check_property(path, table);
        return;
    }
    
    struct dirent *filename;    
    char slash='/'; 
     
    while((filename=readdir(dp))) {
 
        // exclude . and ..
        if( string(filename->d_name)==".." | string(filename->d_name)=="." ) {
            continue;
        }
        
        // recursively find
        string newpath = path;
        if(newpath[newpath.length()-1]!=slash) {
            newpath = newpath + slash + "\0";
        }
        newpath += string(filename->d_name); 
        dir_recursive(newpath, table);
        
    }
   
    // check dir properties 
    closedir(dp);
    return;    
}


int main(int argc, char* argv[]) {

    // wrong arguments handling
    if(argc<2 | argc>10 | argc%2==1) {
        cout<<"wrong arguments...\n";
        return 1;
    }

    // declaration
    string path;
    string options[5], para[5];
    map<string, string> option_table; 
    path = argv[1];

    // arguments assignment
    for(int i=1; i<argc/2; ++i) {
        options[i]=argv[i*2];
        para[i]=argv[i*2+1];
        option_table[options[i]]=para[i];
    }

    dir_recursive(path, option_table); 

    return 0;
}
