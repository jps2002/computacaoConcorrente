public class RecursoCompartilhado
{
    // atributos
    private int[] vetor; // representa o vetor de inteiros a ter seus elementos somados nas threads
    private int somatorio; // representa o valor do somatório parcial dos elementos do vetor compartilhado pelas threads

    // construtores
    public RecursoCompartilhado ( final int tamanhoVetor)
    {
        // alocar espaço para o vetor
        this.vetor = new int[tamanhoVetor];
        this.somatorio = 0;

        System.out.println("Vetor inicial:");
        // inicializar o vetor com valores aleatórios no intervalo [0, 100]
        for (int i = 0; i < tamanhoVetor; i++)
        {
            vetor[i] = getRandomNumber(0, 100);
            System.out.printf("[%d] = %d\n", i, vetor[i]);
        }
    }

    // métodos

    /**
     * Gera um número inteiro no intervalo [min, max]
     * @param min limite inferior
     * @param max limite superior
     * @return inteiro no intervalo [min, max]
     */
    private static int getRandomNumber(int min, int max) {
        return (int) ((Math.random() * (max - min)) + min);
    }


    /**
     * Soma sequencial dos elementos do vetor de vetor[inicio] até inclusive vetor[fim]
     * @param inicio index do primeiro elemento a ser somado
     * @param fim index do último elemento a ser somado
     */
    public synchronized void somar(int inicio, int fim)
    {
        System.out.printf("Estamos somando de [%d] até [%d]\n", inicio, fim);
        for (int i = inicio; i <= fim; i++)
        {
            System.out.printf("Somatório Parcial = somatório parcial atual + [%d] = %d + %d = %d\n",  i, somatorio, vetor[i], somatorio + vetor[i]);
            this.somatorio += this.vetor[i];
        }
    }

    /**
     * Retorna o valor do somatório dos elementos do vetor até o momento.
     * @return valor do somatório
     */
    public synchronized int getSomatorio()
    {
        return this.somatorio;
    }
}

class T extends Thread
{
    // atributos
    private int id; // identificador local da thread
    RecursoCompartilhado s; // objeto compartilhado entre as threads
    private int limiteInferior; // index do elemento do vetor em que deve começar o somatório
    private int limiteSuperior; // index do elemento do vetor em que deve terminar o somatório

    // construtores
    public T(int tid, RecursoCompartilhado s, int inicio, int fim)
    {
        this.id = tid;
        this.s = s;
        this.limiteInferior = inicio;
        this.limiteSuperior = fim;
    }

    // métodos
    public void run()
    {
        System.out.println("Thread " + this.id + " iniciou!");
        this.s.somar(limiteInferior, limiteSuperior);
        System.out.println("Thread " + this.id + " terminou!");
    }
}


// Classe de Aplicação
 class SomaVetor
{
    static final int num_threads = 3; // representa o número de threads desejado
    static final int tamanho_vetor = 10; // representa o tamanho(número de elementos) do vetor

    public static void main(String[] args)
    {
        // alocar espaço para o vetor de threads
        Thread[] threads = new Thread[num_threads]; // representa o vetor de threads de execução

        // criar uma instância do recurso compartilhado entre as threads
        RecursoCompartilhado s = new RecursoCompartilhado(tamanho_vetor);

        // criar variáveis de controle do intervalo de atuação das threads sobre o vetor
        int inicio = 0; // index do primeiro elemento em um intervalo do vetor a ser somado por uma thread
        int fim = inicio + num_threads; // index do último elemento em um intervalo do vetor a ser somado por uma thread

        // cria as threads da aplicação
        for (int i = 0; i < threads.length; i++)
        {
            threads[i] = new T(i, s, inicio , fim);
            System.out.printf("Criando thread %d que somará de [%d] até [%d]\n", i, inicio, fim);
            // atualizar ponteiros de intervalos de atuação
            inicio = fim + 1;
            fim = inicio + num_threads;
            if (fim > (tamanho_vetor-1))
            {
                fim = tamanho_vetor - 1;
            }
        }

        // iniciar execução das as threads
        for (int i = 0; i < threads.length; i++)
        {
            threads[i].start();
        }

        // esperar pelo término de todas as threads
        for (int i = 0; i < threads.length; i++)
        {
            try
            {
                threads[i].join();
            }
            catch (InterruptedException e)
            {
                return;
            }
        }
        // imprimir somatório dos elementos do vetor de inteiros ao final da execução do algoritmo
        System.out.println("Somatório final dos elementos do vetor = " + s.getSomatorio());
    }
}

