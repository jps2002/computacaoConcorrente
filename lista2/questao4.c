/* Universidade Federal do Rio de Janeiro - Instituto de Computação
   Disciplina: Computação Concorrente em 21.2 mistrado pela Profª Silvanna Rossetto.
   Autor: João Pedro Silveira Gonçalves
   Código: Resolução das questões 4 e 5 da lista 2 de computação concorrente
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>


/*
    Uso: ./questao4 <tamanho da fila de impressão> <limite de caracteres em uma frase> <número threads consumidoras> <número de threads produtoras>
    // Sugestão para visualização clara de logs: ./questao4 3 10 1 1 
*/ 


// Variáveis Globais
    // Relacionadas à sincronização condicional
sem_t slotCheio; // representa a quantidade de slots preenchidos na fila, isto é, número de requisições aguardando a serem atendidas
sem_t slotVazio; // representa a quantidade de slots vazios na fila

	// Relacionados à sincronização por exclusão mútua
sem_t mutexProdutor; 
sem_t mutexConsumidor;

	// Variáveis compartilhadadas entre threads
int tamanhoFila = 0; // representa o número máximo de requisições suportadas na fila de impressão
int tamanhoPalavra = 0; // representa o tamanho máximo de uma frase/palavra
char** fila = NULL; // representa a fila de impressão

    // Constantes relacionadas 
int numThreadsConsumidoras = 1;
int numThreadsProdutoras = 1;

// Protótipos de funções 
    // Manipulação da fila
void iniciarFila();
void reinicializaFila();
void imprimirFila();
void Insere (int idThread, char* aSerInserido);
void Retira(int idThread);

    // Função das threads
      // Produtoras  
void* produtor(void* arg);
        // Consumidoras
void* consumidor (void* arg);

/**
 * @brief Tarefa executada pelas threads produtoras
 * 
 * @param arg é o id da thread
 * @return void* 
 */
void* produtor( void* arg)
{
    int *tid = (int*) arg; // representa o id local da thread 
    printf("Thread produtora %d foi criada.\n", *tid);

    // Caso de Teste: 

    /* Insere(*tid, "hey");
    Insere(*tid, "hey");
    Insere(*tid, "hey"); */
	while(1)
	{
        // mude o valor a ser  inserido na na fila de impressão
	    Insere(*tid, "hey"); // produzir elemento()
	} 
	pthread_exit(NULL);
}

/**
 * @brief Função executada pelas threads consumidoras
 * 
 * @param arg é o id da thread
 * @return void* 
 */
void* consumidor (void* arg)
{
	int *tid = (int*) arg; // representa o id local da thread 
    printf("Thread consumidora %d foi criada.\n", *tid);

    /* Caso de teste:
    Retira(*tid);
    Retira(*tid);
    Retira(*tid); */

	while(1)
	{
        Retira(*tid); // consumir o elemento....
	} 
	pthread_exit(NULL);
}


/**
 * @brief Insere uma string aSerInserido na fila de impressão
 * 
 * @param idThread 
 * @param aSerInserido 
 */
void Insere (int idThread, char* aSerInserido)
{
	static int in = 0; // representa a próxima posição vazia disponível para inserção na fila de impressão
   // representa o elemento a ser inserido na posição in da fila

	// Aguardar slot vazio
    printf("Thread produtora %d quer inserir.\n", idThread);
	sem_wait(&slotVazio); // Há menos um espaço vazio na fila

	// Exclusão mútua entre produtores
	sem_wait(&mutexProdutor); // bloqueia outras produtoras de acessarem área crítica

    printf("Thread produtora %d vai inserir na posição fila[%d].\n", idThread, in);
    fila[in] = aSerInserido;
    printf("Thread produtora %d acabou inserção: fila[%d] = %s.\n", idThread, in, fila[in]);

	in = (in + 1) % tamanhoFila;

	sem_post(&mutexProdutor); // desbloqueia outras produtoras para acessarem área crítica
	
	// Sinalizar que há mais um slot cheio na fila após a inserção
	sem_post(&slotCheio); // desbloqueia consumidor
}

/**
 * @brief Atende a uma requisição de impressão e a retira da fila de impressão.
 * 
 * @param idThread 
 */
void Retira(int idThread)
{
    char item[tamanhoPalavra]; // representa a palavra a ser retirada da fila e que será impressa
	static int out = 0; // próxima posição preenchida/requisição na fila a ser retirada/atendida
	
	// Aguarda  por um slot cheio/preenchido
    printf("Thread consumidora %d quer retirar.\n", idThread);
	sem_wait(&slotCheio); // há menos um espaço preenchido na fila

	// exclusão mútua entre consumidores
	sem_wait(&mutexConsumidor);  // bloqueia outras consumidoras de acessarem área crítica
    printf("Thread consumidora %d vai retirar na posição fila[%d] = %s.\n", idThread, out, fila[out]);

    // Retirar item apontado por out da fila
    strcpy(item, fila[out]);
    fila[out] = NULL;
    
    // Imprimir item retirado
    printf("------ INICIO DA IMPRESSAO ------\n");
    for (int i = 0; i < strlen(item); i++) printf("-");
    printf("\n%s\n", item == NULL? "NULL2" : item);
    for (int i = 0; i < strlen(item); i++) printf("-");
    printf("\n");
    printf("------ FIM DA IMPRESSAO ------\n");

    printf("Thread consumidora %d acabou retirada: fila[%d] = %s.\n", idThread, out, fila[out] == NULL? "NULL" : fila[out]);
    

    // QUESTÃO 5: imprimir o número de requisições que aguardam a serem atendidas:
    int valorSemaforo = 0;
    sem_getvalue(&slotCheio, &valorSemaforo);
    printf("Thread consumidora %d informa: há agora %d requisições na fila de impressão a serem atendidos.\n", idThread, valorSemaforo);
    
	out = (out + 1) % tamanhoFila; 

    sem_post(&mutexConsumidor); // desbloqueia outras consumidoras para acessarem área crítica
	
	// sinalizar que há mais um slot vazio na fila após a retirada
	sem_post(&slotVazio); // desbloqueia uma produtora
}


// Função main
int main(int argc, char* argv[])
{
    // Ler e validar entrada do usuário
    if (argc != 5)
    {
        printf("Usage error: ./questao4 <tamanho da fila de impressão> <tamanho maximo conteúdo> <numero de consumidores> <numero de produtores>\n");
        return 1;
    }
    tamanhoFila = atoi(argv[1]);
    tamanhoPalavra = atoi(argv[2]);
    numThreadsConsumidoras = atoi(argv[3]);
    numThreadsProdutoras = atoi(argv[4]);

    int numTotalThreads = numThreadsConsumidoras+ numThreadsProdutoras; // representa a quantidade de threads a serem criadas(consumidoras + produtoras) 
    pthread_t threadIds [numTotalThreads]; // representa o id local das threads
    int *id[numTotalThreads]; // representa os ponteiros para os ids locais das threads no sistema(argumento das threads)
    
    // Alocar espaço para a fila de impressão e inicializá-la
    iniciarFila();
    printf("INÍCIO:\n");
    imprimirFila();

    // Inicializar de semáforos 
	    // Relacionados à sincronização condicional
    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, tamanhoFila);
	    // Relacionados à sincronização por exclusão mútua
    sem_init(&mutexConsumidor, 0, 1); // binário
    sem_init(&mutexProdutor, 0, 1);

    // Inicializar argumentos para as threads
    // printf("Id das threads a serem criadas:\n");
    for (int thread = 0, temp = 0; thread < numTotalThreads; thread++) 
    {
        if (!(id[thread] = malloc(sizeof(int)))) 
        {
            pthread_exit(NULL); 
            return 1;
        }
        
        if (thread >= numThreadsConsumidoras) // garantir que threads produtoras tenham ids de 1 a numThreadsProdutoras
        {
            *id[thread] = temp+1;
            temp++;
            // printf("Futura produtora: id[%d] = %d\n", thread, *id[thread]);
        }
        else // garantir que threads consumidoras tenham ids de 1 a numThreadsConsumidoras
        {
            *id[thread] = thread+1; 
            // printf("Futura consumidora: id[%d] = %d\n", thread, *id[thread]);
        }
    }

    
    // Criar threads
        // Threads consumidoras
    for (int i = 0; i < numThreadsConsumidoras; i++)
    {
        printf("Criando thread %d consumidora\n", *id[i]);
        if (pthread_create(&threadIds[i], NULL, consumidor, (void*) id[i])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
        
    }
        // Threads produtoras
    for (int i = numThreadsConsumidoras; i < numTotalThreads; i++)
    {
        printf("Criando thread %d produtora\n", *id[i]);
        if (pthread_create(&threadIds[i], NULL, produtor, (void*) id[i])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
    }
  
    //   Esperar as threads terminarem
    for (int thread = 0; thread < numTotalThreads; thread++) 
    {
        if (pthread_join(threadIds[thread], NULL)) 
        {
            printf("Error: pthread_join()\n"); 
            exit(-1); 
        } 

        free(id[thread]);
    } 

    pthread_exit(NULL);
    
    return 0;
}

// Definição das funções prototipadas

/**
 * @brief Torna todos os campos da fila NULL
 * 
 */
void reinicializaFila ()
{
    for (size_t i = 0; i < tamanhoFila; i++)
    {
        // char* elementoAtual = fila[i];
        // free(elementoAtual);
        fila[i]  = NULL;
    }
}

/**
 * @brief Imprime o conteúdo da fila
 * 
 */
void imprimirFila()
{
    printf("------ ESTADO DA FILA ------\n");
    // printf("Fila:\n");
    for (int i = 0; i < tamanhoFila; i++)
    {
        printf("fila[%d] = %s\n", i, fila[i] == NULL ? "NULL" : fila[i]);
    }
    printf("----------------------------\n");
    

}

/**
 * @brief Aloca espaço e inicializa a fila com tamanhoFila elementos NULL
 * 
 */
void iniciarFila()
{
    fila = (char**) malloc(sizeof(char*) * tamanhoFila);

    for (int i = 0; i < tamanhoFila; i++)
    {
        fila[i] = (char*) malloc(sizeof(char) * tamanhoPalavra);
        fila[i] = NULL;
    }
}