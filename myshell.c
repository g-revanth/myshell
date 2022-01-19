#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>
#include<signal.h>
#include<time.h>
#include <errno.h>

pid_t jobspid[100];
int listposition=0;
char jobslist[100][1000] ;

int printprompt(){
    char currentworkingdirectory[1001]; 
    char *dir ;
    char *token;
    char dir1[1001];

    dir = getcwd(currentworkingdirectory,sizeof(currentworkingdirectory));
    strcpy(dir1,dir);
    token = strtok(dir,"/");
    while(token != NULL){
    strcpy(dir1,token);
    token = strtok(NULL,"/");
     }
    printf("[myshell %s]$", dir1);
    fflush(stdout);
    return 0;
}

char* readinp(char *inputstr) {
  
    char *tempinput = (char*)calloc(1001,sizeof(char)); //buffer
    inputstr = (char*)calloc(1001,sizeof(char));

    if(fgets(tempinput, 1001, stdin) == NULL ) {}

    strcpy(inputstr,tempinput);
    return inputstr;
}

int builtincommandcheck(char* builtinstr[1001],int tokensize) {
    
    pid_t pid;
    char space[1] = " ";
    char s[100];
    char *directory;
    directory = getcwd(s,sizeof(s));
     pid_t processid;
    int suspendstatus;

        if(strcmp(builtinstr[0],"exit") == 0){
            if(listposition>0){
             fprintf(stderr,"Error: there are suspended jobs\n");
            }

            else{
            exit(0);
            }

            return 1;
            }

        if(strcmp(builtinstr[0],"cd") == 0){
            if(tokensize == 1 || tokensize>2){
               fprintf(stderr,"Error: invalid command\n");
               return 1;
            }

            chdir(builtinstr[1]);
            return 1;

        }

        if(strcmp(builtinstr[0],"jobs")== 0){
            if(tokensize >1){
               fprintf(stderr,"Error: invalid command\n");
               return 1;

            }
            if(listposition>0){
            for(int c =0;c<listposition;c++){
                printf("%d %s\n",c+1,jobslist[c]);
            }
            }
            return 1;
        }

        if(strcmp(builtinstr[0],"fg")== 0){

          
            if(tokensize == 1 || tokensize>3){
               fprintf(stderr,"Error: invalid command\n");
                return 1;
            }

            int index = (builtinstr[1][0] - '0');

           if(index>listposition){
                fprintf(stderr,"Error: invalid job\n");
                    return 1;
            }

            else { 
                pid_t cont = jobspid[index-1];  

                kill(cont,SIGCONT);
                   
                if(index>0){
                for(int t=index;t<listposition;t++){
                    strcpy(jobspid[t-1],jobspid[t]);
                    strcpy(jobslist[t-1],jobslist[t]);
                }

                listposition = listposition-1;
                }
                else {
                        listposition=0;
                }
                return 1;
            }
        }

    return 0;
}

//execvp - array and p->filename - /bin & /user/bin 
//execv - array and directory -  ./ 
//execv - arrray and directory - append end of string

int executesimplecommands(char *inputtokens[1001],char *command){
    pid_t pid = fork(); 
    pid_t processid;
    int suspendstatus;

    if (pid == -1) {
        // stderr
        return 1;
    } 
    else if (pid == 0) {

        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);
  
        execvp(inputtokens[0],inputtokens); 
        
        exit(0);
    }
    else  {

         processid = waitpid(0,&suspendstatus,WUNTRACED);

           if(WIFSTOPPED(suspendstatus)){

           jobspid[listposition] = processid;
             strcpy(jobslist[listposition],command); 
             listposition+=1;
        }

        wait(NULL); 
    }

           return 1;
}

int executeioredirection(char *inputtokens[1001], int tokensize,int inputredirectionispresent,int outputredirectionispresent,int outputappendispresent){

        char *input = (char*)calloc(1000,sizeof(char));
        char *output = (char*)calloc(1000,sizeof(char));
        pid_t pid;
        int inputok = 1;
         pid_t processid;
    int suspendstatus;

  
        for(int i=0;i<tokensize;i++){
            if(strcmp(inputtokens[i],"<") == 0){
                // check if inputtokens[i+1] is a file. Else error

                strcpy(input,inputtokens[i+1]);
                inputtokens[i] = NULL; // '\0'
                inputtokens[i+1] = NULL; // \0
                i=i+1;

                continue;

            }
            if(strcmp(inputtokens[i],">") == 0){
                // check if inputtokens[i+1] is a file. Else creat one

                strcpy(output,inputtokens[i+1]);
                inputtokens[i] = NULL; // \0
                inputtokens[i+1] = NULL; // \0
                i=i+1;


                continue;
        
            }
            if(strcmp(inputtokens[i],">>") == 0){
                // check if inputtokens[i+1] is a file. Else create one

                strcpy(output,inputtokens[i+1]);
                inputtokens[i] = NULL; // \0
                inputtokens[i+1] = NULL;// \0
                i=i+1;

                continue;
            }
        }

        if ((pid = fork()) < 0){
            //stderr
        }

        else if (pid == 0)
        {
        signal(SIGINT,SIG_DFL);
        signal(SIGQUIT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);

            if(inputredirectionispresent){
                if(access(input,F_OK) == 0 && access(input,R_OK) == 0){
                int fd0 = open(input, O_RDONLY);
                dup2(fd0, 0);
                close(fd0);
                inputredirectionispresent =0;
                }

                else{
                    fprintf(stderr,"Error: invalid file\n");
                    fflush(stderr);
                    inputok = 0;

                    return 0;
                }
            }

            if(inputok){

            if(outputredirectionispresent){
                int fd1 = creat(output,0644) ; 
                dup2(fd1, 1);
                close(fd1);
            }

            if(outputappendispresent){
                int fd2 = open(output ,O_RDWR|O_CREAT|O_APPEND) ; 
                dup2(fd2, 1);
                close(fd2);

            }
            // now the child has stdin coming from the input file, 
            // stdout going to the output file, and no extra files open.
            execvp(inputtokens[0],inputtokens); 
            exit(0);

            }
            exit(0);        
        }

        else
        {
         processid = waitpid(0,&suspendstatus,WUNTRACED);

           if(WIFSTOPPED(suspendstatus)){

           jobspid[listposition] = processid;
             strcpy(jobslist[listposition],inputtokens); 
             listposition+=1;
        }       
            wait(NULL);
        }

        return 1;
}

int executepipe(char *inputtokens[1001] ,int tokensize,int inputredirectionispresent,int outputredirectionispresent,int outputappendispresent,int pipecount){
 

    pid_t pid;
    char **pipetokens = (char**)calloc(1002,sizeof(char)) ;
    int fd[pipecount][2];
    int childcount =0;
    pid_t processid;
    int suspendstatus;

    int i = 0;
    int k = 0;
    int p = 0;
    int j = 0;
    int newpipecount=0;
    int pipeinputredirectionispresent =0;
    int pipeoutputredirectionispresent =0;
    int pipeoutputappendispresent =0;

    // for(i=0;i<tokensize;i++){
    // strcpy(pipetokens[i],inputtokens[i]);
    // }

    for(i=0;i<pipecount;i++){

        if(pipe(fd[i])<0){
            fprintf(stderr,"Cannot Create Pipes\n");
            return 1;
        }
    }

    //strcpy(pipetokens[0],inputtokens[0]); // for solving segfault when inputtokens[i-1] is called
    for(i=0;i<tokensize;i++){

        if(i<(tokensize-1)){
            p = i+1;
        }

        else{
            p = i-1;
        }


        if( (i == (tokensize-1))  || ((strcmp(inputtokens[i],"|") == 0)  && (strcmp(inputtokens[p],"|") != 0 )) )   {

            //pid = fork();

            if(i==tokensize-1){
                if(strcmp(inputtokens[i],"<") == 0){
                    pipeinputredirectionispresent = 1;
                }
                if(strcmp(inputtokens[i],">") == 0){
                    pipeoutputredirectionispresent = 1;
                }
                if(strcmp(inputtokens[i],">>") == 0){
                    pipeoutputappendispresent = 1;
                }

                pipetokens[j] = inputtokens[i];
                
                j=j+1;

            }

            childcount+=1;

            if ((pid = fork()) < 0){
               //stderr
            }

            else if(pid == 0){

            signal(SIGINT,SIG_DFL);
            signal(SIGQUIT,SIG_DFL);
            signal(SIGTERM,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            if(newpipecount!=0){
                dup2(fd[newpipecount-1][0],0);
            }

            if(newpipecount!=pipecount){
                dup2(fd[newpipecount][1],1);
            }

            for(k = 0; k < pipecount ;k++){
                close(fd[k][0]);
                close(fd[k][1]);
            }

            if( (newpipecount==0 && inputredirectionispresent==1) || (newpipecount==pipecount && (outputappendispresent+outputredirectionispresent)==1) ) {


               if(executeioredirection(pipetokens,j,pipeinputredirectionispresent,pipeoutputredirectionispresent,pipeoutputappendispresent)==0){
                }
                exit(0);
            }

            else  {
                  //executesimplecommands(pipetokens); 
                  execvp(pipetokens[0],pipetokens);
                  exit(0);
            }               

            }

            else{

            }

            newpipecount+=1;

            j=0;
            pipeinputredirectionispresent = 0;
            pipeoutputappendispresent = 0;
            pipeoutputappendispresent = 0;
            
        }

        else if(strcmp(inputtokens[i],"|")==0  && strcmp(inputtokens[i+1],"|")==0 ){
            return 0;
        }

        else{

            if(strcmp(inputtokens[i],"<") == 0){
                pipeinputredirectionispresent = 1;
            }
            if(strcmp(inputtokens[i],">") == 0){
                pipeoutputredirectionispresent = 1;
            }
            if(strcmp(inputtokens[i],">>") == 0){
                pipeoutputappendispresent = 1;
            }
       
            pipetokens[j] = inputtokens[i];
    

            j=j+1;

        }

    }

        fflush(stdout);
        for( k = 0; k < pipecount; k++ ){
        close(fd[k][0]);
        close(fd[k][1]);
       }

     for(k=0;k<pipecount+1;k++){

        wait(NULL);
     }

    return 0;
}

int main() {

    int i;
    char *command = (char*)calloc(1001,sizeof(char));
    char *inputstring = (char*)calloc(1001,sizeof(char));
    char *inputstring1 = (char*)calloc(1001,sizeof(char));
    char *inputtokens[1001];
    char space[1] = " ";

        signal(SIGINT,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGTERM,SIG_IGN);
        signal(SIGTSTP,SIG_IGN);


    while(1) {

        int pipecount = 0;
        int pipeispresent = 0;
        int incorrectpipe = 0;
        int inputredirectionispresent = 0;
        int inputredirectioncount = 0;
        int outputredirectionispresent = 0;
        int outputredirectioncount = 0;
        int outputappendispresent = 0;
        int outputappendcount = 0;
        int tokensize = 0;
        int relative =0;

        printprompt();

        command= readinp(inputstring1);

        strcpy(inputstring,command);
        inputtokens[0] = strtok(inputstring," ,\n");
        
            
            if(inputtokens[0] == NULL){
                tokensize = 0;
              continue;
            }


            if(strcmp(inputtokens[0],"|")==0){
                // error function
                fprintf(stderr,"Error: invalid command\n");
                continue;
            }

            if(strcmp(inputtokens[0],"<")==0){
                // error function
                fprintf(stderr,"Error: invalid command\n");
                continue;
            }

            if(strcmp(inputtokens[0],">")==0){
                // error function
                fprintf(stderr,"Error: invalid command\n");
                continue;
            }

            if(strcmp(inputtokens[0],">>")==0){
                // error function
                fprintf(stderr,"Error: invalid command\n");
                continue;
            }

        for(i=1;i<1001;i++){
            inputtokens[i] = strtok(NULL," ,\n");


            if(inputtokens[i] == NULL){
                tokensize = i;
                break;
            }

            if(strcmp(inputtokens[i],"|") == 0){
                if(outputredirectioncount+outputappendcount>0){ 
                    incorrectpipe = 1;
                    break;
                } 

                    
                pipeispresent = 1;
                pipecount+=1;
            }
            if(strcmp(inputtokens[i],"<") == 0){

                if(pipecount>0){

                    incorrectpipe = 1;
                    break;
                }

                inputredirectionispresent = 1;
                inputredirectioncount+=1;
            }

            if(strcmp(inputtokens[i],">") == 0){
                outputredirectionispresent = 1;
                outputredirectioncount+=1;
            }

            if(strcmp(inputtokens[i],">>") == 0){
                outputappendispresent = 1;
                outputappendcount+=1;
            }

            if(strcmp(inputtokens[i],"<<")==0){
                incorrectpipe = 1;
                break;
            }
        }

        if((outputappendcount + outputredirectioncount) > 1 || inputredirectioncount > 1 || incorrectpipe > 0){
            fprintf(stderr,"Error: invalid command\n");
            continue;
        }    
            
        // if(strcmp(inputtokens[0],space) == 0){       // Blank line case
        //     continue;
        // }

        if(tokensize>0){

            if(pipecount<1 && (outputappendcount + outputredirectioncount + inputredirectioncount) > 0 ){
                executeioredirection(inputtokens,tokensize,inputredirectionispresent,outputredirectionispresent,outputappendispresent);
                continue;
            }

            if(pipecount>0){
                executepipe(inputtokens,tokensize,inputredirectionispresent,outputredirectionispresent,outputappendispresent,pipecount);
                continue;
            }

            if(builtincommandcheck(inputtokens,tokensize)){
                continue;
            } 

            if(executesimplecommands(inputtokens,command)){
                continue;
             }   

        }
    }
}












