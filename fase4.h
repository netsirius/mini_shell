#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#define PROMPT "miniShell$ "
#define LINEA 2048
#define N_JOBS 10
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

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

struct info_process {
    pid_t pid;
    char status; // ’E’ o ‘D’
    char command_line[LINEA]; // Comando
};
static struct info_process jobs_list[N_JOBS];
