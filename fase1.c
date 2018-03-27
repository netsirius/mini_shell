
/**
* Fase 1 - Práctica 2
* Sistemas Operativos 1
* Autores: Luis Arjona y Hector Santos
*/

#include "fase1.h"

int internal_cd(char **args){
  printf("%s\n", "Comando cd");
  return 1;
}
int internal_export(char **args){
  printf("%s\n", "Comando export");
  return 1;
}
int internal_source(char **args){
  printf("%s\n", "Comando source");
  return 1;
}
int internal_jobs(char **args){
  printf("%s\n", "Comando jobs");
  return 1;
}

/*
* Llamamos a la función del comando correspondiente
*/
int check_internal(char **args){
  char *comando = args[0];

  if (strcmp(comando,"cd") == 0) {
    return internal_cd(args);
  } else if (strcmp(comando,"export") == 0) {
    return internal_export(args);
  } else if (strcmp(comando,"source") == 0) {
    return internal_source(args);
  } else if (strcmp(comando,"jobs") == 0) {
    return internal_jobs(args);
  } else if (strcmp(comando,"exit") == 0){
    exit(0);
  }

  return 0;

}

/*
*   El contenido de 'line' es troceado, es decir cada palabra separada por espacios, saltos de línea o tabulaciones es insertada en el array 'args'
*/
int parse_args(char **args, char *line) {
  int i = 0;
  char *token;
  token = strtok(line, " \n\r");
  while (token != NULL) {
    args[i] = token;
    if (strncmp(args[i],"#",1) != 0) { // Si no es un comentario lo añadimos como argumento
      i++;
    }
    token = strtok(NULL, " \n\r"); // Ponemos NULL para no sobrescribir la 'line'
  }
  args[i] =  0; //Null al final, ya que no habra nada mas que trocear
  return i;
}

int execute_line(char *line) {
  char *args[LINEA];
  parse_args(args, line);

  if (check_internal(args) == 0) {
    fprintf(stderr, ANSI_COLOR_RED"miniShell %s: command not found\n", args[0]);
    printf(ANSI_COLOR_RESET);
  }
  return 0;
}

char *read_line(char *line) {
    char *ptr_line;
    printf(ANSI_COLOR_BLUE"%s %s", getenv("USER"), PROMPT);
    printf(ANSI_COLOR_RESET);
    ptr_line = fgets(line, LINEA, stdin);

    if (ptr_line[0] == '\n') {
        line = ptr_line;
        strcpy(ptr_line, "NULL");
    }

    fflush(stdin);
    return ptr_line;
}

int main(int argc, char const *argv[]) {
  char line[LINEA];
  while (read_line(line)) {
    execute_line(line);
  }
  return 0;
}
