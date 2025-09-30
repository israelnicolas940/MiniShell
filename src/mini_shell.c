// Guilherme Gomes Botelho - 539008
// Israel Nícolas de Souza Mendes - 537604
// Denis da Silva Victor - 539198

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_CMD_LEN 256
#define MAX_ARGS 32
#define BG_PROCESSES_LEN 10

pid_t null_pid = -10;
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
  int args_count = 0; // Quantidade de argumentos da palavra
  int *is_background = background; // String auxiliar
  *is_background = 0; // Binário que determina se é foreground ou background

  // Esse módulo inteiro serve para dividir a string em palavras e inserí-las no vetor args[]
  char *token = strtok(input, " \t");
  while (token != NULL && args_count < MAX_ARGS - 1) {
    args[args_count++] = token;
    token = strtok(NULL, " \t");
  }
  // Colocando um NULL na última posição para marcar o fim do vetor
  args[args_count] = NULL;

 // Esse módulo serve somente para determinar se o
 // Se sim, muda o binário de estado 
 // E tira o último elemento
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

 // Enumeração dos comandos internos para indexação
enum INTERNAL_COMMAND { EXIT, PID, JOBS, WAIT, INTERNAL_COMMAND_COUNT };

// Estrutura para armazenar nome dos comandos
typedef struct {
  char name[50];
} Command;

// Array constante com os nomes dos comandos internos
static Command const internal_commands[INTERNAL_COMMAND_COUNT] = {
    [EXIT] = {"exit"}, [PID] = {"pid"}, [JOBS] = {"jobs"}, [WAIT] = {"wait"}};

// Ponteiro de função para handlers de comandos internos
typedef void (*InternalCommandHandlerFn)(char **args);

// Declaração das funções handler
void handle_exit(char **);
void handle_pid(char **);
void handle_jobs(char **);
void handle_wait(char **);

// Array de ponteiros para funções handler (dispatch table)
InternalCommandHandlerFn internal_cmd_handler_arr[INTERNAL_COMMAND_COUNT] = {
    [EXIT] = handle_exit,
    [PID] = handle_pid,
    [JOBS] = handle_jobs,
    [WAIT] = handle_wait};

// Macro para iterar sobre comandos internos
#define for_each_internal_command(cmd_iter, internal_cmd_idx)                  \
  for (internal_cmd_idx = 0, cmd_iter = internal_commands[0];                  \
       internal_cmd_idx < INTERNAL_COMMAND_COUNT;                              \
       internal_cmd_idx++,                                                     \
      cmd_iter = (internal_cmd_idx < INTERNAL_COMMAND_COUNT)                   \
                     ? internal_commands[internal_cmd_idx]                     \
                     : internal_commands[0])

// Verifica se o comando é interno comparando com a lista
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

// Finaliza processos em background e encerra o shell
void handle_exit(char **args) {
  clean_finished_processes();
  printf("minishell encerrado\n");
  exit(0);
}

// Exibe PID do shell e do último processo filho criado
void handle_pid(char **args) {
  printf("Pid do shell: %d\n", getpid());
  printf("Pid do último processo filho %d\n", last_child_pid);
}

// Lista todos os processos em background ativos
void handle_jobs(char **args) {
  if (bg_count == 0) {
    printf("Nenhum processo em background\n");
    return;
  }

  int bg_iter = 0;
  printf("Processo(s) em background:\n");
  for (; bg_iter < BG_PROCESSES_LEN; bg_iter++) {
    if (bg_processes[bg_iter] != null_pid) {
      printf("[%d] %d Executando\n", bg_iter + 1, bg_processes[bg_iter]);
    }
  }
}

// Aguarda finalização de todos os processos em background
void handle_wait(char **args) {
  if (bg_count == 0) {
    printf("Nenhum processo em background\n");
    return;
  }
  int status;
  pid_t pid;

  // Loop bloqueante até todos os processos finalizarem
  while (bg_count > 0) {
    pid = wait(&status);

    if (pid == -1) {
      perror("Erro no wait");
      break;
    }

    // Localiza o PID no array de processos em background
    int i;
    for (i = 0; i < BG_PROCESSES_LEN && pid != bg_processes[i]; i++)
      ;
    if (i < BG_PROCESSES_LEN) {
      printf("[%d]+ Finalizou (PID: %d)\n", i + 1, pid);

      bg_processes[i] = null_pid;
      bg_count--;
    }
  }
  printf("Todos os processos terminaram\n");
}

// Despacha comando para o handler apropriado usando a dispatch table
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

 // Esta função serve para, dado um ID de um processo, adicioná-lo 
 // ao vetor de processos rodando no background
void add_bg_process(pid_t pid) {
  if (bg_count < 10) {
    int i = 0;
    for (; i < BG_PROCESSES_LEN && bg_processes[i] != null_pid; i++)
      ;
    bg_processes[i] = pid;
    bg_count++;

    printf("[%d] %d\n", i + 1, pid);
  } else {
    printf("Lista de processos em background cheia! Por favor, aguarde \n");
  }
}

// Função para executar comandos externos do mini-shell
void execute_command(char **args, int background) {
  pid_t pid = fork(); // Divide o processo pai em um processo pai e um processo filho
  if (pid < 0) {
    perror("Erro no fork");
    exit(1);
  } else if (pid == 0) { // processo filho
    // Aqui a função execvp substitui o processo filho por um programa executável que utiliza como parâmetros 
    // as informações passadas no input. Caso ocorra algum problema com a identificação do executável, 
    // o execvp retorna -1 e dá erro no mini-shell
    if (execvp(args[0], args) == -1) {
      perror("Erro no execvp");
      exit(1);
    }
  } else { // processo pai
    // Salva o PID do último processo filho
    last_child_pid = pid;
    // Adiciona o novo filho ao vetor de processos rodando no background
    if (background) {
      add_bg_process(pid);
    } else {
      wait(NULL);
    }
  }
}

// Essa função serve para limpar quaisquer processos que estavam rodando
// em background, mas que, agora, já foram finalizados
void clean_finished_processes(void) {
  int status;
  pid_t pid;
  // WNOHANG: não bloqueia se nenhum processo terminou
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    for (int i = 0; i < BG_PROCESSES_LEN; i++) {
      if (bg_processes[i] == pid) {
        printf("[%d]+ Finalizou\n", i + 1);
        bg_processes[i] = null_pid;
        bg_count--;
        break;
      }
    }
  }
}

// Essa função serve somente para incializar o vetor de processo rodando em background
void init_bg_processes(void) {
  for (int i = 0; i < BG_PROCESSES_LEN; i++) {
    bg_processes[i] = null_pid;
  }
}

int main(void) {
  char input[MAX_CMD_LEN];
  char *args[MAX_ARGS];
  int background;
  init_bg_processes();
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
