/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente (21.2) - Profª Silvana Rossetto
   Resolução do Laboratório 2 do Módulo 1
   
   Autor: João Pedro Silveira Gonçalves (DRE 120019402) 
   Data: 30/11/2021
   Código: Multiplicação de matrizes e avaliação de desempenho com Lei de Amdahl.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

// variáveis globais

float* matrizEntrada1; // representa a primeira matriz de entrada
float* matrizEntrada2; // representa a segunda matriz de entrada
float* matrizSaida; // representa a matriz produto de saída do algoritmo concorrente
int numThreads; // representa o número de threads a serem criadas 

// modelagem de dados

// representa a estrutura de argumentos passadas para a thread
typedef struct
{
    int idElemento; // representa o identificador do elemento a ser processado pela thread
    int dimensaoMatriz1[2]; // representa a dimensão da primeira matriz de entrada
    int dimensaoMatriz2[2]; // representa a dimensão da segunda matriz de entrada
    int dimensaoMatrizSaida[2]; // representa a dimensão da matriz de saída que conterá o produto das entradas
} argumentosThread;

// protótipos de funções

float produtoInterno(float* vetor1, float* vetor2, int dimensaoVetores);
int comparaMatriz(float* matriz1, int dimensaoMatriz1[2], float* matriz2, int dimensaoMatriz2[2]);
float geraAleatorio(float limiteSuperior);
double calculaDesempenho(double tempoSequencial, double tempoConcorrente);
void* tarefa (void *arg);

// função main
int main(int argc, char* argv[])
{
    int dimensaoMatriz1[2]; // representa as dimensões nº de linhas e nº de colunas respectivamente da primeira matriz de entrada
    int dimensaoMatriz2[2]; // representa as dimensões nº de linhas e nº de colunas respectivamente da segunda matriz de entrada
    int dimensaoMatrizSaida[2]; // representa as dimensões nº de linhas e nº de colunas respectivamente da matriz produto de saída
    float* saidaSequencial; // representa a matriz produto de saída do algoritmo sequencial
    double tempoSequencial = 0.0; // representa o tempo de execução do algoritmo sequencial para as matrizes de entrada
    double tempoConcorrente = 0.0; // representa o tempo de execução do algoritmo concorrente para as matrizes de entrada
    double tempoInicial = 0.0, tempoFinal = 0.0, tempoDecorrido = 0.0; // representam variáveis auxiliares no cálculo de execuções de processos
    
    srand((unsigned int)time(NULL));

    // 1º - Leitura e avaliação dos parâmetros de entrada
    
    GET_TIME(tempoInicial);

    if (argc < 4) // args são dimensão e número de threads
    {
        printf("Digite: %s <dimensão da matriz 1> <dimensão da matriz 2> <número de threads>\n", argv[0]);
        return 1;
    }
    dimensaoMatriz1[1] = dimensaoMatriz1[0] = atoi(argv[1]);
    dimensaoMatriz2[1] = dimensaoMatriz2[0] = atoi(argv[2]);
    // a = matriz m x n e b = matriz n x p, dimensão matriz de saída é m x p
    dimensaoMatrizSaida[0] = dimensaoMatriz1[0];
    dimensaoMatrizSaida[1] = dimensaoMatriz2[1];

    numThreads = atoi(argv[3]);

    // TODO: Estabelecer limite do número de threads
    int maxNumThreads = dimensaoMatriz1[0] * dimensaoMatriz2[1];
    if (numThreads > maxNumThreads) 
    {
        numThreads = maxNumThreads;
    } 

    // 2º - Alocação de memória para as estruturas de dados

    matrizEntrada1 = (float*) malloc(sizeof(float) * dimensaoMatriz1[0] * dimensaoMatriz1[1]);
    if (!matrizEntrada1) 
    {
        printf("Error: alocação de memória para matriz 1 de entrada.\n");
    }

    matrizEntrada2 = (float*) malloc(sizeof(float) * dimensaoMatriz2[0] * dimensaoMatriz2[1]);
    if (!matrizEntrada2) 
    {
        printf("Error: alocação de memória para matriz 2 de entrada.\n");
    }

    matrizSaida = (float*) malloc(sizeof(float) * dimensaoMatrizSaida[0] * dimensaoMatrizSaida[1]);
    if (!matrizEntrada2) 
    {
        printf("Error: alocação de memória para matriz saida.\n");
    }

    // alocação de memória para a matriz de saída do algoritmo sequencial 
    saidaSequencial = (float*) malloc(sizeof(float) * dimensaoMatrizSaida[0] * dimensaoMatrizSaida[1]);
    if (!saidaSequencial)
    {
        printf("Error: alocação de memória para saída sequencial.\n");
        return 1;
    }

    // 3º - Inicialização das estruturas de dados

    float contador = 1.0;

    // inicialização da matriz 1
    for (int linha = 0; linha < dimensaoMatriz1[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatriz1[1]; coluna++)
        {
            matrizEntrada1[linha * dimensaoMatriz1[1] + coluna] = geraAleatorio(20.0);
            /* matrizEntrada1[linha * dimensaoMatriz1[1] + coluna] = contador;
            contador++; */
        }
    }
    
    //inicialização da matriz 2
    for (int linha = 0; linha < dimensaoMatriz2[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatriz2[1]; coluna++)
        {
           matrizEntrada2[linha * dimensaoMatriz2[1] + coluna] = geraAleatorio(20.0);
           /*  matrizEntrada2[linha * dimensaoMatriz2[1] + coluna] = contador;
            contador++; */
        }
    }

    // inicialização da matriz de saida
    for (int linha = 0; linha < dimensaoMatrizSaida[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatrizSaida[1]; coluna++)
        {
           
            matrizSaida[linha * dimensaoMatrizSaida[1] + coluna] = 0;
        }
    }
    // inicialização da matriz de saida da versão sequencial
    for (int linha = 0; linha < dimensaoMatrizSaida[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatrizSaida[1]; coluna++)
        {
           
            saidaSequencial[linha * dimensaoMatrizSaida[1] + coluna] = 0;
        }
    }

    GET_TIME(tempoFinal);
    tempoDecorrido = tempoFinal - tempoInicial;
    printf("Tempo inicializacao: %lf s.\n", tempoDecorrido);

    // 4º - Multiplicação da matriz 1 pela matriz 2

    // Forma Sequencial 
    GET_TIME(tempoInicial);

        // multiplicação de matrizes 
    for (int linha = 0; linha < dimensaoMatrizSaida[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatrizSaida[1]; coluna++)
        {
            float valor = 0.0;
            // o valor é o produto interno da linha da matriz1 com a coluna da matriz 2
            float* linhaMatriz1 = (float*) malloc(sizeof(float) * dimensaoMatriz1[1]);
            float* colunaMatriz2 = (float*) malloc(sizeof(float) * dimensaoMatriz2[0]);
            
            // preencher linhaMatriz1
            for (int i = 0; i < dimensaoMatriz1[1]; i++)
            {
                linhaMatriz1[i] = matrizEntrada1[linha * dimensaoMatriz1[1] + i];
            }
            // preencher colunaMatriz2
            for (int i = 0; i < dimensaoMatriz2[0]; i++)
            {
                colunaMatriz2[i] = matrizEntrada2[i* dimensaoMatriz2[1] + coluna];
            }
            
            valor = produtoInterno(linhaMatriz1, colunaMatriz2, dimensaoMatriz1[1]);
            saidaSequencial[linha * dimensaoMatrizSaida[1] + coluna] = valor;
        }
    }

    GET_TIME(tempoFinal);
    tempoSequencial = tempoFinal - tempoInicial;
    printf("Tempo multiplicação sequencial: %lf s.\n", tempoSequencial);

    // Forma Concorrente
    
    GET_TIME(tempoInicial);
        // Alocação das estruturas

    pthread_t* idsThreads; // representa os identificadores das threads no sistema
    idsThreads = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
    if (idsThreads == NULL) 
    {
        printf("Error: alocação de memória para threads.\n");
        return 1;
    }

    argumentosThread* argumentos; // representa os argumentos para as thread: identificadores locais (dos elementos) das threads e dimensão da matriz
    argumentos = (argumentosThread*) malloc(sizeof(argumentosThread) * numThreads);
    if (argumentos == NULL) 
    {
        printf("Error: alocação de memória para argumentos para threads.\n");
        return 1;
    }
        // Criação das threads
    
    for (int thread = 0; thread < numThreads; thread++)
    {
        (argumentos + thread)->idElemento = thread;
        for (int i = 0; i < 2; i++)
        {
            (argumentos + thread)->dimensaoMatriz1[i] = dimensaoMatriz1[i];
            (argumentos + thread)->dimensaoMatriz2[i] = dimensaoMatriz2[i];
            (argumentos + thread)->dimensaoMatrizSaida[i] = dimensaoMatrizSaida[i];
        }
        if (pthread_create(idsThreads+thread, NULL, tarefa, (void*) (argumentos+thread)))
        {
            printf("Error: criação de threads.\n");
            return 1;
        }
    }
        // Esperar término das threads

    for (int thread = 0; thread < numThreads; thread++)
    {
        pthread_join(*(idsThreads + thread), NULL);
    }
    
    GET_TIME(tempoFinal);
    tempoConcorrente = tempoFinal - tempoInicial;
    printf("Tempo multiplicação concorrente: %lf s.\n", tempoConcorrente);

    // 5º - Exibição dos resultados

    // Verificar corretude 
    if (comparaMatriz(matrizSaida, dimensaoMatrizSaida, saidaSequencial, dimensaoMatrizSaida))
        printf("O resultado é correto e igual em ambos o método sequencial e o concorrente.\n");
    else 
        printf("O resultado é incorreto e diferente entre o método sequencial e oconcorrente.\n");

    // Obter desempenho
    printf("Lei de Amdahl: %lf\n", calculaDesempenho(tempoSequencial, tempoConcorrente));

    /*
    printf("Matriz 1 de entrada:\n");
    for (int linha = 0; linha < dimensaoMatriz1[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatriz1[1]; coluna++)
        {
           
            printf("%.1f ", matrizEntrada1[linha * dimensaoMatriz1[1] + coluna]);
        }
        printf("\n");
    }

    printf("Matriz 2 de entrada:\n");
    for (int linha = 0; linha < dimensaoMatriz2[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatriz2[1]; coluna++)
        {
           
            printf("%.1f ", matrizEntrada2[linha * dimensaoMatriz2[1] + coluna]);
        }
        printf("\n");
    }
    
    printf("Matriz de saída:\n");
    for (int linha = 0; linha < dimensaoMatrizSaida[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatrizSaida[1]; coluna++)
        {
           
            printf("%.1f ", matrizSaida[linha * dimensaoMatrizSaida[1] + coluna]);
        }
        printf("\n");
    }

    printf("Matriz de saída sequencial:\n");
    for (int linha = 0; linha < dimensaoMatrizSaida[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatrizSaida[1]; coluna++)
        {
           
            printf("%.1f ", saidaSequencial[linha * dimensaoMatrizSaida[1] + coluna]);
        }
        printf("\n");
    } 
    */

    // 6º - Liberação de memória alocada

    GET_TIME(tempoInicial);
    
    free(matrizEntrada1);
    free(matrizEntrada2);
    free(matrizSaida);
    free(saidaSequencial);
    free(idsThreads);
    free(argumentos);

    GET_TIME(tempoFinal);
    tempoDecorrido = tempoFinal - tempoInicial;
    printf("Tempo finalização: %lf s.\n", tempoDecorrido);
        
}

// definição de funções prototipadas

// realiza parte da multiplicação de matrizes por thread
void* tarefa (void *arg)
{
    argumentosThread* args = (argumentosThread*) arg;

    // printf("Thread %d\n", args->idElemento);

    for (int linha = args->idElemento; linha < args->dimensaoMatrizSaida[0]; linha += numThreads)
    {
        for (int coluna = 0; coluna < args->dimensaoMatrizSaida[1]; coluna++)
        {
            float valor = 0.0; // representa o valor do elemento[linha][coluna] da matriz produto de saída
            // o valor é o produto interno da linha da matriz1 com a coluna da matriz 2
            float* linhaMatriz1 = (float*) malloc(sizeof(float) * args->dimensaoMatriz1[1]);
            float* colunaMatriz2 = (float*) malloc(sizeof(float) * args->dimensaoMatriz2[0]);

            // preencher linhaMatriz1
            for (int i = 0; i < args->dimensaoMatriz1[1]; i++)
            {
                linhaMatriz1[i] = matrizEntrada1[linha * args->dimensaoMatriz1[1] + i];
            }
            // preencher colunaMatriz2
            for (int i = 0; i < args->dimensaoMatriz2[0]; i++)
            {
                colunaMatriz2[i] = matrizEntrada2[i * args->dimensaoMatriz2[1] + coluna];
            }

            valor = produtoInterno(linhaMatriz1, colunaMatriz2, args->dimensaoMatriz1[1]);
            matrizSaida[linha * args->dimensaoMatrizSaida[1] + coluna] = valor;
        }
        
    }

    pthread_exit(NULL);
}

// Retorna o (float) produto interno entre os vetores de mesma dimensão passados
float produtoInterno(float* vetor1, float* vetor2, int dimensaoVetores)
{
    float somatorio = 0;
    for (int i = 0; i < dimensaoVetores; i++)
    {
        somatorio += (vetor1[i] * vetor2[i]);
    }
    
    return somatorio;
}

/* Compara as duas matrizes passadas.
Retorno:
    - int 0, se são diferentes
    - int 1, se são iguais */
int comparaMatriz(float* matriz1, int dimensaoMatriz1[2], float* matriz2, int dimensaoMatriz2[2])
{
    int resultado = 1;
    for (int linha = 0; linha < dimensaoMatriz1[0]; linha++)
    {
        for (int coluna = 0; coluna < dimensaoMatriz1[0]; coluna++)
        {
            if (!(matriz1[linha * dimensaoMatriz1[1] + coluna] == matriz2[linha * dimensaoMatriz2[1] + coluna]))
                resultado = 0;
        }
    }
    return resultado;
}

// Retorna um número float aleatório no intervalo [0, limiteSuperior]
float geraAleatorio(float limiteSuperior)
{
    return ((float)rand()/(float)(RAND_MAX)) * limiteSuperior;
}

// Retorna o valor da Lei de Amdahl para calculo de desempenho
double calculaDesempenho(double tempoSequencial, double tempoConcorrente)
{
    return tempoSequencial / tempoConcorrente;
}