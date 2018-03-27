
/**
* Fase 6 - Práctica 2
* Sistemas Operativos 1
* Autores: Luis Arjona y Hector Santos
*/

#include "fase6.h"

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
      fprintf(stderr, ANSI_COLOR_RESET);
    }  else{
      fprintf(stderr, ANSI_COLOR_RED"%s\n", error_msg);
      fprintf(stderr, ANSI_COLOR_RESET);
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
  int nargs;
  nargs = contar_elementos(args);
  if (nargs > 1) imprimir_error("Error de sintaxis. Uso: <jobs>");
  else {
    for (int i = 1; i <= n_pids; i++) {
      printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status,
      jobs_list[i].command_line);
    }
  }
  return 1;
}

int check_internal(char **args) {
    if (args[0] == 0) {
      return -1;
    } else {
      if (strcmp(args[0], "cd") == 0) {
          return internal_cd(args);
      } else if (strcmp(args[0], "export") == 0) {
          return internal_export(args);
      } else if (strcmp(args[0], "source") == 0) {
          return internal_source(args);
      } else if (strcmp(args[0], "jobs") == 0) {
          return internal_jobs(args);
      } else if (strcmp(args[0], "exit") == 0) {
          exit(0);
      } else if (strcmp(args[0], "fg") == 0) {
          return internal_fg(args);
      } else if (strcmp(args[0], "bg") == 0) {
          return internal_bg(args);
      } else {
        return 0;
      }
    }
}

/*
*   El contenido de 'line' es troceado, es decir cada palabra separada por espacios, saltos de línea o tabulaciones es insertada en el array 'args'
*/
int parse_args(char **args, char *line) {
  char lineaux[LINEA];
  strcpy(lineaux, line);
  int i = 0;
  char *token;
  token = strtok(lineaux, " \n\r");
  while (token != NULL) {
    args[i] = token;
    if (strncmp(args[i],"#",1) == 0) { // Si no es un comentario lo añadimos como argumento
      break;
    }
    i++;
    token = strtok(NULL, " \n\r"); // Ponemos NULL para no sobrescribir la 'line'
  }
  args[i] =  0; //Null al final, ya que no habra nada mas que trocear
  strtok(line, "\n\r");
  return i;
}

int is_background (char **args) {
  int i = 0;
  while (args[i] != NULL) {
    i++;
  }
  if (strcmp(args[i-1], "&") == 0) {
    args[i-1] = NULL;
    return 1;
  }
  return 0;
}

void reaper(int signum) {
  signal(SIGCHLD, reaper);
  pid_t ended;
  int status;
  while ((ended = waitpid(-1, &status, WNOHANG)) > 0) {
      if (jobs_list[0].pid == ended) {
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'F';
        switch (status) {
          case 0:
            printf("[reaper()→ Proceso hijo %d finalizado con exit code: %d]\n",
            ended, status);
            break;
          default:
            printf("[reaper()→ Proceso hijo %d finalizado por señal: %d]\n",
            ended, status);
            break;
        }
      } else {
        int pos = jobs_list_find(ended);
        fprintf(stderr, ANSI_COLOR_RED "\rTerminado PID %d en jobs_list[%d] con"
        " status %d\n", ended, pos, status);
        fprintf(stderr, ANSI_COLOR_RESET);
        jobs_list_remove(pos);
      }
  }
}

void ctrlc(int signum) {
  signal(SIGINT, ctrlc);
  printf("\n[ctrlc()→ Soy el proceso con PID %d, el proceso en foreground es %d "
  "(%s)]\n", pidshell, jobs_list[0].pid, jobs_list[0].command_line);
  if (jobs_list[0].pid > 0) {
    if (jobs_list[0].pid != pidshell) {
      printf("[ctrlc()→ Señal SIGTERM enviada a %d (%s) por %d]\n",
              jobs_list[0].pid, jobs_list[0].command_line, pidshell);
      kill(jobs_list[0].pid, SIGTERM);
    } else printf("Señal SIGTERM no enviada debido a que el proceso en "
                  "foreground es el shell\n");
  } else printf("Señal SIGTERM no enviada debido a que no hay "
          "proceso en foreground\n");
}

int jobs_list_add(pid_t pid, char status, char *command_line) {
  if (n_pids < N_JOBS) {
    n_pids++;
    jobs_list[n_pids].pid = pid;
    jobs_list[n_pids].status = status;
    strcpy(jobs_list[n_pids].command_line, command_line);
    return 0;
  } else return -1;
}

int jobs_list_find(pid_t pid) {
  int p_pos;
  p_pos = 1;
  while (jobs_list[p_pos].pid != pid) p_pos++;
  return p_pos;
}

int jobs_list_remove(int pos) {
  if (pos < n_pids) {
    jobs_list[pos] = jobs_list[n_pids];
  }
  n_pids--;
  return 0;
}

int internal_fg(char **args) {
  int nargs, pos;
  nargs = contar_elementos(args);
  if (nargs != 2) imprimir_error("Error de sintaxis. Uso: <fg NºTRABAJO>");
  else {
    pos = atoi(args[1]);
    if (pos > 0 && pos <= n_pids) {
      if (jobs_list[pos].status == 'D') {
        kill(jobs_list[pos].pid, SIGCONT);
        jobs_list[pos].status = 'E';
        printf("[internal_fg()→ señal 18 (SIGCONT) enviada a %d]\n",
                jobs_list[pos].pid);
      }
      int i = 0;
      while (jobs_list[pos].command_line[i] != 0 &&
              jobs_list[pos].command_line[i] != '&') {  // buscamos &
        i++;
      }
      if (jobs_list[pos].command_line[i] == '&')
      jobs_list[pos].command_line[i-1] = 0;  // quitamos " &"
      jobs_list[0] = jobs_list[pos];
      jobs_list_remove(pos);
      printf("%s\n", jobs_list[0].command_line);
      while (jobs_list[0].pid != 0) {
        pause();
      }
    } else {
      fprintf(stderr, ANSI_COLOR_RED"%s", args[1]);
      imprimir_error(": no existe ese trabajo");
    }
  }
  return 1;
}

int internal_bg(char **args) {
  int nargs, pos;
  nargs = contar_elementos(args);
  if (nargs != 2) imprimir_error("Error de sintaxis. Uso: <bg NºTRABAJO>");
  else {
    pos = atoi(args[1]);
    if (pos > 0 && pos <= n_pids) {
      if (jobs_list[pos].status == 'E') {
        fprintf(stderr, ANSI_COLOR_RED"%s", args[1]);
        imprimir_error(": el trabajo ya está en 2º plano");
      } else {
        jobs_list[pos].status = 'E';
        int i = 0;
        while (jobs_list[pos].command_line[i] != 0 &&
                jobs_list[pos].command_line[i] != '&') {  // tiene & ?
          i++;
        }
        if (jobs_list[pos].command_line[i] == 0) { // añadimos " &"
          jobs_list[pos].command_line[i] = ' ';
          jobs_list[pos].command_line[i+1] = '&';
          jobs_list[pos].command_line[i+2] = 0;
        }
        kill(jobs_list[pos].pid, SIGCONT);
        printf("[internal_bg()→ señal 18 (SIGCONT) enviada a %d]\n",
                jobs_list[pos].pid);
        printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid,
                jobs_list[pos].status, jobs_list[pos].command_line);
      }
    } else {
      fprintf(stderr, ANSI_COLOR_RED"%s", args[1]);
      imprimir_error(": no existe ese trabajo");
    }
  }
  return 1;
}

void ctrlz(int signum) {
  signal(SIGTSTP, ctrlz);
  if (jobs_list[0].pid > 0) {
    if (jobs_list[0].pid != pidshell) {
      printf("\n[ctrlz()→ Señal 20 (SIGTSTP) enviada a %d (%s) por %d]\n",
              jobs_list[0].pid, jobs_list[0].command_line, pidshell);
      kill(jobs_list[0].pid, SIGTSTP);
      jobs_list[0].status = 'D';
      if (jobs_list_add(jobs_list[0].pid, jobs_list[0].status,
                        jobs_list[0].command_line) == 0) {
        int pos;
        pos = jobs_list_find(jobs_list[0].pid);
        jobs_list[0].pid = 0;
        printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid,
                jobs_list[pos].status, jobs_list[pos].command_line);
      } else {
        fprintf(stderr, ANSI_COLOR_RED "(%s) no se ha podido ejecutar en "
        "background, número máximo de procesos en background alcanzado.\n",
        jobs_list[0].command_line);
        fprintf(stderr, ANSI_COLOR_RESET);
      }
    } else printf("\rSeñal SIGTSTP no enviada debido a que el proceso en "
                  "foreground es el shell\n");
  } else printf("\rSeñal SIGTSTP no enviada debido a que no hay "
          "proceso en foreground\n");
}

int is_output_redirection (char **args){
  // if(contar_elementos(args) == 3 && *args[1] == '>' && args[2] != NULL){
  //   return 1;
  // }
  int cont = 0;
  while (args[cont] != NULL){
    if (*args[cont] == '>' && args[cont+1] != NULL) return 1;
    cont++;
  }
  return 0;
}

void redireccionar(char **args){
  int fdesc;
  int nargs;
  nargs = contar_elementos(args);
  fdesc = open (args[nargs-1],  O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fdesc < 0) {
    imprimir_error(NULL);
    exit(0);
  }else{
    args[nargs-2] = 0; // ponemos NULL en la posición que contiene '>'
    args[nargs-1] = 0; // ponemos NULL en la posición que contiene el fichero
    dup2(fdesc,1);
    close(fdesc);
  }
}

int execute_line(char *line) {
  char *args[LINEA];
  parse_args(args, line);
  if (check_internal(args) == 0) {
    int is_bg;
    is_bg = is_background(args);
    pid_t pid;
    pid = fork();
    if (pid == -1) {
      imprimir_error(NULL);
      exit(0);
    } else if (pid == 0) {  // HIJO
      signal(SIGCHLD, SIG_DFL);
      if (is_bg == 1) {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
      } else {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_DFL);
      }
      if (is_output_redirection(args)) {
        printf("%s\n", "Es una redireccion");
        redireccionar(args);
      }
      execvp(args[0], args);
      fprintf(stderr, ANSI_COLOR_RED"%s", args[0]);
      imprimir_error(": no se encontró la orden");
      exit(0);
    } else {  // PADRE
      pidshell = getpid();
      printf("[execute_line()→ PID padre: %d]\n", pidshell);
      printf("[execute_line()→ PID hijo: %d]\n", pid);
      if (is_bg == 0) {
        jobs_list[0].pid = pid;
        jobs_list[0].status = 'E';
        strcpy(jobs_list[0].command_line, line);
        while (jobs_list[0].pid != 0) {
          pause();
        }
      } else {
        if (jobs_list_add(pid, 'E', line) != 0) {
          fprintf(stderr, ANSI_COLOR_RED "(%s) no se ha podido ejecutar en background, "
          "número máximo de procesos en background alcanzado.\n", line);
          fprintf(stderr, ANSI_COLOR_RESET);
          exit(0);
        } else {
          printf("[%d] %d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid,
          jobs_list[n_pids].status, jobs_list[n_pids].command_line);
        }
      }
    }
  }
  return 0;
}

char *getRelPath(char **args) {
  char lineaux[LINEA]; // contenedor auxiliar
  char path_actual[256]; // contenedor ppath absoluto
  strcpy(lineaux, getcwd(path_actual, sizeof(path_actual))); // Dejamos line sin modificar con el comando entero
  int i = 0;
  char *token;
  token = strtok(lineaux, "/");
  while (token != 0) { // Troceamos los distintos directorios del path absoluto
    args[i] = token;
    i++;
    token = strtok(NULL, "/"); // Ponemos NULL para no sobreescribir
  }
  return args[i-1]; // Devolvemos el último directorio del path absoluto
}

char *read_line(char *line) {
    char *ptr_line;
    sleep(0.5); // PROVISIONAL
    char *args[LINEA];
    getRelPath(args);
    printf(ANSI_COLOR_GREEN"%s"ANSI_COLOR_BLUE":~/%s"ANSI_COLOR_RESET"%s", getenv("USER"),getRelPath(args), PROMPT);
    printf(ANSI_COLOR_RESET);
    fflush(stdin);
    ptr_line = fgets(line, LINEA, stdin);

     if (ptr_line == NULL && !feof(stdin)) {
        ptr_line = line;
        ptr_line[0] = 0;
    }
    fflush(stdin); // liberamos la entrada estandar
    fflush(stdout); // liberamos la salida estandar
    return ptr_line;
}

int main(int argc, char const *argv[]) {
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);
    n_pids = 0;
    char line[LINEA];
    while (read_line(line)) {
      execute_line(line);
    }
    return 0;
}
