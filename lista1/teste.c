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

bool contadorMultiploDe100 = false;
bool acabouImpressao = false;
bool atingiuN = false;
    
pthread_mutex_t lock; 
pthread_cond_t multiploDe100; 
pthread_cond_t finalizou;
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

        // se o valor de contador chegou a um multiplo de 100
        if (contador % 100 == 0)
        {
            printf("T1: encontrei um múltiplo de 100\n");
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
        }
        pthread_mutex_unlock (&lock);
        
    }

    printf("T1: contador chegou a N\n");
    pthread_mutex_lock (&lock);
    atingiuN = true;
    pthread_mutex_unlock (&lock);

    pthread_exit(NULL);
}
void* thread2(void* arg)
{
    // enquanto contador não atingiu o valor de N
    while (!atingiuN)
    {
        pthread_mutex_lock(&lock);

        // se contador não é múltiplo de 100
        if(!contadorMultiploDe100)
        {
            // esperar contador ser múltiplo de 100
            pthread_cond_wait(&multiploDe100, &lock);
            printf("T2: recebi sinal de multiplo de 100");
        }

        // se contador é múltiplo de 100: imprimir contador
        printf("T2: Contador = %d\n", contador);

        // sinalizar que acabou a impressão
        acabouImpressao = true;
        pthread_cond_signal(&imprimiu);
        
        pthread_mutex_unlock(&lock);
        
    }

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

