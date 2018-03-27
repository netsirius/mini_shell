#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#define PROMPT "$ "
#define LINEA 2048
#define N_JOBS 10
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
//#define USE_READLINE

#ifdef USE_READLINE
  #include <readline/readline.h>
  #include <readline/history.h>
#endif

/**********************
****** CABECERAS ******
***********************/

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
void reaper(int signum);
void ctrlc(int signum);
int is_background (char **args);
int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int jobs_list_remove(pid_t pid);
int internal_fg(char **args);
int internal_bg(char **args);
void ctrlz(int signum);
int is_output_redirection (char **args);

struct info_process {
    pid_t pid;
    char status; // ’E’ o ‘D’
    char command_line[LINEA]; // Comando
};
static struct info_process jobs_list[N_JOBS];
static int n_pids;
static int pidshell;
