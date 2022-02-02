/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Trabalho de Implementação 1 de Computação Concorrente - 21.2 - Profª Silvanna Rossetto.
   Autor: João Pedro Silveira Gonçalves
   Código: Um exemplo simples de cliente de uma aplicação de char utilizando networking com threading onde se pode enviar ou receber
   mensagens de/para outros clientes conectados ao mesmo servidor.
*/

// Usage: ./client {hostname}
// Exemplo de uso: se seu computador se chama teste123 use ./client teste123 para abrir a aplicação/terminal de chat

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SEND_ERROR_CODE -1
#define PORT "3490"     // porta com a qual se deve conectar no servidor
#define MAXDATASIZE 4090 // tamanho máximo de bytes a serem recebidos
#define MAX_BUFFER_SIZE 4000 // tamanho máximo de bytes a serem recebidos e enviados
#define MAX_USERNAME_SIZE 20 // tamanho máximo de caracteres em um nome de usuário
#define NUM_THREADS 2
#define END_CONECTION_CODE "end"

// Variáveis Globais
    // Relacionadas ao multithreading
pthread_t threadsId[NUM_THREADS]; // representa o id no sistema das threads criadas para ler mensagens dos clientes
    
    /* OBS: atualmente, nessa versão, apenas uma thread será utilizada( uma para ler mensagens enviadas pelo servidor) [
        apesar de haver espaço para criação de mais */

// Protótipo de funções

    // relacionadas à networking
void *get_in_addr(struct sockaddr *sa);
int setup_connection(char* hostname);
   // relacionadas a threading e problemas do domínio 
int send_message(int sockfd, char* message);
void send_hasEnteredChat_message(int sockfd, char* user_name);
void* thread_task_read(void* arg);

// Função main
// Exemplo de uso: se seu computador se chama teste123 use ./client teste123 para abrir a aplicação/terminal de chat
int main(int argc, char *argv[])
{
    int sockfd; // file descriptor do soquete onde a conexão com o servidor dado por hostname acontece
    char* hostname; // hostname
    char buffer[MAX_BUFFER_SIZE]; 
    char user_name[MAX_USERNAME_SIZE];
    char final_message[MAX_BUFFER_SIZE+MAX_USERNAME_SIZE+2];
    memset(user_name, 0, MAX_USERNAME_SIZE);

    // Verificar argumentos de entrada
    if (argc != 2)
    {
        fprintf(stderr, "error(client): correct usage is: client.c <hostname>\n");
        exit(1);
    }

    // Estabelecer conexão com servidor identificado por hostname
    // por exemplo: se seu computador se chama teste123 chame cliente como ./client teste123
    hostname = argv[1];  
    sockfd = setup_connection(hostname);
    
    // Ler nome de usuário 
    printf("Usage tip: if you want to close this chat just send \"end\" or press CTRL+C\n");
    printf("Now please, enter a nickname up to 19 characters: ");
    scanf("%s", user_name);
    user_name[strlen(user_name)] = 0; // retirar newline 
    
    // Criar thread para receber mensagens 
    pthread_create(&threadsId[0], NULL, thread_task_read, &sockfd);

    // Mandar mensagem de que entrou no chat a outros usuários
    send_hasEnteredChat_message(sockfd, user_name);
    
    // Receber mensagens do usuário e enviá-las 
    while(1)
    { 
        // limpar buffers antes de usar
        memset(buffer, 0, MAXDATASIZE);
        memset(final_message, 0, MAX_BUFFER_SIZE+MAX_USERNAME_SIZE+2);
        

        // ler da entrada do usuário o corpo desejado da mensagem para envio
        printf("Send message: ");
        fflush(stdout);
        fflush(stdin);
        while (strlen(buffer) == 0)
        {
            fgets( buffer, sizeof( buffer ), stdin); 
            buffer[ strcspn( buffer, "\n" ) ] = '\0'; // retirar newline do final   
        }
        
        // preparar a mensagem final para envio
        strcat(final_message, user_name); // adicionar marcador de usuário à mensagem final
        strcat(final_message, ": ");
        strcat(final_message, buffer); // adicionar corpo da mensagem à mensagem final
        
        // enviar a mensagem final
        send_message(sockfd, final_message);


        // verificar se foi uma mensagem de finalizar conexão
        if (strcmp(buffer, END_CONECTION_CODE) == 0)
        {
            printf("\nClosing client connection\n");
            close(sockfd);
            break;
        }
       
    }

    return 0;
}

// Functions definitions

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


/**
 * @brief Manda "user x has entered the chat message" para outros clientes na conversa.
 * 
 * @param sockfd 
 * @param user_name 
 */
void send_hasEnteredChat_message(int sockfd, char* user_name)
{
    if(strlen(user_name) != 0) 
    {
        char message_to_send[80];
        memset(message_to_send, 0, 80);
        strcat(message_to_send, user_name);
        strcat(message_to_send, ": I have entered the chat.");
        send_message(sockfd, message_to_send);
    }
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
    int number_bytes = strlen(message); // número de bytes na mensagem passada
    if (number_bytes == 0) return 0; // se a mensagem for vazia, fazer nada

    // log do envio para propósitos de teste
    printf("LOG --> \"%s\" containing %d bytes will be sent to client socket %d.\n", message, number_bytes, sockfd);

    // enviar mensagem no soquete sockfd
    if (write(sockfd, message , number_bytes) != number_bytes)
    {
        perror("error(client): send()");
        return SEND_ERROR_CODE;
    }

    return number_bytes;
}

/**
 * @brief Estabelece conexão com hostname na porta PORT
 * 
 * @param hostname 
 * @return file descriptor do soquete em que essa conexão se estabelecerá
 */
int setup_connection(char* hostname)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);

    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); 

    return sockfd;
}


void* thread_task_read(void* arg)
{
    int sockfd = *(int*) arg; // soquete onde há conexão com servidor
    size_t bytes_read = 0; // número de bytes lido
    char buffer[MAX_BUFFER_SIZE]; // buffer onde mensagens lidas serão salvas
    
    // printf("Listening on socket %d for messages\n", sockfd);
    fflush(stdout);

    // Ouvir por mensagens recebidas do servidor e imprimi-las
    while (1)
    { 
        // Limpar buffer antes de uso
        memset(buffer, 0, MAX_BUFFER_SIZE);

        // Ler a mensagem do servidor
        bytes_read = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        
        // Caso houve erros de leitura ou desconexão: retornar
        if (bytes_read == -1) // erros de leitura
        {
            perror("\nserver: read()\n");
            printf("Press ctrl-c to kill chat terminal\n");
            break;
        }
        if (bytes_read == 0) // erros de conexão
        {
            printf("\nClient: Server disconnected\n");
            printf("Press ctrl-c to kill chat terminal\n");
            break;
        }

        // Caso tudo certo: imprimir a mensagem recebida
        buffer[strlen(buffer)] = '\0';
        // printf("Recieved and read %ld bytes from socket %d\n", bytes_read, sockfd);
        printf("\nNEW  MESSAGE(%d) FROM %s\n", sockfd, buffer);
        fflush(stdout);         
    }

    pthread_exit(NULL);
}