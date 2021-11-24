/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente (21.2) - Profª Silvana Rossetto

   Resolução do Laboratório 1 do Módulo 1
   
   Autor: João Pedro Silveira Gonçalves (DRE 120019402) 
   Data: 23/11/2021

   Código: Transformar os valores de um vetor de ARRAYSIZE inteiros em seus respectivos quadrados, dividindo o trabalho em NUMTHREADS threads.
*/


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define ARRAYSIZE 100
#define NUMTHREADS 2

// modelagem de dados

// estrutura que armazena os argumentos para a thread
typedef struct 
{
    int inicio; // representa a posição do vetor de início da transformação
    int range; // representa o número de posições do vetor que se quer transformar de cada vez
    int* array; // representa o array que se quer transformar
} argumentos;

// protótipos de funções
int quadradoInt(int numero);
void* quadradoArrayInt (void* args);

// função main: transforma os valores de um vetor em seus respectivos quadrados, dividindo o trabalho em NUMTHREADS threads
int main(void)
{
    pthread_t idThreadsSis[NUMTHREADS]; // identificadores das threads no sistema
    int meuArray[ARRAYSIZE]; // array que se quer transformar
    argumentos* args; // representa os argumentos para a thread atual
    int threadAtual; // variável auxiliar que serve como identificador local da thread atual

    // atribuir valor ou popular o array com cada posição
    // para fins de testagem: cada posição terá como valor o seu index: 
    for (int i = 0; i < ARRAYSIZE; i++) 
        meuArray[i] = i;

    // imprimir array antes da transformação:
    printf("Vetor inicial é:\n");
    for (int i = 0; i < ARRAYSIZE; i++) 
        printf("[%d] = %d\n", i, meuArray[i]);

    // a transformação ocorrerá por range posições a partir inclusive de inicio em cada thread
    int inicio = 0; // posição do array em que há início da transformação em determinada thread
    int range = ARRAYSIZE/NUMTHREADS; // número de posições que precisam ser transformadas por thread para que haja NUMTHREADS threads.

    // Executar a transformação por meio de NUMTHREADS threads executando quadradoArrayInt
    for (threadAtual = 0; threadAtual < NUMTHREADS; threadAtual++)
    {
        // 1º - Preparar argumentos para a thread atual.
        // printf("--Aloca e preenche argumentos para thread %d\n", threadAtual);
        if (!(args = malloc(sizeof(argumentos))))  
            printf("ERROR: criação de argumentos\n");
        args->array = meuArray;
        args->inicio = inicio;
        args->range = range;

        // 2º - Criar a thread atual e nela executar a transformação por meio de quadradoArrayInt
        // printf("Thread %d criada.\n", threadAtual);
        if (pthread_create(&idThreadsSis[threadAtual], NULL, quadradoArrayInt, (void*) args))
        {
            printf("ERROR: criação de thread\n"); 
            exit(-1);
        }

        /* Se quiséssemos que uma thread fosse executada estritamente em sequência, uma antes da outra:*/
        pthread_join(idThreadsSis[threadAtual], NULL); 

        inicio += range; // atualizar range de atuação de cada thread sobre o vetor 
    }
    
    // printf("Fim da thread principal.\n");

    // imprimir vetor após transformações
    printf("O vetor final é:\n");
    for (int i = 0; i < ARRAYSIZE; i++) 
        printf("[%d] = %d\n", i, meuArray[i]);

    pthread_exit(NULL);
}

// definição de funções

// Tranforma os valores inteiros no range do array passado em seu quadrado
void* quadradoArrayInt (void* args)
{
    argumentos* info = (argumentos*) args;

    /* // imprimir range do array antes da transformação
    printf("O range do vetor inicialmente é:\n");
    for (int i = 0; i < ARRAYSIZE; i++) 
        printf("[%d] = %d\n", i, info->array[i]); */

    // realizar transformação: atualizar os valores no range do vetor para seu quadrado
    for (int i = info->inicio, j = 0; j < info->range; j++)
    {
        info->array[i] = quadradoInt(info->array[i]);
        i++;
    }

    /* // imprimir range do array após a transformação
    printf("O range do vetor ao final é:\n");
    for (int i = 0; i < ARRAYSIZE; i++) 
        printf("[%d] = %d\n", i, info->array[i]); */

    free(args); // liberar o espaço ocupado pelos argumentos na main

    pthread_exit(NULL); // retornar para thread principal
}

// retorna o quadrado do inteiro passado como argumento
int quadradoInt(int numero) 
{
    return (numero*numero);
}
