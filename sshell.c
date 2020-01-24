#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD 512
#define MAX_ARG 16
#define MAX_LEN 32

struct command{
  int count;
  char* real_cmd;
  char* arguments[MAX_ARG];
  char* fileName;
  char* text;
};

struct command splitRegularCommand(char* input){
  struct command c;
  char* tok = malloc(MAX_LEN*sizeof(char));

  tok = strtok(input," ");
  c.real_cmd = tok;
  c.arguments[0] = tok;
  c.count = 1;

  while(tok != NULL){
    tok = strtok(NULL, " ");
    c.arguments[c.count] = tok;
    c.count ++;
  }
  return c;
}

struct command outputRedirectionCommand(char* input){
  struct command c;
  char* tok = malloc(MAX_LEN*sizeof(char));
  char* tok2 = malloc(MAX_LEN*sizeof(char));
  char* trans = malloc(MAX_LEN*sizeof(char));
  char* tempCopy = malloc(MAX_CMD*sizeof(char));

  strcpy(tempCopy,input);

  tok = strtok(tempCopy, ">");
  trans = tok;

  while(tok != NULL){
    tok = strtok(NULL, ">");
    c.fileName = tok;
    break;
  }

  tok2 = strtok(trans, " ");
  c.real_cmd = tok2;

  while(tok2 != NULL){
    tok2 = strtok(NULL, " ");
    c.text = tok2;
    break;
  }
  return c;
}

//char* | char* | char*
char** pipeCommand(char* input){
  char** pipeCmd = malloc(MAX_CMD*sizeof(char*));
  char* tok = malloc(MAX_CMD*sizeof(char));
  char* tempCopy = malloc(MAX_CMD*sizeof(char));
  int pipeIndex = 0;

  strcpy(tempCopy,input);

  tok = strtok(tempCopy, "|");
  pipeCmd[pipeIndex] = tok;

  while(tok != NULL){
    pipeIndex ++;
    tok = strtok(NULL, "|");
    pipeCmd[pipeIndex] = tok;
  }
  return pipeCmd;
}

int countPipeNum(char* input){
  int num = 0;

  for(int i = 0; i < strlen(input); i++){
    if(input[i] == '|'){
      if(i == 0 || i == strlen(input)-1){
        fprintf(stderr, "Error: Invalid command line\n");
        return 0;
      }else{
        num++;
      }
    }
  }
  return num;
}

char* eliminateWhitespace(char* input){
  char* cmdCopy = malloc(MAX_CMD*sizeof(char));
  strcpy(cmdCopy,input);

  for(int i=0; i < strlen(input);i++){
    if(i != 0){
      if(cmdCopy[i] == ' '){
        if(cmdCopy[i-1] == '>'){
          //https://stackoverflow.com/questions/5457608/how-to-remove-the-character-at-a-given-index-from-a-string-in-c
          memmove(&cmdCopy[i],&cmdCopy[i+1],strlen(cmdCopy)-i);
        }else if(cmdCopy[i+1] == '>'){
           memmove(&cmdCopy[i],&cmdCopy[i+1],strlen(cmdCopy)-i);
         }
      }
    }
  }
  return cmdCopy;
}


int main(void)
{
  while (1) {
    char *nl;
    char* cmd = malloc(MAX_CMD*sizeof(char));
    char* cmdCopy = malloc(MAX_CMD*sizeof(char));
    char* path = malloc(MAX_CMD*sizeof(char));
    char* pathname = malloc(MAX_CMD*sizeof(char));
    //pipes
    //Not used in this submission but will fixed after submitted.
    //char** pipeCmd = malloc(MAX_CMD*sizeof(char*));
    char* pCopy = malloc(MAX_CMD*sizeof(char));

    int status;
    pid_t pid;
    pid_t pidPipe;

    fflush(stderr);
    printf("sshell$ ");
    fflush(stdout);

    fgets(cmd,MAX_CMD,stdin);

    /* Print command line if stdin is not provided by terminal */
    if (!isatty(STDIN_FILENO)) {
            printf("%s", cmd);
            fflush(stdout);
    }

    /* Remove trailing newline from command line */
    nl = strchr(cmd, '\n');
    if (nl){
      *nl = '\0';
    }
    if(cmd[0] == '\n'){
      continue;
    }

    strcpy(cmdCopy,cmd);
    strcpy(pCopy,cmd);

    struct command rcmd;

    if(strstr(cmd, "|") == NULL){//no pipe
      if(strstr(cmd, ">") != NULL){//redirection commands.
        cmdCopy = eliminateWhitespace(cmdCopy);
        rcmd = outputRedirectionCommand(cmdCopy);
        if(strcmp(rcmd.real_cmd, " ") == 0 || cmd[0] == '>'){
          fprintf(stderr,"Error: missing command\n");
          exit(1);
        }
      }else{//builtin commands
        rcmd = splitRegularCommand(cmdCopy);

        if(strcmp(rcmd.real_cmd, " ") == 0){
          fprintf(stderr,"Error: missing command\n");
          exit(1);
        }else if(strcmp(rcmd.real_cmd, "exit") == 0){
          fprintf(stderr, "Bye...\n");
          fprintf(stderr,"+ completed '%s' [%d]\n",cmd,WEXITSTATUS(status));
          exit(0);
        }
      }

      if(strcmp(rcmd.real_cmd, "cd") == 0 && strstr(cmd, ">") == NULL){
          getcwd(path,MAX_CMD);
          if(rcmd.arguments[1] == NULL){
            fprintf(stderr, "Error: No such directory\n");
          }else{
            chdir(rcmd.arguments[1]);
          }
        }

      pid = fork();

      if(pid == 0){//child
        if(strstr(cmd,">") != NULL){
          int fd;

          if(rcmd.fileName == NULL){
            fprintf(stderr, "Error: no output file\n");
            exit(1);
          }else{
            chdir(rcmd.fileName);
            getcwd(pathname,MAX_CMD);

            strcat(pathname, "/");
            strcat(pathname,rcmd.fileName);

            if(!(fopen(pathname,"w"))){
              fprintf(stderr, "Error: cannot open output file\n");
            }
            fd = open(pathname, O_RDWR);

            if(fd >= 0){
              dup2(fd,STDOUT_FILENO);
              printf("%s\n",rcmd.text);
              close(fd);
              exit(0);
            }else{
              fprintf(stderr,"Error: no such directory\n");
              close(fd);
              exit(1);
            }
          }
        }

        if(rcmd.count > MAX_ARG){
          fprintf(stderr, "Error: too many process arguments\n");
          exit(1);
        }

        if(strcmp(rcmd.real_cmd, "pwd") == 0){
          getcwd(path,MAX_CMD);
          fprintf(stdout, "%s\n",path);
          exit(0);
        }

        if(strcmp(rcmd.real_cmd, "cd") == 0 && strstr(cmd, ">") == NULL){
          getcwd(path,MAX_CMD);
          if(rcmd.arguments[1] == NULL){
            exit(1);
          }else{
            exit(0);
          }
        }

        execvp(rcmd.real_cmd,rcmd.arguments);
        fprintf(stderr,"Error: no such directory\n");
        exit(1);
      }else if(pid > 0){//parent
        waitpid(-1,&status,0);
        if(WEXITSTATUS(status) == 0){
          //cmdCopy(strlen(cmdCopy)-1) = '\0';
          fprintf(stderr,"+ completed '%s' [%d]\n",cmd,WEXITSTATUS(status));
        }
      }
    }else{//PIPES
      int pNum = 0;

      //pipeCmd = pipeCommand(pCopy);
      pNum = countPipeNum(cmd);
      //https://www.programiz.com/c-programming/c-structures-pointers
      struct command* pipes = malloc(pNum*sizeof(struct command));

      int fdp[2];
      int stdin = 0;
      int count[pNum];

      for(int i = 0; i < pNum; i ++){
        pipe(fdp);

        if(pipe(fdp) == -1){
          perror("Pipe");
          exit(EXIT_FAILURE);
        }else{
          pidPipe = fork();
          if(pidPipe < 0){
            perror("fork");
            exit(EXIT_FAILURE);
          }else if(pidPipe == 0){// child
            dup2(stdin,STDIN_FILENO);

            if(i != pNum-1){//when there are other programs left.
              dup2(fdp[1],STDOUT_FILENO);
              close(fdp[0]);
            }else{
              close(fdp[1]);
            }

            execvp(pipes[i].real_cmd,pipes[i].arguments);
            fprintf(stderr,"Error: no such directory\n");
            exit(1);
          }else{//parent
            wait(&status);
            close(fdp[1]);
            stdin = fdp[0];
            count[i] = WEXITSTATUS(status);
            fprintf(stderr,"[%d]", WEXITSTATUS(status));
          }
        }
      }

      char* message = malloc(MAX_CMD*sizeof(char));


      strcat(message," + completed '");
      strcat(message, cmd);
      strcat(message, "' ");

      for(int m = 0; m < pNum; m++){
        char* temp = malloc(MAX_CMD*sizeof(char));
        //https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
        sprintf(temp,"%d",count[m]);
        strcat(message, "[");
        strcat(message, temp);
        strcat(message,"]");
      }
      fprintf(stderr, "%s\n", message);
    }

  }
      return EXIT_SUCCESS;
}
