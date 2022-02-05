/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Trabalho de Implementação 1 de Computação Concorrente - 21.2 - Profª Silvanna Rossetto.
   Autor: João Pedro Silveira Gonçalves
   Código: Um exemplo simples de servidor de chat aplicando networking com multithread onde se recebe mensagens de um cliente em uma thread
   e as encaminhamos aos demais clientes ativos e conectados no servidor. 
   Ouvimos cada cliente em uma thread diferente a fim de aumentar a quantidade de clientes que podem utilizar  simultaneamente o serviço de chat.
*/


/*
    USO: 
    --> CHAMADA: Digite ./server <número de threads> no seu terminal
    --> FINALIZAR EXECUÇÃO: Pressione CTRL + C para suspender execução
    OBS: o número de threads corresponde ao número máximo de conexões simultâneas que nosso servidor consegue atender simultaneamente
         Se o número de conexões for maior do que esse limite, a conexão fica na fila para entrar na sala de chat.
         O servidor envia uma mensagem "you have entered the chat room" quando uma thread atende a uma conexão e permite que ela envie mensagem na sala de chat
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "data.h"
#include <time.h>
#include "timer.h"

// Constantes Globais

#define PORT "3490" // representa a porta em que os usuários devem se conectar
#define BACKLOG 10  // representa  quantas conexões a fila de conexões vai suportar
#define SOCKETERROR (-1)
#define MAX_BUFFER_SIZE 4096 // representa um tamanho máximo de uma mensagem lida ou enviada
#define END_COMUNICATION_CODE "end"
#define SEND_ERROR_CODE -1

// Variáveis Globais
    // Relacionadas ao multithreading
int NUM_THREADS; // indica o número de threads a serem criadas e consequentemente o número máximo de conexões simultâneas ativas no servidor
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t condition = PTHREAD_COND_INITIALIZER; // representa a sincronização condicional que sinaliza às threads a inserção de uma nova tarefa na fila de tarefas 

    // Variáveis globais compartilhadas entre threads
linkedList* client_sockets_list; // lista de file descriptors de soquetes ativos em que há conexão com cliente, funcionando como uma lista de clientes ativos

// Protótipo de Funções

    // Funções relacionadas à configuração de redes
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int get_socket(void);

    // Funções relacionadas ao domínio(chat) e multithreading 
        // Tarefa passada para threads: responsável pelo acesso à fila de tarefas
void* thread_task(void* arg);
        // Funções chamadas pelas threads e que implementam funcionalidades do domínio(serviço de chat em grupo)
void* handle_connection(void* client_socket_p);
void* handle_connection2(void* client_socket_p);
int send_message(int sockfd, char* message);

// Função Main
int main(int argc, char* argv[])
{

    // Ler entrada o usuário e obter número de threads desejado
    if(argc != 2) 
    {
        printf("Usage error: ./server <numeroDeConexoesSimultaneasPermitidas>\n");
        return 1;
    }
    NUM_THREADS = atoi(argv[1]);

    pthread_t thread_pool[NUM_THREADS]; // representa o id no sistema das threads criadas para ler mensagens dos clientes
    double time_start = 0.0;
    double time_done = 0.0;
    
    // Ouvir em server_socket por novas conexões e lidar com essas conexões nos soquetes client_socket
    int server_socket; // representa o handler(file descriptor) do soquete em que o servidor vai esperar conexões
    int client_socket; // representa o handler(file descriptor) do soquete em que haverá a conexão cliente-servidor
    char printable_ip[INET6_ADDRSTRLEN]; // representa uma versão string do ip do cliente
    struct sockaddr_storage client_adrress; // guarda as informações de endereços do cliente

    int *thread_ids[NUM_THREADS]; // representa os ponteiros para os ids locais das threads no sistema(argumento das threads)

    // Inicializar argumentos para as threads
    for (int thread = 0; thread < NUM_THREADS; thread++) 
    {
        if (!(thread_ids[thread] = malloc(sizeof(int)))) 
        {
            pthread_exit(NULL); 
            return 1;
        }
        *thread_ids[thread] = thread+1;
    }

    // Criar lista para guardar soquetes ativos de cliente
    client_sockets_list = createList();

    // Criar threads para lidar com conexões de clientes
    int i = 0;
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&thread_pool[i], NULL, thread_task, (void*) thread_ids[i]);
    }
    printf("Threads for communication with clients created\n");

    // Obter socket para servidor ouvir e vinculá-lo à porta.
    
    server_socket = get_socket();
    
    // Ouvir no servidor por conexões(BACKLOG conexões no máximo)

    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("Error: listen()");
        exit(1);
    }
    
    // Lidar com pedidos de conexão
    while (1) 
    { 
        // esperar por conexão
        printf("server: waiting for connections...\n");

        // aceitar conexão
        socklen_t sin_size = sizeof (client_adrress);
        client_socket = accept(server_socket, (struct sockaddr *)&client_adrress, &sin_size);
        if (  client_socket == -1)
        {
            perror("server: accept()");
            continue;
        }

        // imprimir informações da conexão
        inet_ntop(client_adrress.ss_family, get_in_addr((struct sockaddr *)&client_adrress), printable_ip, sizeof printable_ip);
        printf("server: got connection from %s\n", printable_ip);

        // lidar com a conexão: adicionar so à fila de tarefas para que alguma thread lide com a comunicação com clientes
        pthread_mutex_lock(&lock);

        insertList(client_socket, client_sockets_list); // adicionar soquete em que há a conexão com cliente à lista de soquetes ativos de clientes
        enqueue(&client_socket); // adicionar a fila de tarefas

        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&lock);

    }

    // Desalocar memória utilizada
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&condition);
    return 0;
}   


// Definição de Funções

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// Retorna uma estrutura sockaddr de IP4 ou IP6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Retorna um soquete para o servidor
int get_socket(void)
{
    int server_socket = 0;
    struct addrinfo* server_adress;
    struct addrinfo hints, *p;
    int yes = 1;
    int rv;

    // 1º - Obter structs de endereço do servidor utilizando getaddrinfo()

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use o IP do host, da máquina em que o servidor roda

    if ((rv = getaddrinfo(NULL, PORT, &hints, &server_adress)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // 2º - Criar um soquete para leitura pelo servidor e vinculá-lo à porta desejada
    for (p = server_adress; p != NULL; p = p->ai_next)
    {
        // Obter file descriptor de um soquete 
        if ((server_socket = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        // Forçar uso do soquete
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        // Vincular soquete obtido à porta desejada
        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(server_socket);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(server_adress); 

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    return server_socket;  
}

/**
 * @brief Tarefa das threads: espera por uma tarefa estar disponível e chama as funções necessárias para executá-la.
 * Uma tarefa disponível é um novo soquete de cliente que precisa ter sua conexão atendida.
 * 
 * @param arg id da thread
 * @return void* 
 */
void* thread_task(void* arg)
{
    int thread_id = *(int*) arg;

    while (1)
    {
        int* client_socket;

        // retirar tarefa da fila compartilhada de tarefas(seção crítica)
        pthread_mutex_lock(&lock); 

        if((client_socket = dequeue())  == NULL)
        {
            pthread_cond_wait(&condition, &lock); // esperar por uma nova tarefa ser adicionada na fila
            client_socket = dequeue(); // ler tarefa da fila
        }
        pthread_mutex_unlock(&lock);

        // Se a fila de tarefas não estiver vazia: lidar com a conexão nela.
        if (client_socket != NULL)
        {
            printf("Connection with client socket %d will be handled by thread %d\n", *client_socket, thread_id);
            // handle_connection(client_socket);
            handle_connection2(client_socket);
        }
    }
}

/**
 * @brief Função Inicial do Servidor: Receber de clientes pedidos de envio de arquivos e enviá-los.
 * 
 * Nessa versão não é utilizada, mas em futuras versões pode ser modificada e servir de inspiração para envio de mensagens como arquivos jsons. 
 * 
 * @param client_socket_p é o soquete onde se deve ouvir por pedidos de clientes
 * @return void* 
 * 
 * Inspirada nos exemplos do Professor Jacob Sorber
 */
void* handle_connection(void* client_socket_p)
{ 
    int client_socket = *(int*)client_socket_p;
    char buffer[MAX_BUFFER_SIZE];
    size_t bytes_read;
    char actual_path [MAX_BUFFER_SIZE +1];
    printf("Initializing interaction with client socket %d.\n", client_socket);
    fflush(stdout);

    // clean buffer before using 
    //printf("BUFFER BEFORE: %s\n", buffer);
    memset(buffer, 0, MAX_BUFFER_SIZE);
    memset(actual_path, 0, MAX_BUFFER_SIZE);
    printf("BUFFER BEFORE USE: %s\n", buffer);

    // read the client´s message: the name of the file to read
    bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read == -1)
    {
        perror("server: read()\n");
    }
    // print request information
    printf("Bytes read: %ld\n", bytes_read);
    buffer[strlen(buffer)] = '\0';
    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    // validate path requested
    if (realpath(buffer, actual_path) == NULL)
    {
        printf("server: %s is bad path\n", buffer);
        close(client_socket);
        return NULL;
    }

    // if path is valid, open requested file, read content into buffer and send to client
    FILE *fp = fopen(actual_path, "r");
    if (fp == NULL)
    {
        printf("server: fopen() %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    memset(actual_path, 0, MAX_BUFFER_SIZE);
    while ((bytes_read = fread(buffer, sizeof(char), MAX_BUFFER_SIZE, fp)) > 0)
    {
        printf("sending %ld bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    // after sending, close read file and end connection
    close(client_socket);
    fclose(fp);
    printf("Closing connection\n"); 

    return NULL;
}

/**
 * @brief Ouve no soquete passado por mensagens do cliente e as reenviam para outros clientes ativos presentes na lista compartilhada de clientes ativos
 * 
 * @param client_socket_p é o soquete de cliente onde se deve escutar
 * @return void* NULL 
 */
void* handle_connection2(void* client_socket_p)
{
    int client_socket = *(int*)client_socket_p; // file descriptor do soquete em que há conexão com cliente
    char buffer[MAX_BUFFER_SIZE]; // buffer para mensagens recebidas
    size_t bytes_read; // número de bytes recebido

    // Limpar buffer antes de uso
    memset(buffer, 0, MAX_BUFFER_SIZE);

    char* freeToGoMessage = "server: you have entered the chat room.";
    send_message(client_socket, freeToGoMessage);

    printf("Initializing interaction with client socket %d.\n", client_socket);
    fflush(stdout);

    // Ouvir por mensagens do cliente recebidas no soquete
    while (1)
    {
        // Limpar buffer antes de uso
        memset(buffer, 0, MAX_BUFFER_SIZE);

        double time_start = 0.0, time_done = 0.0; // para testar tempo de execução
        double time_beforeReading = 0.0, time_afterFowarding = 0.0;
        double time_fromReadingToFowarding = 0.0; double time_buffer;

        time_start = (double) clock () / CLOCKS_PER_SEC;
        time_beforeReading = (double) clock () / CLOCKS_PER_SEC;

        // Ler mensagem do enviada pelo cliente
        bytes_read = read(client_socket, buffer, sizeof(buffer));

        time_done = (double) clock () / CLOCKS_PER_SEC;

        // Caso houve erros de leitura ou desconexão: retornar
        if (bytes_read == -1) // erro de leitura
        {
            perror("server: read()\n");
            break;
        }
        else if(bytes_read == 0) // erro de conexão
        {
            printf("server: client on socket %d has disconnected.\n", client_socket);
            
            // remover cliente da lista de conexões ativas
            pthread_mutex_lock(&lock);
            removeList(client_socket, client_sockets_list);
            close(client_socket);
            pthread_mutex_unlock(&lock);

            break;
        }

        // Caso tudo certo: imprimir a mensagem recebida
        buffer[strlen(buffer)] = '\0';
        printf("RECIEVED NEW MESSAGE:\n");
        printf("-->Time took to read: %lf \n", time_done-time_start);
        printf("-->Sender: client socket %d\n", client_socket);
        printf("-->Size: %ld bytes\n", bytes_read);
        printf("-->Content: \"%s\"\n", buffer);

        
        // Se é uma mensagem de fim de conexão("end"): finalizar conexão
        if (strcmp(END_COMUNICATION_CODE, buffer) == 0)
        {
            printf("Closing connection with client socket %d\n", client_socket); 

            // remover da lista de clientes ativos
            pthread_mutex_lock(&lock);
            removeList(client_socket, client_sockets_list);
            close(client_socket);
            pthread_mutex_unlock(&lock);
            break;
        }

        // Se há conexões ativas: reenviar a mensagem recebida para elas.
        pthread_mutex_lock(&lock);
        nodeFromList* current_node = client_sockets_list->head;
        if (current_node)
        {
            // Limpar buffer antes de usar
            char message_to_be_sent[MAX_BUFFER_SIZE]; // mensagem a ser enviada aos clientes 
            memset(message_to_be_sent, 0, MAX_BUFFER_SIZE);

            printf("FOWARDING MESSAGE\n");
            printf("-->Original Message Sender: client socket %d\n", client_socket);
            printf("-->Fowarding to: \n");

            time_start = (double) clock () / CLOCKS_PER_SEC;
            
            // Percorrer a lista de conexões ativas
            while(current_node)
            {
                int client = current_node->client_socket;

                if (client != client_socket)
                {
                   send_message(client, buffer);
                   printf("---->client socket %d\n", client);
                }
                
                current_node = current_node->next_node;
            } 

            time_done = (double) clock () / CLOCKS_PER_SEC;
            time_afterFowarding = (double) clock () / CLOCKS_PER_SEC;
            printf("--> Time took to foward message to all active clients:%lf\n", time_done- time_start);
            printf("--> Time took from recieving message to fowarding it to all active clients:%lf\n", time_afterFowarding - time_beforeReading);
        }    
        pthread_mutex_unlock(&lock);


    }
    return NULL;
}

/**
 * @brief Envia message para soquete sockfd.
 * 
 * @param sockfd soquete para o qual a mensagem deve ser enviada 
 * @param message a mensagem a ser enviada
 * @return error code, se falhar; ou número de bytes enviados, se enviado com sucesso.
 */
int send_message(int sockfd, char* message)
{
    int number_bytes = strlen(message); // representa o número de bytes a serem enviados
    // printf("\"%s\" containing %d bytes will be sent to client socket %d.\n", message, number_bytes, sockfd); // log do envio
    if (write(sockfd, message , number_bytes) != number_bytes) 
    {
        perror("error(client): send()");
        return SEND_ERROR_CODE;
    }

    return number_bytes;
}
