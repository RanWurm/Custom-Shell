#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMANDS 100
#define MAX_COMMAND_lEN 100
#define PATH "PATH"


typedef struct History{
    char commands[MAX_COMMANDS][MAX_COMMAND_lEN];
    int counter;
}History;


typedef struct Terminal{
    struct History* history;
    char* workingDir;
    int terminalIsRunning;
}Terminal;

struct History* initialHistory(){
    History* ret = (History*)malloc(sizeof(History));
    if(ret ==NULL){
        free(ret);
        printf("failed To Malloc History!\n");
        return NULL;
    }
    ret->counter = 0;
    return ret;
}


struct Terminal* creaTerminal(char* workingDir){
    Terminal* terminal = (Terminal*)malloc(sizeof(Terminal));
    terminal->terminalIsRunning = 1;
    terminal->history = initialHistory();
    terminal->workingDir = workingDir;
    return terminal;
}

void printHistory(History* history){
    int len = (history->counter) - 1;
    for(int i = len; i >=0; i--){
        printf("%s\n", history->commands[i]);
    }
}

int addToHistory(Terminal* t,char* input){
    int index = t->history->counter;
    if(index == MAX_COMMANDS){
        printf("command history is full!\n");
        return 1;
    }
    strcpy(t->history->commands[index],input);
    t->history->commands[index][MAX_COMMAND_lEN - 1 ] = '\0';
    t->history->counter++;
    return 0;
}

void freeHistoryMem(History* his){
    free(his);
    his = NULL;
}

void ExistShell(Terminal* t){
    freeHistoryMem(t->history);
    free(t);
}


int processBuildInCommands(char* input,Terminal* terminal){
    char history[] = "history";
    char pwd[] = "pwd";
    char exit[] = "exit";
    char cd[] = "cd ";
    if(terminal->history->counter ==   MAX_COMMANDS) {
        printf("max commands reached!\n");
    }else if(strcmp(input,history) == 0) {
        printHistory(terminal->history);
    }else if(strcmp(input,pwd) == 0){
        printf("%s\n",terminal->workingDir);
    }else if(strcmp(input,exit) == 0) {
        printf("exiting!\n");
        terminal->terminalIsRunning = 0;
    }else if(strncmp(input, cd, 3) == 0){
        char* tmp = input + 3;
        strcpy(terminal->workingDir,tmp);
    }else{
        return 0;
    }
    return 1;
}


int processCustomCommands(char* input){
    char* argv[MAX_COMMANDS] = {0};
    int count = 0;
    char* delim = " ";
    argv[count] = strtok(input,delim);

    while(argv[count] != NULL){
        ++count;
        argv[count] = strtok(NULL,delim);
    }

    pid_t pid = fork();
    if(pid < 0 ){
        perror("fork has failed!\n");
    }else if(pid == 0) { // in child process
        execvp(argv[0],argv);
        perror("exec failure");
        exit(1);
    }else{//in parent
        waitpid(pid,NULL,0);
        return 1;
    }
    return 0;
}




void processCommand(char* input,Terminal* terminal){
    int isBuiltInProcess = processBuildInCommands(input, terminal);
    int isCustomProcess;
    if(!isBuiltInProcess){
       isCustomProcess =  processCustomCommands(input);
    }
    if(isBuiltInProcess || isCustomProcess){
        addToHistory(terminal,input);
    }else{
        perror("failed to execute!\n");
    }


}

void setPathEnviVariables(int argc,char* argv[]){
    char* newPath = getenv(PATH);
    for (int i = 1; i < argc; i++){
        strcat(newPath,":");
        strcat(newPath,argv[i]);
    }
    setenv(PATH,newPath,1);
}

int main(int argc,char* argv[]){
    Terminal* terminal = creaTerminal(argv[0]);
    setPathEnviVariables(argc,argv);
    char userInput[MAX_COMMAND_lEN];
    int len;
    while(terminal->terminalIsRunning){
        printf("$ ");
        fflush(stdout);
        if(fgets(userInput,MAX_COMMAND_lEN,stdin)== NULL){
            perror("failed to get Input");
        }
        len =(int)strlen(userInput) - 1;
        userInput[len] = 0;
        processCommand(userInput,terminal);

    }
    ExistShell(terminal);
    exit(0);
}
