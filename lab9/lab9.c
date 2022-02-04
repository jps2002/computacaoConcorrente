/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente em 21.2 mistrado pela Profª Silvanna Rossetto.
   Autor: João Pedro Silveira Gonçalves
   Código: Resolução do Laboratório 9 
*/

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 5

// Variáveis Globais
    // Relacionadas a sincronização entre threads:
sem_t penultima, posT5, ultima; //semaforos para sincronizar a ordem de execucao das threads

//funcao executada pela thread 1
void *t1 (void *threadid) 
{
    int *tid = (int*) threadid;

    //printf("Thread : %d esta executando...\n", *tid);

    sem_wait(&ultima); 
    printf("Volte sempre!\n");

    //printf("Thread : %d terminou!\n", *tid);

    pthread_exit(NULL);
}

//funcao executada pela thread 2
void *t2 (void *threadid) 
{
    int *tid = (int*) threadid;
    int valor_semaforo;
    
    //printf("Thread : %d esta executando...\n", *tid);

    sem_wait(&posT5); //espera T5 executar
    printf("Fique a vontade.\n");
    sem_post(&penultima);
    sem_getvalue(&penultima, &valor_semaforo);
    if (valor_semaforo == 3) // se as as threads 2, 3, 4 já executaram
    {
        sem_post(&ultima); //permite que T1 execute
    }

    //printf("Thread : %d terminou!\n", *tid);

    pthread_exit(NULL);
}

//funcao executada pela thread 3
void *t3 (void *threadid) 
{
    int *tid = (int*) threadid;
    int valor_semaforo;

    //printf("Thread : %d esta executando...\n", *tid);

    sem_wait(&posT5); //espera T5 executar
    printf("Sente-se por favor.\n");
    sem_post(&penultima);
    sem_getvalue(&penultima, &valor_semaforo);
    if (valor_semaforo == 3) // se as as threads 2, 3, 4 já executaram
    {
        sem_post(&ultima); //permite que T1 execute
    }

    //printf("Thread : %d terminou!\n", *tid);

    pthread_exit(NULL);
}

//funcao executada pela thread 4
void *t4 (void *threadid) 
{
    int *tid = (int*) threadid;
    int valor_semaforo;

    //printf("Thread : %d esta executando...\n", *tid);

    sem_wait(&posT5); //espera T5 executar
    printf("Aceita um copo d'água?\n");
    sem_post(&penultima);
    sem_getvalue(&penultima, &valor_semaforo);
    if (valor_semaforo == 3) // se as as threads 2, 3, 4 já executaram
    {
        sem_post(&ultima); //permite que T1 execute
    }

    //printf("Thread : %d terminou!\n", *tid);

    pthread_exit(NULL);
}

//funcao executada pela thread 5
void *t5 (void *threadid) 
{
    int *tid = (int*) threadid;

    //printf("Thread : %d esta executando...\n", *tid);

    printf("Seja bem-vindo!\n");
    for (size_t i = 0; i < 3; i++) 
        sem_post(&posT5); // permite que outras threads sem ser T1 executem 

    //printf("Thread : %d terminou!\n", *tid);
    
    pthread_exit(NULL);
}


//funcao principal
int main(int argc, char *argv[]) 
{
    pthread_t thread_ids[NUM_THREADS]; // representa os ids das threads no sistema
    int *id[5]; // representa os ponteiros para os ids locais das threads no sistema(argumento das threads)


    // Inicializar argumentos para as threads
    for (int thread = 0; thread < NUM_THREADS; thread++) 
    {
        if (!(id[thread] = malloc(sizeof(int)))) 
        {
            pthread_exit(NULL); return 1;
        }
        *id[thread] = thread+1;
    }

    // Inicializar os semáforos para sincronização de threads
    sem_init(&posT5, 0, 0);
    sem_init(&ultima, 0, 0);
    sem_init(&penultima, 0, 0);

    // Criar as threads  as tres threads
    if (pthread_create(&thread_ids[0], NULL, t1, (void *)id[0])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    if (pthread_create(&thread_ids[1], NULL, t2, (void *)id[1])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    if (pthread_create(&thread_ids[2], NULL, t3, (void *)id[2])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    if (pthread_create(&thread_ids[3], NULL, t4, (void *)id[3])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    if (pthread_create(&thread_ids[4], NULL, t5, (void *)id[4])) { printf("--ERRO: pthread_create()\n"); exit(-1); }


    // Esperar as threads terminarem
    for (int thread = 0; thread < NUM_THREADS; thread++) 
    {
        if (pthread_join(thread_ids[thread], NULL)) 
        {
            printf("Error: pthread_join()\n"); 
            exit(-1); 
        } 

        free(id[thread]);
    } 

    pthread_exit(NULL);
}