#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void grsh_loop();
int interact();
int script(char filename[100]);
char *grsh_readln();
char **grsh_tokenize(char *line);
int grsh_run(char **args);
int grsh_exec(char **args);
int grsh_cd(char **args);
int grsh_exit(char **args);
char *builtin_str[] = {"cd","exit"};
int (*builtin_func[]) (char **) = {&grsh_cd,&grsh_exit};

char error_message[30] = "An error has occured\n";

int main(int argc, char **argv){

	if(argc == 1)
	  interact();
	else if(argc == 2)
	  script(argv[1]);
	else
	  write(STDERR_FILENO, error_message, strlen(error_message));

	exit(1);

}

int script(char filename[100]){

	FILE *fptr;
	char line[200];
	char **args;
	fptr = fopen(filename, "r");
	if(fptr == NULL){
	  write(STDERR_FILENO, error_message, strlen(error_message));
	  exit(0);
	}
	else{
	  while(fgets(line, sizeof(line), fptr) != NULL){
	    args = grsh_tokenize(line);
	    grsh_exec(args);
	  }
	}
	free(args);
	fclose(fptr);
	return 1;
}

int interact(){

	grsh_loop();
	return 1;
}

void grsh_loop(){

	char *cmd;
	char **args;
	int status;

	do{
	  printf("grsh> ");
	  cmd = grsh_readln();
	  args = grsh_tokenize(cmd);
	  status = grsh_exec(args);

	  free(args);
	} while(status);
}

char *grsh_readln(){

	char *line = NULL;
	size_t buff = 0;

	if(getline(&line, &buff, stdin) == -1){
	  if(feof(stdin)){
	    exit(1);
	  }
	  else{
	    write(STDERR_FILENO, error_message, strlen(error_message));
	    exit(0);
	  }
	}

	return line;
}

char **grsh_tokenize(char *line){

	int buff = 64;
	int pos = 0;
	char **tokens = malloc(buff * sizeof(char*));
	char *token;

	if(!tokens){
	  write(STDERR_FILENO, error_message, strlen(error_message));
	  exit(0);
	}

	token = strtok(line, " \t\r\n\a");
	while(token != NULL) {
	  tokens[pos] = token;
	  pos++;

	  if(pos >= buff){
	    buff += 64;
	    tokens = realloc(tokens, buff * sizeof(char*));
	    if(!tokens){
	      write(STDERR_FILENO, error_message, strlen(error_message));
	      exit(0);
	    }
	  }

	  token = strtok(NULL, " \t\r\n\a");
	}
	tokens[pos] = NULL;
	return tokens;
}

int grsh_run(char **args){

	pid_t pid;
	int status;

	pid = fork();
	if(pid == 0){
	  if(execvp(args[0], args) == -1){
	    write(STDERR_FILENO, error_message, strlen(error_message));
	  }
	  exit(0);
	}
	else if(pid < 0){
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else{
	  do{
	    waitpid(pid, &status, WUNTRACED);
	  } while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int grsh_exec(char **args){

	int i;

	if(args[0] == NULL) {
	  return 1;
	}

	for(i = 0; i < (sizeof(builtin_str) / sizeof(char *)); i++){
	  if(strcmp(args[0], builtin_str[i]) == 0){
	    return(*builtin_func[i])(args);
	  }
	}
	int ret = grsh_run(args);
	return ret;
}

int grsh_cd(char **args){

	if(args[1] == NULL){
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else{
	  if(chdir(args[1]) != 0){
	    write(STDERR_FILENO, error_message, strlen(error_message));
	  }
	}
	return 1;
}


int grsh_exit(char **args){

	return 0;
}
