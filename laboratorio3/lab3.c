/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente (21.2) - Profª Silvana Rossetto
   Resolução do Laboratório 3 do Módulo 1
   
   Autor: João Pedro Silveira Gonçalves (DRE 120019402) 
   Data: 07/12/2021
   Código: Contagem dos valores de um vetor que estão dentro dos limites lidos
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define DELTA 50.0

// variáveis globais

long long int dimensaoEntrada; // representa a dimensão do vetor de entrada
int numThreads; // representa o número de threads
float *vetor; // representa o vetor de entrada com dimensãoEntrada elementos
float limiteSuperior = 0.0; // representa o limite superior de avaliação do valor dos elementos do vetor
float limiteInferior = 0.0; // representa o limite inferior de avaliação do valor dos elementos do vetor

// protótipo de funções

void* tarefa(void* arg);
float geraAleatorio(float limiteSuperior);
void inicializaVetorEntrada();
void imprimeVetorEntrada();

// função main(thread principal)
int main(int argc, char* argv[])
{
    pthread_t* idsThreads; // representa o array com os identificadores das threads no sistema

    long long int*  retornoThreads; // representa um ponteiro para onde está guardado o valor de retorno de uma thread
    long long int contadorSequencial = 0; // representa a quantidade de valores do vetor dentro dos limites lidos de acordo com o algoritmo sequencial
    long long int contadorConcorrente = 0; // representa a quantidade de valores do vetor dentro dos limites lidos de acordo com o algoritmo concorrente
    double tempoSequencial = 0.0; // representa o tempo de execução do algoritmo sequencial 
    double tempoConcorrente = 0.0; // representa o tempo de execução do algoritmo concorrente
    double tempoInicial = 0.0, tempoFinal = 0.0; // representam variáveis auxiliares no cálculo de tempo de execuções de processos
    srand((unsigned int)time(NULL));

    // 1º - Leitura e validação dos parâmetros de entrada
    
    // parâmetros de entrada: dimensão do vetor, número de threads, respectivamente
    if (argc < 3)
    {
        fprintf(stderr, "Error: digite %s <dimensão do vetor> <número de threads>\n", argv[0]);
        return 1;
    }

    dimensaoEntrada = atoll(argv[1]);
    numThreads = atoi(argv[2]);

    if (numThreads > dimensaoEntrada)
    {
        fprintf(stderr, "Error: número de threads superior ao de elementos no vetor de entrada.\n");
        return 1;
    }

    // ler da entrada do usuário os limites superior e inferior
    printf("Limite Superior: ");
    scanf("%f", &limiteSuperior);
    printf("Limite Inferior: ");
    scanf("%f", &limiteInferior);

    if (limiteSuperior < limiteInferior)
    {
        fprintf(stderr, "Error: limite superior maior que inferior.\n");
        return 1;
    }
    
    // 2º - Alocação de memória e inicialização das estruturas
    
    // para o vetor de entrada
    inicializaVetorEntrada();
    // imprimeVetorEntrada();

    // para as threads a serem criadas
    if (!(idsThreads = (pthread_t*) malloc(sizeof(pthread_t) * numThreads)))
    {
        fprintf(stderr, "Error: alocação de memória para as threads.\n");
        return 1;
    }   

    // 3º - Contar elementos do vetor de entrada que estão dentro dos limites lidos na entrada

        // Forma Sequencial

    GET_TIME(tempoInicial);
    
    for (long long int i = 0; i < dimensaoEntrada; i++)
    {
        if (limiteSuperior > vetor[i] && vetor[i] > limiteInferior)
        {
            // printf("--> [%lld]: %f\n", i, vetor[i]);
            contadorSequencial++;
        }
    }

    GET_TIME(tempoFinal);
    tempoSequencial = tempoFinal-tempoInicial;

        // Forma Concorrente

    GET_TIME(tempoInicial);

    // Criação das threads
    for (long int thread = 0; thread < numThreads; thread++)
    {
        if (pthread_create(idsThreads + thread, NULL, tarefa, (void*) thread))
        {
            fprintf(stderr, "Error: criação das threads.\n");
            return 1;
        }
    }

    // Aguardar término das threads
    for (long int thread = 0; thread < numThreads; thread++)
    {
        if (pthread_join(*(idsThreads + thread), (void**) &retornoThreads))
        {
            fprintf(stderr, "Error: término das threads.\n");
            return 1;
        }

        // Computação do valor final
        contadorConcorrente += (*retornoThreads);

        // Desalocação da memória utilizada para guardar os retornos das threads
        free(retornoThreads);
    }
    
    GET_TIME(tempoFinal);
    tempoConcorrente = tempoFinal-tempoInicial;

    // 4º - Exibição dos valores finais

    printf("Contador sequencial: %lld\n", contadorSequencial);
    printf("Tempo de execucao sequencial: %lf\n", tempoSequencial);
    printf("Contador concorrente: %lld\n", contadorConcorrente);
    printf("Tempo de execucao concorrente: %lf\n", tempoConcorrente);

    printf("Portanto, os resultados são %s entre os algoritmos sequencial e concorrente.\n", 
        contadorConcorrente == contadorSequencial ? "iguais": "diferentes");

    printf("Aceleração pelo algoritmo concorrente(Tseq/Tconc): %lf\n", tempoSequencial / tempoConcorrente);
    
    // 5º - Desalocação de memória utilizada

    free(vetor);
    free(idsThreads);

}

// Retorna um número float aleatório no intervalo [0, limiteSuperior]
float geraAleatorio(float limiteSuperior)
{
    return ((float)rand()/(float)(RAND_MAX)) * limiteSuperior;
}

// Conta o número de elementos do bloco do vetor passado que estão dentro dos limites passados na entrada
void* tarefa(void* arg)
{
    long int idThread = (long int) arg; // representa o identificador da thread
    long long int tamanhoBloco = dimensaoEntrada / numThreads; // representa o número de elementos a serem trabalhados na thread
    long long int posicaoInicial = idThread * tamanhoBloco; // representa o elemento inicial do bloco da thread
    long long int posicaoFinal = idThread == numThreads - 1 ? dimensaoEntrada : posicaoInicial + tamanhoBloco; // representa o elemento final(não processado) do bloco da thread
    // representa a contagem dos elementos do vetor que estão dentro dos limites lidos em determinada parte do vetor original passada à thread
    long long int* contadorBloco = (long long int*) malloc(sizeof(long long int)); 
    if (!contadorBloco) 
    {
        fprintf(stderr, "Error: alocação de memória para valor de retorno durante thread.\n");
        exit (1);
    }
    *contadorBloco = 0;

    // contar os elementos do vetor que estão dentro dos limites lidos
    for (long long int i = posicaoInicial; i < posicaoFinal; i++)
    {
        if (limiteSuperior > vetor[i] && vetor[i] > limiteInferior)
        {
            // printf("--> [%lld]: %f\n", i, vetor[i]);
            (*contadorBloco)++;
        }
    }

    // retornar o resultado da contagem
    pthread_exit((void*) contadorBloco);     
}

// Inicializa o vetor de entrada com valores floats aleatórios
void inicializaVetorEntrada()
{
    // 1º - alocação de memória
    if (!(vetor = (float*) malloc(sizeof(float) * dimensaoEntrada)))
    {
        fprintf(stderr, "Error: alocação de memória para vetor de entrada.\n");
        exit(1);
    }

    // 2º - inicialização do vetor com valores aleatórios
    for (long long int i = 0; i < dimensaoEntrada; i++)
    {
        /* para definir o valor limite do float aleatório a ser gerado, escolhemos um valor
        superior ao limite superior que geramos somando o limite superior a um certo delta pré-definido */
        vetor[i] = geraAleatorio(limiteSuperior + DELTA);
    }
}

// Imprime os valores dos elementos do vetor de entrada
void imprimeVetorEntrada()
{
    for (long long int i = 0; i < dimensaoEntrada; i++)
    {
        vetor[i] = geraAleatorio(100.0);
        printf("[%lld]: %f\n", i, vetor[i]);
    }
}