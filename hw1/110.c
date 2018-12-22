#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#define MAXLEN 1024

char command[MAXLEN];
char *argv1[MAXLEN];
char *argv2[MAXLEN];
char *instr;
char *token;
short idx;

//detect '\t', '\r', '\n'
void detecttab(char *array){
    int i;
    for(i=0;i<strlen(array);++i){
        if(array[i]=='\t' | array[i]=='\r')
            array[i]=' ';
        else if(array[i]=='\n')
            array[i]='\0';
        
    }
    array[strlen(array)]='\0';

}

//double fork used when & or | appears
pid_t double_fork(int flag){

    //declare child_pid and grandchild_pid
    pid_t ch_id, gr_id;

    //var for waitpid
    pid_t end_id, temp_status;


    //First fork
    if((ch_id = fork())==-1){
        fprintf(stderr,"First fork failed! -in double_fork()\n");
        exit(-1);
    }
    //child proc.
    if(ch_id==0){

	//declare file descripter for pipe
	int pipefd[2];    
        int save_stdin, save_stdout;
	pipe(pipefd);

        //Second fork
        if((gr_id = fork())==-1){
            fprintf(stderr,"Second fork failed! -in double_fork()\n");
            exit(-1);
        }
        //grandchild proc.
        else if(gr_id==0){

	    switch(flag){
	        //grandchild proc. writes pipefd[1] as output
		case 'p':
		    save_stdout = dup(1);	
                    dup2(pipefd[1], 1);
		    close(pipefd[0]);
		    execvp(instr, argv1);
		    close(pipefd[1]);
		    dup2(1, save_stdout);
		    break;
		//grandchild proc. executes argv1 anyway
		case 'e':
		    close(pipefd[0]);
	    	    close(pipefd[1]);	
		    execvp(instr, argv1);    
		    break;
	    }
	

        }
        //child proc.
        else{
	    switch(flag){
	        //child proc. reads pipefd[0] as input
		case 'p':
		    save_stdin = dup(0);	
                    dup2(pipefd[0], 0);
		    close(pipefd[1]);
		    execvp(argv2[0],argv2);
		    close(pipefd[0]);
	     	    dup2(0, save_stdin);
		    break;
		//child proc. suicides when & appears
		case 'e':
		    close(pipefd[0]);
	    	    close(pipefd[1]);	    
	            exit(0);
		    break;
	    }
        }
    }
    //parent proc.
    else{
    }
    return ch_id;
}

void cut_command_to_token(char **argvec, char *com){
    idx = 0;
    argvec[idx] = strtok(com, " ");
    instr = argvec[idx];
    while(argvec[idx] != NULL){
        argvec[++idx] = strtok(NULL, " ");
    }
}

int main(){

    int status;
    

    while(1){
	//prompt sign & read command
        printf(">");
        fgets(command, sizeof(command), stdin);

        //deal with '\t', '\r', '\n'
        detecttab(command);
        fflush(stdin);

	char flag = 0;
        //detect rediection or pipe
        char *redirect = strchr(command,'>');
        if(redirect!=NULL) {
            *redirect = '\0';
            redirect = redirect + 2;
            cut_command_to_token(argv2, redirect);
	    flag = 'r';
        }
        char *pipe = strchr(command,'|');
        if(pipe!=NULL) {
            *pipe = '\0';
            pipe = pipe + 2;
            cut_command_to_token(argv2, pipe);
       	    flag = 'p';
       	}

        //detect anpersand
        char *et = strchr(command,'&');
        if(et!=NULL) {
	    *et=' ';
	    flag='e';
	}

	//cut command to token
        cut_command_to_token(argv1, command);
        
	
	
	pid_t pid;
        pid_t df_pid;
        if(et!=0 | pipe!=0 ) {
            df_pid = double_fork(flag);
            if(df_pid==0){}
            else
		waitpid(df_pid, NULL, 0);    
            
        }
        else{
            //simple fork
            if((pid = fork())<0){
                fprintf(stderr, "simple fork failed\n");
                exit(-1);
            }
            //child proc
            else if(pid==0){
		 //simple exec
		 if(redirect==NULL) 
                     execvp(instr, argv1);
		 //redirect stdout to file
		 else{
	             int outfile = open(argv2[0], O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
		     //save stdout fd
		     int save_stdout = dup(1);
		     //replace stdout to output file
		     dup2(outfile, 1); 
		     execvp(instr, argv1);
	   	     close(outfile);
		     //replace back
		     dup2(1, save_stdout);
		 }
            }
            //parent proc
            else{            
                printf("pid=%d\n", waitpid(pid, &status, 0));
            }

        }//end simple fork
    }//end while(1)
    return 0;
}
