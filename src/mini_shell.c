// Guilherme Gomes Botelho - 539008
// Coloquem o nome e a matrículo de vocês

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_CMD_LEN 256
#define MAX_ARGS 32

// Array para armazenar PIDs de processos em background
pid_t bg_processes[10];
int bg_count = 0;
pid_t last_child_pid = 0; // Armazena PID do último processo filho

enum INTERNAL_COMMAND { EXIT, PID, JOBS, WAIT, INTERNAL_COMMAND_COUNT };

typedef struct 
{
  char name[50];
} command;

static command const internal_commands[INTERNAL_COMMAND_COUNT] = 
{
    [EXIT] = {"exit"}, [PID] = {"pid"}, [JOBS] = {"jobs"}, [WAIT] = {"wait"}
};

//#define for_each_internal_command(cmd)                                         \
  for (cmd = &init_task; (cmd = next_task(cmd)) != &init_task;)

//#define for_each_internal_command(cmd, internal_cmd_idx)                       \
  for (cmd = internal_commands[0]; i < INTERNAL_COMMAND_COUNT;                 \
       ++i, cmd = internal_commands[i])

//typedef void *(CommandHandler)(command cmd);

//void loop_internal_commands(CommandHandler handle) {}
// ou
// generico que passa por cada um dos elementos de internal_commands aceita como
// entrada command e o retorno é um argumento com tipo void*, dependendo da
// funcao o casting é feito.
// int get_internal_command_id(char *cmd) {
//   for (command cmd = internal_commands[])
// }

void add_bg_process(pid_t pid)  // Esta função serve para, dado um ID de um processo, adicioná-lo 
{                               // ao vetor de processos rodando no background
    if (bg_count < 10) 
    {
        bg_processes[bg_count++] = pid;
    }
}

void clean_finished_processes() // Essa função serve para limpar quaisquer processos que estavam rodando
{                               // em background, mas que, agora, já foram finalizados
    int status;
    pid_t pid;
    // WNOHANG = não bloqueia se nenhum processo terminou
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)  // Esse waitpid atua como um "Listener", o parâmetro -1
    {                                                  // informa que ele está ouvindo todos os chamados filhos
        // Remove da lista de background               // e esperando algum deles encerrar. Quando isso ocorre,
        for (int i = 0; i < bg_count; i++)             // essa função percorre o vetor de processo rodando em
        {                                              // background, acha o processo com o PID que foi encerrado,
            if (bg_processes[i] == pid)                // avisa o usuário que ele finalizou, remove-o do vetor e,
            {                                          // por fim, reorganiza o vetor. 
                printf("[%d]+ Done\n", i+1);           // Ainda, devido ao parâmetro WNOHANG, caso nenhum processo
                // Remove elemento da lista            // filho tenha terminado ainda, ele não bloqueia o processo
                for (int j = i; j < bg_count - 1; j++) // o processo pai, o waitpid apenas retorna 0
                {
                    bg_processes[j] = bg_processes[j+1];
                }
                bg_count--;
                break;
            }
        }
    }
}

// void (*functions)(char **args)[] = {};
void parse_command(char *input, char **args, int *background) 
{
    // TODO: Implementar parsing do comando
    // Dividir a string em argumentos
    // Verificar se termina com &

    int i = 0; // Quantidade de argumentos da palavra
    char *token; // String auxiliar
    *background = 0; // Binário que determina se é foreground ou background

    token = strtok(input, " \t"); // Esse módulo inteiro serve para dividir a string em 
    while(token != NULL && i < MAX_ARGS - 1) // Em palavras e inserí-las no vetor args[]
    {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;     // Colocando um NULL na última posição para marcar o fim do vetor                                     

    if(i > 0 && strcmp(args[i - 1], "&") == 0) // Esse módulo serve somente para determinar se o
    {                                 // Comando é foreground ou background
        *background = 1;              // Se sim, muda o binário de estado 
        args[i - 1] = NULL;           // E tira o último elemento
    }
}

void execute_command(char **args, int background) // Função para executar comandos externos do mini-shell
{
    pid_t pid = fork(); // Divide o processo pai em um processo pai e um processo filho
    if (pid == 0)
    {
        // Processo filho - executa normalmente
        if(execvp(args[0], args) == -1) // Aqui a função execvp substitui o processo filho por um programa
        {                               // executável que utiliza como parâmetros as informações passadas no input.
            printf("Erro no execvp");   // Caso ocorra algum problema com a identificação do executável, 
            exit(1);                    // o execvp retorna -1 e dá erro no mini-shell
        }
        
    } 
    else 
    {
        // Processo pai
        if (background) 
        {
            // Background: NÃO aguarda, armazena PID
            add_bg_process(pid); // Adiciona o novo filho ao vetor de processos rodando no background
            printf("[%d] %d\n", bg_count, pid); 
        } 
        else 
        {
            // Foreground: aguarda terminar
            wait(NULL); // função para fazer com que o pai aguarde a conclusão do processo filho
        }
    }
}

int is_internal_command(char **args) {
  if (args[0] != NULL) {
    int i = 0;
    for (command cmd = internal_commands[i]; i < INTERNAL_COMMAND_COUNT;
         ++i, cmd = internal_commands[i]) {
      if (strcmp(cmd.name, args[0]) == 0) {
        return 1;
      }
    }
  }

  return 0;
}

void handle_jobs() {}

void handle_pid() {}

void handle_wait() {}

void handle_internal_command(char **args) {
  // TODO: Executar comandos internos
  // exit, pid, jobs, wait
  // handle Ctrl-C
  char *cmd = args[0];
}

int main() 
{
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int background;
    printf("Mini-Shell iniciado (PID: %d)\n", getpid());
    printf("Digite 'exit' para sair\n\n");
    while (1) 
    {
        clean_finished_processes(); // Função para limpar processos terminados

        printf("minishell> ");
        fflush(stdout);
        // Ler entrada do usuário
        if (!fgets(input, sizeof(input), stdin)) 
        {
            break;
        }
        // Remover quebra de linha
        input[strcspn(input, "\n")] = 0;
        // Ignorar linhas vazias
        if (strlen(input) == 0) 
        {
            continue;
        }
        // Fazer parsing do comando
        parse_command(input, args, &background);
        //printf("%d", background);
        // Executar comando
        if (is_internal_command(args)) 
        {
            handle_internal_command(args);
            //printf("internal \n");
        }  
        else 
        {
            //printf("external \n");
            execute_command(args, background);
        }
    }
    printf("Shell encerrado!\n");
    return 0;
}
