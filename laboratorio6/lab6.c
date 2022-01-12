/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente (21.2) - Profª Silvana Rossetto
   Resolução do Laboratório 6 do Módulo 2
   
   Autor: João Pedro Silveira Gonçalves (DRE 120019402) 
   Data: 11/01/2021
   Código: Barreiras, atores e escritores
   Instrução de execução: ./lab6 {numeroDeThreads(int)}
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Variáveis Globais

int NUM_THREADS; // representa o número de threads a serem geradas e também, por requisito, tamanho do vetor de inteiros
int* vetorInteiros; // representa o vetor de inteiros sendo manipulado pelo programa

// Relacionadas à barreira
int bloqueadas = 0; 
pthread_mutex_t x_mutex;
pthread_cond_t x_cond;

// variaveis para sincronizacao de trabalho de threads seguindo o modelo leitor x escritor 
int leit=0; //contador de threads realizando leitora
int escr=0; //contador de threads escrevendo
pthread_mutex_t mutex; 
pthread_cond_t cond_leit, cond_escr;

// Protótipo de Funções

int geraAleatorio(int limiteSuperior);
void* tarefa (void* arg);
void barreira(int numThreads);
void imprimirVetor(void);
void InicEscr (int id);
void FimEscr (int id);
void InicLeit (int id);
void FimLeit (int id);

// Função Main
int main(int argc, char* argv[])
{
    // Ler entrada o usuário e obter número de threads desejado
    if(argc != 2) 
    {
        printf("Usage error: ./lab6 numeroDeThreads\n");
        return 1;
    }
    NUM_THREADS = atoi(argv[1]);

    // Alocar espaço para as variáveis locais necessárias com base no número de threads lido na entrada

    vetorInteiros = (int*) malloc(sizeof(int)*NUM_THREADS);
    pthread_t threads[NUM_THREADS]; // representa os ids das threads no sistema
    int id[NUM_THREADS]; // representa os ids locais das threads
    int tamanhoVetor = NUM_THREADS; // representa o vetor do vetor de inteiros
    int retornoThreads[NUM_THREADS]; // representa os retornos das threads
    int saoIguais = 1; // variável auxiliar que auxilia na verificação da uniformidade de valores de retorno das threads
    srand(time(0)); // alimentar o gerador de números aleatórios

    // Inicializar o vetor com números inteiros aleatórios no intervalo [0, 9]
    for (int elemento = 0; elemento < tamanhoVetor; elemento++)
        vetorInteiros[elemento] = geraAleatorio(10); 
    printf("Valores iniciais do vetor:\n");
    imprimirVetor();

    // Inicializar variáveis de sincronização
    pthread_mutex_init(&x_mutex, NULL); // lock de exclusão mútua
    pthread_cond_init(&x_cond, NULL); // variável de condição

    printf("Início do algoritmo:\n");
    // Criar as threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        id[i] = i;
        pthread_create(&threads[i], NULL, tarefa, (void*) &id[i]);
    }

    // Esperar as threads completarem
    for (int i = 0; i < NUM_THREADS; i++)
    {
        // Esperar as threads terminarem e salvar seus valores de retorno 
        int* retorno;
        pthread_join(threads[i], (void**) &retorno);
        retornoThreads[i] = *retorno;
        printf("Thread [%d]: retornou %d\n", id[i],  retornoThreads[i]);

        // Verificar uniformidade nos valores de retornos
        if(i > 0)
        {
            if (retornoThreads[i] != retornoThreads[i-1])
                saoIguais = 0;
        }
    }   

    // Exibir resultados dos testes de uniformidade dos valores de retornos das threads
    if (saoIguais) 
        printf("Os valores retornados pelas %d threads são iguais.\n", NUM_THREADS);
    else 
        printf("Os valores não são iguais.");

    // Desalocar memória utilizada
    pthread_mutex_destroy(&x_mutex);
    pthread_cond_destroy(&x_cond);
    
    return 0;
}


/**
 * @brief Instaura uma barreira condicional. 
 * @param numThreads número de threads necessárias para romper a barreira
 */
void barreira(int numThreads)
{ 
    // Início da Seção Crítica: 

    pthread_mutex_lock(&x_mutex); // lock

    // se é a última thread a chegar na barreira: mandar sinal
    if (bloqueadas == (numThreads-1))
    {
        pthread_cond_broadcast(&x_cond);
        bloqueadas = 0;
    }
    else // caso contrário: incrementar contagem de bloqueadas e se bloquear até a barreira estar liberada
    {
        bloqueadas++;
        pthread_cond_wait(&x_cond, &x_mutex);
    }

    pthread_mutex_unlock(&x_mutex); // unlock 

    // Fim da Seção Crítica
}

// Definição das Funções Prototipadas

/**
 * @brief Gera números aleatórios no intervalo [0, limiteSuperior - 1]
 * 
 * @param limiteSuperior valor limite para geração de números aleatórios
 * @return int inteiro aleatório
 */
int geraAleatorio(int limiteSuperior) 
    {return rand() % limiteSuperior;}

/**
 * @brief Imprime os valores do vetor global de inteiros sendo manipulado pela aplicação. 
 */
void imprimirVetor(void)
{   
    printf("Estado do vetor de inteiros:\n");
    for (int i = 0; i < NUM_THREADS; i++)
        printf("[%d] = %d\n", i, vetorInteiros[i]);
}

//entrada leitura
void InicLeit (int id) {
   pthread_mutex_lock(&mutex);
   printf("L[%d] quer ler\n", id);
   // se ninguém estiver escrevendo
   while(escr > 0) {
     printf("L[%d] bloqueou\n", id);
     pthread_cond_wait(&cond_leit, &mutex);
     printf("L[%d] desbloqueou\n", id);
   }
   leit++;
   pthread_mutex_unlock(&mutex);
}

/**
 * @brief Retira a thread na lista de threads que querem realizar leitura em uma área compartilhada de memória. 
 * 
 * @param id representa o id local da thread leu a área compartilhada de memória
 * @author Professora Silvana Rossetto em aula
 */
void FimLeit (int id) {
   pthread_mutex_lock(&mutex);
   printf("L[%d] terminou de ler\n", id);
   leit--;
   if(leit==0) pthread_cond_signal(&cond_escr);
   pthread_mutex_unlock(&mutex);
}

/**
 * @brief Insere a thread na lista de threads que querem realizar leitura de uma área compartilhada de memória. 
 * 
 * @param id representa o id local da thread que quer ler
 * @author Professora Silvana Rossetto em aula
 */
void InicEscr (int id) 
{
   pthread_mutex_lock(&mutex);
   printf("Thread [%d] quer escrever\n", id);
   while((leit>0) || (escr>0)) 
   {
     printf("Thread [%d] bloqueou\n", id);
     pthread_cond_wait(&cond_escr, &mutex);
     printf("Thread [%d] desbloqueou\n", id);
   }
   escr++;
   pthread_mutex_unlock(&mutex);
}


/**
 * @brief Retira a thread na lista de threads que querem realizar escrita em uma área compartilhada de memória. 
 * 
 * @param id representa o id local da thread que realizou a escrita.
 * @author Professora Silvana Rossetto em aula
 */
void FimEscr (int id) 
{
   pthread_mutex_lock(&mutex);
   printf("Thread [%d] terminou de escrever\n", id);
   escr--;
   pthread_cond_signal(&cond_escr);
   pthread_cond_broadcast(&cond_leit);
   pthread_mutex_unlock(&mutex);
}

/**
 * @brief Função a ser executada pelas threds(segue as especificaçãoes dadas no enunciado da questão)
 * 
 * @param arg id local da thread
 * @return void* somatório final dos valores do vetor de inteiros após o fim das alterações
 */
void* tarefa (void* arg)
{
    int idThread = * (int*) arg; // representa o identificador local(main) da thread
    int* somatorioVetor = (int* ) malloc(sizeof(int)); // representa o somatório local à função dos elementos do vetor
    *somatorioVetor = 0;

    // Executar N iterações
    for (int iteracao = 0; iteracao < NUM_THREADS;  iteracao++)
    {
        printf("Thread %d --> iteração nº %d\n", idThread, iteracao);
        
        // Processo de Leitura do Vetor de Inteiros:

        InicLeit(idThread);
        
        // No início de cada iteração: somar todos os elementos do vetor de inteiros e salvar na variável local
        for (int elemento = 0; elemento < NUM_THREADS; elemento++)
            *somatorioVetor += vetorInteiros[elemento];

        printf("Somatório na thread %d e iteração %d foi %d\n", idThread, iteracao, *somatorioVetor);
        FimLeit(idThread);

        // sincronização condicional: vamos interromper essa thread e aguardas todas as threads terminarem suas somas
        barreira(NUM_THREADS);

        // Processo de Escrita no Vetor de Inteiros:
        
        // gerar um novo valor aleatório e escrever esse valor na posição do vetor de inteiros correspondente ao seu identificador (ID).
        InicEscr(idThread);
        vetorInteiros[idThread] = geraAleatorio(10);
        printf("Thread %d acabou de atualizar o vetor: [%d] = %d\n", idThread, idThread, vetorInteiros[idThread]);
        FimEscr(idThread);

        // sincronização condicional: vamos interromper essa thread e aguardas todas as threads terminarem de alterar o vetor
        barreira(NUM_THREADS);
    }

    // retornar somatório final
    pthread_exit((void*) somatorioVetor);
}
