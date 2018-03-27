
/**
* Fase 3 - Práctica 2
* Sistemas Operativos 1
* Autores: Luis Arjona y Hector Santos
*/

#include "fase3.h"

/*
* Contamos los elementos que contiene args
*/
int contar_elementos(char **args) {
    int nargs;
    nargs = 0;
    while (args[nargs] != NULL) {
        nargs++;
    }
    return nargs;
}

/*
* Imprimir error en color rojo y por la salida de errores
*/
void imprimir_error(char *error_msg) {
    if (error_msg == NULL) {
      fprintf(stderr, ANSI_COLOR_RED"Error %d: %s\n", errno, strerror(errno));
      printf(ANSI_COLOR_RESET);
    }else{
      fprintf(stderr, ANSI_COLOR_RED"%s\n", error_msg);
      printf(ANSI_COLOR_RESET);
    }
}

/*
* Simulación de la ejecución del comando 'cd'
*/
int internal_cd(char **args) {
    char path_actual[256];
    int nargs;
    nargs = contar_elementos(args);

    if ( nargs > 2) imprimir_error("Error de sintaxis. Uso: <cd dir_path>");
    else{
        int succes;
        char *ptr_path;
        if (nargs == 1) succes = chdir(getenv("HOME")); // cd sin argumentos, vamos a $HOME
        else succes = chdir(args[1]); // si existe el directorio pasado como argumento, succes = 0
        if (succes != 0) imprimir_error(NULL);
        else {
            ptr_path = getcwd(path_actual, sizeof(path_actual));
            if (ptr_path == NULL) imprimir_error(NULL);
            else {
                printf(ANSI_COLOR_GREEN "Nuevo directorio actual: %s\n", ptr_path);
                printf(ANSI_COLOR_RESET);
            }
        }
    }
    return 1;
}

/*
* Exportamos el valor de las variables de entorno del minishell
*/
int internal_export(char **args) {
    char *text,*nombre,*valor;

    text = NULL;

    if(args[1]!=NULL){ // Si después del export hay algo más
        //args[1]= NOMBRE=VALOR;
        text=args[1];
        const char *separador= "=";
        char *token;
        token =strtok(text,separador);
        nombre=token; // Extraemos nombre
        token = strtok(NULL,separador);
        valor=token; // Extraemos valor
    }

    if (args[1] == NULL || valor==NULL||nombre==NULL){ // error
        imprimir_error("Error de sintaxis. Uso: export Nombre=Valor");
    }else{ // todo correcto
        printf(ANSI_COLOR_GREEN "nombre: %s\nvalor:%s\n",nombre,valor);
        printf("antiguo valor de %s es %s\n",nombre,getenv(nombre));
        setenv(nombre,valor,1);
        printf("ahora valor de %s= %s\n",nombre,getenv(nombre));
        printf(ANSI_COLOR_RESET);
    }
    return 1;
}

int internal_source(char **args) {
    FILE *fp;
    fp = fopen(args[1], "r");
    if (fp == NULL) {
      imprimir_error(NULL);
    } else {
      char linea[LINEA];
      while (fgets(linea, LINEA, fp) != NULL) {
        execute_line(linea);
      }
      fclose(fp);
    }
    return 1;
}

int internal_jobs(char **args) {
    printf("%s\n", "Comando jobs");
    return 1;
}


int check_internal(char **args) {
    char *comando = args[0];
    if (strcmp(comando, "NULL") == 0) {
      return -1;
    } else {
      if (strcmp(comando, "cd") == 0) {
          return internal_cd(args);
      } else if (strcmp(comando, "export") == 0) {
          return internal_export(args);
      } else if (strcmp(comando, "source") == 0) {
          return internal_source(args);
      } else if (strcmp(comando, "jobs") == 0) {
          return internal_jobs(args);
      } else if (strcmp(comando, "exit") == 0) {
          exit(0);
      } else return 0;
    }
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
    if (strncmp(args[i],"#",1) == 0) { // Si no es un comentario lo añadimos como argumento
      break;
    }
    i++;
    token = strtok(NULL, " \n\r"); // Ponemos NULL para no sobrescribir la 'line'
  }
  args[i] =  0; //Null al final, ya que no habra nada mas que trocear
  return i;
}

int execute_line(char *line) {
    char *args[LINEA];
    parse_args(args, line);
    if (check_internal(args) == 0) {
      pid_t pid;
      pid = fork();
      if (pid == -1) {
        imprimir_error(NULL);
        exit(0);
      } else if (pid == 0) {
        printf("[execute_line()→ PID padre: %d]\n", getppid());
        printf("[execute_line()→ PID hijo: %d]\n", getpid());
        if (execvp(args[0], args) < 0) {
          fprintf(stderr, ANSI_COLOR_RED"%s", args[0]);
          imprimir_error(": no se encontró la orden");
          exit(0);
        }
      } else {
        wait(NULL);
      }
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

    fflush(stdin); // liberamos la entrada estandar
    return ptr_line;
}

int main(int argc, char const *argv[]) {
    char line[LINEA];
    while (read_line(line)) {
        execute_line(line);
    }
    return 0;
}
