
/**
* Fase 7 - Práctica 2
* Sistemas Operativos 1
* Autores: Luis Arjona y Hector Santos
*/

#include "shell.h"

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

/*
* Ejecuta los comandos contenidos en el fichero indicado
*/
int internal_source(char **args) {
    FILE *fp;
    fp = fopen(args[1], "r");
    if (fp == NULL) {
      imprimir_error(NULL);
    } else {
      char linea[LINEA];
      while (fgets(linea, LINEA, fp) != NULL) {
        execute_line(linea); // Ejecutamos linea a linea
      }
      fclose(fp);
    }
    return 1;
}

/*
* Muestra por pantalla la lista de trabajos que hay en segundo plano
*/
int internal_jobs(char **args) {
  int nargs;  // contendrá el nº de tokens que tiene args
  nargs = contar_elementos(args);
  // si hay más de un token se imprime un error (el comando debe ser sólo jobs)
  if (nargs > 1) imprimir_error("Error de sintaxis. Uso: <jobs>");
  else {
    for (int i = 1; i <= n_pids; i++) {
      printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status,
      jobs_list[i].command_line); // imprime cada elemento
    }
  }
  return 1;
}

/*
* Devuelve 0 si no se trata de una función interna; si se trata de una, cada
* función interna devuelve 1 y, por tanto, check_internal devuelve también 1.
* Si no hay ningún token devuelve -1.
*/
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
* El contenido de 'line' es troceado, es decir, cada palabra separada por
* espacios, saltos de línea o tabulaciones es insertada en el array 'args' en
* una posición diferente. Devuelve el número de tokens.
*/
int parse_args(char **args, char *line) {
  char lineaux[LINEA];
  strcpy(lineaux, line);  // Dejamos line sin modificar con el comando entero
  int i = 0;
  char *token;
  token = strtok(lineaux, " \n\r");
  while (token != NULL) {
    args[i] = token;
    if (strncmp(args[i],"#",1) == 0) { // Si no es un comentario lo añadimos como argumento
      break;
    }
    i++;
    token = strtok(NULL, " \n\r"); // Ponemos NULL para no sobreescribir
  }
  args[i] =  0; // Null al final, ya que no habrá nada más que trocear
  strtok(line, "\n\r"); // Le quitamos el salto de línea a line
  return i;
}

/*
* Indica si el comando escrito es de segundo plano (tiene & al final) con un 1,
* en caso contrario devuelve 0.
*/
int is_background (char **args) {
  int i = 0;
  while (args[i] != NULL) { // llega hasta el último token
    i++;
  }
  if (strcmp(args[i-1], "&") == 0) {  // si el último token es &
    args[i-1] = NULL; // se quita el & para que la instrucción pueda ejecutarse
    return 1;
  }
  return 0;
}

/*
* Cuando detecta que el hijo ha finalizado, lo elimina del registro en el que se
* encuentre para hacer sitio a otros procesos hijo.
*/
void reaper(int signum) {
  signal(SIGCHLD, reaper);  // señal que detecta cuando un hijo finaliza
  pid_t ended;
  int status;
  while ((ended = waitpid(-1, &status, WNOHANG)) > 0) {
    // ended contiene el pid del proceso finalizado
      if (jobs_list[0].pid == ended) {  // si se trata del proceso en foreground
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
      } else {  // si se trata de un proceso en background
        int pos = jobs_list_find(ended);  // encuentra la posición del proceso
        fprintf(stderr, ANSI_COLOR_YELLOW "\rTerminado PID %d en jobs_list[%d] con"
        " status %d\n", ended, pos, status);
        fprintf(stderr, ANSI_COLOR_RESET);
        jobs_list_remove(pos);  // borra el proceso de la lista
      }
  }
}

/*
* Finaliza el proceso que se encuentra en foreground, si hay alguno.
*/
void ctrlc(int signum) {
  signal(SIGINT, ctrlc);
  printf("\n[ctrlc()→ Soy el proceso con PID %d, el proceso en foreground es %d "
  "(%s)]\n", pidshell, jobs_list[0].pid, jobs_list[0].command_line);
  if (jobs_list[0].pid > 0) {
    if (jobs_list[0].pid != pidshell &&
              (strcmp(jobs_list[0].command_line, "./fase7") != 0)) {
      printf("[ctrlc()→ Señal SIGTERM enviada a %d (%s) por %d]\n",
              jobs_list[0].pid, jobs_list[0].command_line, pidshell);
      kill(jobs_list[0].pid, SIGTERM);
    } else printf("Señal SIGTERM no enviada debido a que el proceso en "
                  "foreground es el shell\n");
  } else printf("Señal SIGTERM no enviada debido a que no hay "
          "proceso en foreground\n");
}

/*
* Añade un proceso a la lista de procesos, si hay hueco. Si no, devuelve -1
*/
int jobs_list_add(pid_t pid, char status, char *command_line) {
  if (n_pids < N_JOBS) {  // si hay hueco
    n_pids++; // aumenta el nº de procesos en jobs_list
    jobs_list[n_pids].pid = pid;
    jobs_list[n_pids].status = status;  // estado del proceso
    strcpy(jobs_list[n_pids].command_line, command_line); // string con el comando
    return 0;
  } else return -1;
}

/*
* Encuentra y devuelve la posición en la que se encuentra el proceso en
* background con el pid pasado por parámetro
*/
int jobs_list_find(pid_t pid) {
  int p_pos;
  p_pos = 1;  // posición del primer proceso en background
  while (jobs_list[p_pos].pid != pid) p_pos++;
  return p_pos;
}

/*
* Elimina de la lista el proceso en background que se encuentra en la posición
* pasada por parámetro, moviendo el último proceso de la lista a dicha posición.
*/
int jobs_list_remove(int pos) {
  if (pos < n_pids) {
    jobs_list[pos] = jobs_list[n_pids];
  }
  n_pids--; // decrementamos el nº de procesos en la lista
  return 0;
}

/*
* Mueve un proceso en background a foreground (y lo elimina de background)
*/
int internal_fg(char **args) {
  int nargs, pos;
  nargs = contar_elementos(args);
  // si el comando no tiene el nº de tokens adecuado
  if (nargs != 2) imprimir_error("Error de sintaxis. Uso: <fg NºTRABAJO>");
  else {
    pos = atoi(args[1]);  // convierte la posición de string a int
    if (pos > 0 && pos <= n_pids) { // si la posición de la lista es de un
      // proceso en background
      if (jobs_list[pos].status == 'D') { // si está detenido
        kill(jobs_list[pos].pid, SIGCONT);  // hace que el proceso continúe
        jobs_list[pos].status = 'E';  // se vuelve a poner como ejecutándose
        printf("[internal_fg()→ señal 18 (SIGCONT) enviada a %d]\n",
                jobs_list[pos].pid);
      }
      int i = 0;
      while (jobs_list[pos].command_line[i] != 0 &&
              jobs_list[pos].command_line[i] != '&') {  // buscamos "&"
        i++;
      }
      if (jobs_list[pos].command_line[i] == '&')
      jobs_list[pos].command_line[i-1] = 0;  // quitamos " &"
      jobs_list[0] = jobs_list[pos];  // mueve el proceso a foreground
      jobs_list_remove(pos);  // elimina el proceso de background
      printf("%s\n", jobs_list[0].command_line);
      while (jobs_list[0].pid != 0) {
        pause();  // hace que el padre espere mientras no acabe el proceso
      }
    } else {
      fprintf(stderr, ANSI_COLOR_RED"%s", args[1]);
      imprimir_error(": no existe ese trabajo");
    }
  }
  return 1;
}

/*
* Hace que el proceso de la lista indicado continúe su ejecución en background
*/
int internal_bg(char **args) {
  int nargs, pos;
  nargs = contar_elementos(args);
  if (nargs != 2) imprimir_error("Error de sintaxis. Uso: <bg NºTRABAJO>");
  else {
    pos = atoi(args[1]);
    if (pos > 0 && pos <= n_pids) {
      if (jobs_list[pos].status == 'E') { // si ya está ejecutándose en background
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
        kill(jobs_list[pos].pid, SIGCONT);  // hace que el proceso continúe
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

/*
* Detiene o suspende el proceso que se encuentra en foreground y lo envía con
* estado 'D' a background
*/
void ctrlz(int signum) {
  signal(SIGTSTP, ctrlz); // señal que detecta cuando se pulsa ctrl+z
  if (jobs_list[0].pid > 0) { // si hay algún proceso en foreground
    if (jobs_list[0].pid != pidshell) { // si no se trata del minishell
      printf("\n[ctrlz()→ Señal 20 (SIGTSTP) enviada a %d (%s) por %d]\n",
              jobs_list[0].pid, jobs_list[0].command_line, pidshell);
      kill(jobs_list[0].pid, SIGTSTP); // detiene o suspende el proceso
      jobs_list[0].status = 'D';
      if (jobs_list_add(jobs_list[0].pid, jobs_list[0].status,
                        jobs_list[0].command_line) == 0) {  // si hay hueco, añade a la lista
        int pos;
        pos = jobs_list_find(jobs_list[0].pid); // posición en background
        jobs_list[0].pid = 0; // lo quitamos de foreground
        printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid,
                jobs_list[pos].status, jobs_list[pos].command_line);
      } else {
        fprintf(stderr, ANSI_COLOR_RED "(%s) no se ha podido pasar a "
        "background, número máximo de procesos en background alcanzado.\n",
        jobs_list[0].command_line);
        fprintf(stderr, ANSI_COLOR_RESET);
      }
    } else printf("\rSeñal SIGTSTP no enviada debido a que el proceso en "
                  "foreground es el shell\n");
  } else printf("\rSeñal SIGTSTP no enviada debido a que no hay "
          "proceso en foreground\n");
}
/*
* Comprobamos si el comando a ejecutar contiene '>' seguido de algo
*/
int is_output_redirection (char **args){
  int cont = 0;
  while (args[cont] != NULL){
    if (*args[cont] == '>' && args[cont+1] != NULL) return 1;
    cont++;
  }
  return 0;
}

/*
* Abrimos el fichero donde queremos redireccionar la salida del comando a
* ejecutar al fichero.
*/
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
    dup2(fdesc,1); // Copia el descriptor de un fichero, copiando la salida del primero en el segundo
    close(fdesc);
  }
}

/*
* Ejecutamos la línea leida del minishell
*/
int execute_line(char *line) {
  char *args[LINEA];
  parse_args(args, line); // troceamos la línea dentro de args
  if (check_internal(args) == 0) { // Si no se trata de un comando interno
    int is_bg;
    // COmprobar si se trata de un background(segundo plano)
    is_bg = is_background(args);
    pid_t pid;
    pid = fork(); // Creamos un proceso hijo del minishell
    if (pid == -1) {
      imprimir_error(NULL);
      exit(0);
    } else if (pid == 0) {  // HIJO
      signal(SIGCHLD, SIG_DFL); // reseteamos la señal
      if (is_bg == 1) {
        //en caso de backgroud proceso hiji ignora el señal SIGTSTP y SIGINT
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
      } else {
        signal(SIGINT, SIG_IGN); // ignoramos SIGINT
        signal(SIGTSTP, SIG_DFL); // reseteamos SIG_DFL
      }
      if (is_output_redirection(args)) { // Si se trata de una redirección
        redireccionar(args);
      }
      execvp(args[0], args); // ejecutamos el comando externo
      fprintf(stderr, ANSI_COLOR_RED"%s", args[0]);
      imprimir_error(": no se encontró la orden");
      exit(0);
    } else {  // PADRE
      pidshell = getpid();
      //printf("[execute_line()→ PID padre: %d]\n", pidshell);
      //printf("[execute_line()→ PID hijo: %d]\n", pid);
      if (is_bg == 0) {
        /* Si no se trata de un proceso en background lo añadimos como foreground
        y esperamos a que acabe*/
        jobs_list[0].pid = pid;
        jobs_list[0].status = 'E';
        strcpy(jobs_list[0].command_line, line);
        while (jobs_list[0].pid != 0) {
          pause();
        }
      } else { // proceso en backgroud
        if (jobs_list_add(pid, 'E', line) != 0) { // Si no cabe en la lista
          fprintf(stderr, ANSI_COLOR_RED "(%s) no se ha podido ejecutar en background, "
          "número máximo de procesos en background alcanzado.\n", line);
          fprintf(stderr, ANSI_COLOR_RESET);
          exit(0);
        } else {
          printf(ANSI_COLOR_YELLOW"[%d] %d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid,
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

/*
* Leemos la línea introducida en el minishell
*/
char *read_line(char *line) {
    char *ptr_line;
    sleep(0); // PROVISIONAL
    #ifdef USE_READLINE // Si esta declarada utilizamos la libreria readline
      ptr_line = readline(ANSI_COLOR_GREEN"minishell" ANSI_COLOR_RESET PROMPT);
      if (!ptr_line) {
        exit(0);
      }
      strcpy(line, ptr_line);

      if (ptr_line && *ptr_line) {
        add_history(ptr_line);
      }
      return ptr_line;
    #else // S no esta declarada, leemos la línea con fgets
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
    #endif
}

/*
* Asocia las señales a sus correspondientes funcionones, además de leer y
* ejecutar los comandos introducidos en el minishell.
*/
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
