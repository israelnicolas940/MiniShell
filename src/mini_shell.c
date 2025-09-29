// Guilherme Gomes Botelho - 539008
// Israel Nícolas de Souza Mendes - 537604
// Coloquem o nome e a matrículo de vocês

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_CMD_LEN 256
#define MAX_ARGS 32

// Array para armazenar PIDs de processos em background
pid_t bg_processes[10];
int bg_count = 0;
pid_t last_child_pid = 0; // Armazena PID do último processo filho

void clean_finished_processes();

/*
 * --------------------------------------------------------------------------------------------
 * Parser
 * --------------------------------------------------------------------------------------------
 **/

void parse_command(char *input, char **args, int *background) {
  int args_count = 0;
  int *is_background = background;
  *is_background = 0;

  // Divide a string em palavras e insere-as no vetor args[]
  char *token = strtok(input, " \t");
  while (token != NULL && args_count < MAX_ARGS - 1) {
    args[args_count++] = token;
    token = strtok(NULL, " \t");
  }
  // NULL marca o fim do vetor
  args[args_count] = NULL;

  // Determina se o comando é foreground ou background.
  if (args_count > 0 && strcmp(args[args_count - 1], "&") == 0) {
    *is_background = 1;
    args[args_count - 1] = NULL;
  }
}

/*
 * --------------------------------------------------------------------------------------------
 * Comandos Internos.
 * --------------------------------------------------------------------------------------------
 **/

enum INTERNAL_COMMAND { EXIT, PID, JOBS, WAIT, INTERNAL_COMMAND_COUNT };

typedef struct {
  char name[50];
} Command;

static Command const internal_commands[INTERNAL_COMMAND_COUNT] = {
    [EXIT] = {"exit"}, [PID] = {"pid"}, [JOBS] = {"jobs"}, [WAIT] = {"wait"}};

typedef void (*InternalCommandHandlerFn)(char **args);

void handle_exit(char **);
void handle_pid(char **);
void handle_jobs(char **);
void handle_wait(char **);

InternalCommandHandlerFn internal_cmd_handler_arr[INTERNAL_COMMAND_COUNT] = {
    [EXIT] = handle_exit,
    [PID] = handle_pid,
    [JOBS] = handle_jobs,
    [WAIT] = handle_wait};

#define for_each_internal_command(cmd_iter, internal_cmd_idx)                  \
  for (internal_cmd_idx = 0, cmd_iter = internal_commands[0];                  \
       internal_cmd_idx < INTERNAL_COMMAND_COUNT;                              \
       internal_cmd_idx++,                                                     \
      cmd_iter = (internal_cmd_idx < INTERNAL_COMMAND_COUNT)                   \
                     ? internal_commands[internal_cmd_idx]                     \
                     : internal_commands[0])

int is_internal_command(char **args) {
  if (args[0] != NULL) {
    Command cmd;
    int i = 0;
    for_each_internal_command(cmd, i) {
      if (strcmp(cmd.name, args[0]) == 0) {
        return 1;
      }
    }
  }

  return 0;
}

void handle_exit(char **args) {
  clean_finished_processes();
  printf("minishell encerrado\n");
  exit(0);
}

void handle_pid(char **args) {
  printf("Pid do shell: %d\n", getpid());
  printf("Pid do último processo filho %d\n", last_child_pid);
}

void handle_jobs(char **args) {
  if (bg_count == 0) {
    printf("Nenhum processo em background\n");
  }

  int bg_iter = 0;
  for (; bg_iter < bg_count; bg_iter++) {
    printf("[%d] %d Executando\n", bg_iter + 1, bg_processes[bg_iter]);
  }
}

void handle_wait(char **args) {
  if (bg_count == 0) {
    printf("Nenhum processo em background\n");
    return;
  }
  int status;
  pid_t pid;
  while (bg_count > 0) {
    pid = wait(&status);

    if (pid == -1) {
      perror("Erro no wait");
      break;
    }

    int i;
    for (i = 0; i < bg_count && pid != bg_processes[i]; i++)
      ;
    if (i < bg_count) {
      printf("[%d]+ Finalizou (PID: %d)\n", i + 1, pid);

      // Shift
      for (int j = i; j < bg_count - 1; j++) {
        bg_processes[j] = bg_processes[j + 1];
      }
      bg_count--;
    }
  }
}

void handle_internal_command(char **args) {
  Command cmd_iter;
  int i = 0;
  for_each_internal_command(cmd_iter, i) {
    if (strcmp(cmd_iter.name, args[0]) == 0) {
      internal_cmd_handler_arr[i](args);
      return;
    }
  }
}

/*
 * --------------------------------------------------------------------------------------------
 * Comandos externos
 * --------------------------------------------------------------------------------------------
 **/

void add_bg_process(pid_t pid) {
  if (bg_count < 10) {
    bg_processes[bg_count++] = pid;
  }
}

void execute_command(char **args, int background) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("Erro");
    exit(1);
  } else if (pid == 0) { // processo filho
    if (execvp(args[0], args) == -1) {
      perror("Erro no execvp");
      exit(1);
    }
    // last_child_pid = getpid();
  } else { // processo pai
    last_child_pid = pid;
    if (background) {
      add_bg_process(pid);
      printf("[%d] %d\n", bg_count, pid);
    } else {
      wait(NULL);
    }
  }
}

// Limpa processos em background que já finalizaram
void clean_finished_processes() {
  int status;
  pid_t pid;
  // WNOHANG: não bloqueia se nenhum processo terminou
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    for (int i = 0; i < bg_count; i++) {
      if (bg_processes[i] == pid) {
        printf("[%d]+ Done\n", i + 1);
        // Remove o processo da lista e reorganiza
        for (int j = i; j < bg_count - 1; j++) {
          bg_processes[j] = bg_processes[j + 1];
        }
        bg_count--;
        break;
      }
    }
  }
}

int main() {
  char input[MAX_CMD_LEN];
  char *args[MAX_ARGS];
  int background;
  printf("Mini-Shell iniciado (PID: %d)\n", getpid());
  printf("Digite 'exit' para sair\n\n");
  while (1) {
    clean_finished_processes(); // Função para limpar processos terminados

    printf("minishell> ");
    fflush(stdout);
    // Ler entrada do usuário
    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }
    // Remover quebra de linha
    input[strcspn(input, "\n")] = 0;
    // Ignorar linhas vazias
    if (strlen(input) == 0) {
      continue;
    }
    // Fazer parsing do comando
    parse_command(input, args, &background);
    //  Executar comando
    if (is_internal_command(args)) {
      handle_internal_command(args);
    } else {
      execute_command(args, background);
    }
  }
  printf("Shell encerrado!\n");
  return 0;
}
