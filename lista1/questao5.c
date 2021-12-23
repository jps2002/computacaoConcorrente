#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

/*
Condiçoes Logicas:
- thread1 deve aguardar a impressao ser concluida antes de alterar o valor de contador
- thread2 imprime na tela o valor de contador sempre que este chega a um valor multiplo de 100
- thread2 deve finalizar quando nao houver mais necessidade de sua atuacao
*/

// variáveis globais

#define NUMTHREADS 2 // representa o número de threads
#define N 500 

int contador = 0;

    // variáveis de estado
// bool imprimiu = false; representa uma variável de condição/estado que indica se o valor de contador já foi impresso ou não
bool acabouLoop = false;
bool contadorMultiploDe100 = false;
bool acabouImpressao = false;
// int contador = 0; // representa uma variável de compartilhada entre as threads cujo valor serve para condicionar a execução da thread2
    
    // variáveis de sincronização
pthread_mutex_t lock; // representa uma variável de bloqueio/exclusão mútua 
pthread_cond_t multiploDe100; // representa o aguardo por contador ser múltiplo de 100 para execução de thread 2
pthread_cond_t finalizou; // representa o aguardo da thread2 por seu fim
pthread_cond_t imprimiu;

// definição de funções

void* thread1(void* arg)
{
    for (int i = 1; i <= N; i++)
    {
        pthread_mutex_lock(&lock);

        contadorMultiploDe100 = false;

        // incrementar contador 
        contador++;

        pthread_mutex_unlock(&lock);

        // se o valor de contador chegou a um multiplo de 100
        if (contador % 100 == 0)
        {
            pthread_mutex_lock (&lock);
            // sinalizar a thread2 do valor multiplo de 100 de contador
            contadorMultiploDe100 = true;
            printf("T1: contador = %d vai sinalizar\n", contador);
            acabouImpressao = false;
            pthread_cond_signal(&multiploDe100);

            // se não acabou a impressão, esperar 
            if (!acabouImpressao)
            {
                printf("T1: impressão ainda não acabou, vou esperar\n");
                pthread_cond_wait(&imprimiu, &lock);
                printf("T1: impressão acabou, estou prosseguindo\n");
            }
            pthread_mutex_unlock (&lock);
        }

        
    }

    printf("T1: contador chegou a N\n");
    // se acabou o loop, não precisa mais da thread 2
    pthread_mutex_lock(&lock);
    acabouLoop = true;

    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}
void* thread2(void* arg)
{
    pthread_mutex_lock(&lock);

    if(!contadorMultiploDe100)
    {
        pthread_cond_wait(&multiploDe100, &lock);
    }

    printf("T2: Contador = %d\n", contador);

    acabouImpressao = true;

    pthread_cond_signal(&imprimiu);

    pthread_mutex_unlock(&lock);


    pthread_exit(NULL);
}


// função main(ou fluxo principal)
int main(void)
{
    pthread_t idThreads[NUMTHREADS]; // representa os ids das threads no sistema

    // 1º - Inicializar as variáveis de estado
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&multiploDe100, NULL);
    pthread_cond_init(&finalizou, NULL);
    pthread_cond_init(&imprimiu, NULL);

    // 2º - Criar threads
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
    pthread_cond_destroy(&multiploDe100);
    pthread_cond_destroy(&finalizou);
    pthread_cond_destroy(&imprimiu);

    return 0;
}

