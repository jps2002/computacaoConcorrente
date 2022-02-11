/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente em 21.2 mistrado pela Profª Silvanna Rossetto.
   Autor: João Pedro Silveira Gonçalves
   Código: Resolução do Laboratório 10

   Uso: ./lab <numero de threads consumidoras> <numero de threads produtoras>
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>


#define N 8 // tamanho do buffer compartilhado

// Variáveis globais
    // Relacionadas ao problema de domínio
int Buffer[N]; // área compartilhada entre threads
int count=0, in=0, out=0; // variaveis de estado relacionadas às atividades no domínio do problema

    // Relacionadas à sincronização entre threads
int num_consumidor = 0; // número de threads consumidoras a serem criadas
int num_produtor = 0; // número de threads produtoras a serem criadas
sem_t sem_produtor, sem_consumidor; // semáforos que permitem execuções das funções produtoras e consumidoras

// Protótipos de funções
void * produtor(void * arg);
void * consumidor(void * arg);
void IniciaBuffer(int n);
void ImprimeBuffer(int n);
void Insere (int id);
int Retira (int id);

// Função Main
int main(int argc, char* argv[])
{
    // Validação de entrada
    if (argc != 3)
    {
        printf("Error: ./%s <numero de threads consumidoras> <numero de threads produtoras>\n", argv[0]);
        return 1;    
    }
    num_consumidor = atoi(argv[1]);
    num_produtor = atoi(argv[2]);

    pthread_t threads_Id[num_consumidor + num_produtor]; // representa os ids no sistema das threads criadas
    int *id[num_consumidor + num_produtor]; // representa os ids locais das threads criadas

    // preencher vetor de ids locais 
    for (size_t i = 0; i < (num_consumidor + num_produtor); i++)
    {
        id[i] = malloc(sizeof(int));
        if (id[i] == NULL) 
            exit(-1);
        *id[i] = i+1;
    }

    // criar threads consumidoras
    for (size_t i = 0; i < num_consumidor; i++)
    {
        if (pthread_create(&threads_Id[i], NULL, consumidor, (void *) id[i])) 
            exit(-1);
    }

    // criar threads produtoras
    for (size_t i = 0; i < num_produtor; i++)
    {
        if (pthread_create(&threads_Id[i + num_consumidor], NULL, produtor, (void *) id[i + num_consumidor])) 
            exit(-1);
    }
    
    sem_post(&sem_produtor); // iniciar processo desbloqueando uma thread produtora

    pthread_exit(NULL);
    return 0;
}

// Definições das funções prototipadas


/**
 * @brief Inicializa o buffer global com valor 0 em suas n posições
 * 
 * @param n é o tamanho do buffer
 * @author Profª Silvana Rossetto
 */
void IniciaBuffer(int n) 
{
  for (int i =0; i < n; i++)
    Buffer[i] = 0;
}

/**
 * @brief Imprime o conteúdo de buffer global
 * 
 * @param n é o tamanho do buffer global
 * @author Profª Silvana Rossetto
 */
void ImprimeBuffer(int n) 
{
    printf("BUFFER: ");
    for (int i = 0; i < n; i++)
        printf("-> [%d] = %d ", i, Buffer[i]);
    printf("\n");
}

 
/**
 * @brief Preenche o buffer ou bloqueia a thread caso o Buffer não esteja vazio
 * 
 * @param id é o id da thread
 */
void Insere (int id) 
{
    int valor_semaforo;

    printf("P[%d] quer inserir\n", id);
    sem_wait(&sem_produtor);
    
    // preencher buffer
    for (size_t i = 0; i < N; i++)
    {
        Buffer[i] = i;
    }
    printf("P[%d] inseriu\n", id);

    // imprimir estado
    ImprimeBuffer(N);
    sem_getvalue(&sem_produtor, &valor_semaforo);
    printf("Valor de sem_produtor apos thread produtora %d: %d\n", id, valor_semaforo);

    sem_getvalue(&sem_consumidor, &valor_semaforo);
    printf("Valor de sem_consumidor apos thread produtora %d: %d\n", id, valor_semaforo + N);

    // desbloquear threads consumidoras
    for (size_t i = 0; i < N; i++)
    {
        sem_post(&sem_consumidor);
    }    
}

/**
 * @brief Retira um elemento no Buffer ou bloqueia a thread caso o Buffer esteja vazio
 * 
 * @param id é o id da thread
 * @return int é o valor retirado do buffer
 * 
 */
int Retira (int id) 
{
    int valor_semaforo;
    int item;

    printf("C[%d] quer consumir\n", id);
    sem_wait(&sem_consumidor);

    item = Buffer[out];
    Buffer[out] = 0;
    out = (out + 1) % N;

    printf("C[%d] consumiu %d\n", id, item);
    ImprimeBuffer(N);

    // se já retiramos o vetor todo: permitir que produtor execute
    if (out == 0)
    {
        printf("Thread consumidora %d desbloqueou uma thread produtora.\n", id);
        sem_getvalue(&sem_produtor, &valor_semaforo);
        printf("--> Valor de sem_produtor após thread consumidora %d atualiza-lo: %d\n", id, valor_semaforo + 1);
        sem_post(&sem_produtor); 
    }
    
    return item;
}

/**
 * @brief Função realizada pelas threads produtoras
 * 
 * @param arg é o id da thread
 * @return void* 
 * @author Profª Silvana Rossetto
 */
void * produtor(void * arg) 
{
    int *id = (int *) arg;
    printf("Iniciando a thread produtora %d\n", *id);
    while(1) 
    {
        //produzindo o item
        Insere(*id);
        sleep(1);
    } 
    free(arg);
    pthread_exit(NULL);
}

/**
 * @brief Função realizada pelas threads consumidoras
 * 
 * @param arg é o id da thread
 * @return void* 
 * 
 * @author Profª Silvana Rossetto
 */
void * consumidor(void * arg) 
{
    int *id = (int *) arg;
    int item;
    printf("Iniciando athread consumidora %d\n", *id);
    while(1) 
    {
        item = Retira(*id);
        sleep(1); //faz o processamento do item 
    } 
    free(arg);
    pthread_exit(NULL);
}
