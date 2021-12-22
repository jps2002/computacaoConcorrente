/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente (21.2) - Profª Silvana Rossetto
   Resolução do Laboratório 5 do Módulo 2
   
   Autor: João Pedro Silveira Gonçalves (DRE 120019402) 
   Data: 21/12/2021
   Código: programa concorrente onde a ordem de execução das threads é controlada no programa, usando variáveis de condição.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// OBSERVAÇÃO: essa versão da resolução contém logs

/*  Condições Lógicas: 
    - Thread 5 deve ser a primeira imprimir sua mensagem
    - Thread 1 deve ser a última a imprimir sua mensagem
    - Thread 2, 3, 4 devem imprimir suas mensagens depois da 5 e antes da 1
*/

// variáveis globais

#define NUMTHREADS 5 // representa o número de threads

    // variáveis de estado
bool thread5passou = false; // representa uma variável de condição/estado que indica se a thread 5 já foi executada(true), ou não(false)
int contador = 0; // representa uma variável de condição/estado que indica quantas threads já foram executadas

    // variáveis de sincronização
pthread_mutex_t lock; // representa uma variável de bloqueio/exclusão mútua 
pthread_cond_t executada5; // representa as threads que aguardam a thread 5 ser executada primeiro para depois serem executadas
pthread_cond_t todasExecutadas; // representa o aguardo pela thread 1 de que todas as outras sejam executadas primeiro

// protótipos de funções

void* thread1(void* t);
void* thread2(void* t);
void* thread3(void* t);
void* thread4(void* t);
void* thread5(void* t);

// função main(fluxo/thread principal)
int main(void)
{
    pthread_t idThreads[NUMTHREADS]; // representa os ids das threads no sistema

    // 1º - Inicializar as variáveis de estado
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&todasExecutadas, NULL);
    pthread_cond_init(&executada5, NULL);

    // 2º - Criar threads
    pthread_create(&idThreads[4], NULL, thread5, NULL);
    pthread_create(&idThreads[3], NULL, thread4, NULL);
    pthread_create(&idThreads[2], NULL, thread3, NULL);
    pthread_create(&idThreads[1], NULL, thread2, NULL);
    pthread_create(&idThreads[0], NULL, thread1, NULL);

    // 3º - Esperar threads terminarem
    for (int i = 0; i < NUMTHREADS; i++)
    {
        pthread_join(idThreads[i], NULL);
    }
    printf("\nFim\n");
    
    // 4º - Desalocar variáveis 
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&executada5);
    pthread_cond_destroy(&todasExecutadas);

    return 0;
}

// definição de funções prototipadas

// Thread 1
void* thread1(void* t)
{
    printf("T1: Início concluído!\n");    

    pthread_mutex_lock(&lock);

    // verificar condição lógica para esta thread executar: deve ser a última a imprimir(5ª a ser impressa)
    if (contador < 4)
    {
        // esperar todas as threads terem imprimido suas mensagens primeiro
        printf("T1: as 4 threads ainda não ocorreram, T1 vai se bloquear ...\n");
        pthread_cond_wait(&todasExecutadas, &lock);
        printf("T1: sinal recebido e mutex realocado\n");
    }   
    // imprimir mensagem e incrementar contador
    printf("T1: Volte sempre!\n");
    contador++;

    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

// Thread 2
void* thread2(void* t)
{
    printf("T2: Início concluído!\n");

    pthread_mutex_lock(&lock);

    // verificar condição lógica para esta thread executar: deve imprimir depois da thread 5 e antes da thread 1
    if(!thread5passou)
    {
        // esperar a thread 5 imprimir sua mensagem primeiro
        printf("T2: T5 ainda não ocorreu, T2 vai se bloquear...\n");
        pthread_cond_wait(&executada5, &lock);
        printf("T2: sinal recebido e mutex realocado\n");
    }
    // imprimir mensagem e incrementar contador
    printf("T2: Fique a vontade.\n");
    contador++;

    // verificar condição lógica para thread 1 executar
    if (contador == 4)
    {
        // sinalizar que thread 1 pode ser executada
        printf("T2 vai sinalizar a condição.\n");
        pthread_cond_signal(&todasExecutadas);
    }

    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

// Thread 3
void* thread3(void* t)
{
    printf("T3: Início concluído!\n");

    pthread_mutex_lock(&lock);

    // verificar condição lógica para esta thread executar: deve imprimir depois da thread 5 e antes da thread 1
    if(!thread5passou)
    {
        // esperar a thread 5 imprimir sua mensagem primeiro
        printf("T3: T5 ainda não ocorreu, T3 vai se bloquear...\n");
        pthread_cond_wait(&executada5, &lock);
        printf("T3: sinal recebido e mutex realocado\n");
    }
    // imprimir mensagem e incrementar contador
    printf("T3: Sente-se por favor.\n");
    contador++;

    // verificar condição lógica para thread 1 executar
    if (contador == 4)
    {
        // sinalizar que thread 1 pode ser executada
        printf("T3 vai sinalizar a condição.\n");
        pthread_cond_signal(&todasExecutadas);
    }
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

// Thread 4
void* thread4(void* t)
{
    printf("T4: Início concluído!\n");
    
    pthread_mutex_lock(&lock);

    // verificar condição lógica para esta thread executar: deve imprimir depois da thread 5 e antes da thread 1
    if(!thread5passou)
    {
        // esperar a thread 5 imprimir sua mensagem primeiro
        printf("T4: T5 ainda não ocorreu, T4 vai se bloquear...\n");
        pthread_cond_wait(&executada5, &lock);
        printf("T4: sinal recebido e mutex realocado\n");
    }
    // imprimir mensagem e incrementar contador
    printf("T4: Aceita um copo d´água?\n");
    contador++;

    // verificar condição lógica para thread 1 executar
    if (contador == 4)
    {
        // sinalizar que thread 1 pode ser executada
        printf("T4 vai sinalizar a condição.\n");
        pthread_cond_signal(&todasExecutadas);
    }
    
    pthread_mutex_unlock(&lock);
    
    pthread_exit(NULL);
}

// Thread 5
void* thread5(void* t)
{
    printf("T5: Início concluído!\n");

    pthread_mutex_lock(&lock);

    if ((thread5passou=true))
    {
        // imprimir mensagem e incrementar contador
        printf("T5: Seja bem-vindo!\n");
        contador++;

        // sinalizar que todas as threads seguintes podem ser executadas
        printf("T5: vai sinalizar a condição\n");
        pthread_cond_broadcast(&executada5);
    }

    // verificar condição lógica para thread 1 executar
    if (contador == 4)
    {
        // sinalizar que thread 1 pode ser executada
        printf("T5 vai sinalizar a condição.\n");
        pthread_cond_signal(&todasExecutadas);
    }

    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}