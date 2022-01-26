import jdk.jshell.spi.ExecutionControl;

class LE {
    private int leit, escr;

    // Construtor
    LE() {
        this.leit = 0; //leitores lendo (0 ou mais)
        this.escr = 0; //escritor escrevendo (0 ou 1)
    }

    // Entrada para leitores
    public synchronized void EntraLeitor (int id) {
        try {
            while (this.escr > 0) {
                //if (this.escr > 0) {
                System.out.println ("le.leitorBloqueado("+id+")");
                wait();  //bloqueia pela condicao logica da aplicacao
            }
            this.leit++;  //registra que ha mais um leitor lendo
            System.out.println ("le.leitorLendo("+id+")");
        } catch (InterruptedException e) { }
    }

    // Saida para leitores
    public synchronized void SaiLeitor (int id) {
        this.leit--; //registra que um leitor saiu
        if (this.leit == 0)
            this.notify(); //libera escritor (caso exista escritor bloqueado)
        System.out.println ("le.leitorSaindo("+id+")");
    }

    // Entrada para escritores
    public synchronized void EntraEscritor (int id) {
        try {
            while ((this.leit > 0) || (this.escr > 0)) {
                //if ((this.leit > 0) || (this.escr > 0)) {
                System.out.println ("le.escritorBloqueado("+id+")");
                wait();  //bloqueia pela condicao logica da aplicacao
            }
            this.escr++; //registra que ha um escritor escrevendo
            System.out.println ("le.escritorEscrevendo("+id+")");
        } catch (InterruptedException e) { }
    }

    // Saida para escritores
    public synchronized void SaiEscritor (int id) {
        this.escr--; //registra que o escritor saiu
        notifyAll(); //libera leitores e escritores (caso existam leitores ou escritores bloqueados)
        System.out.println ("le.escritorSaindo("+id+")");
    }
}

//Aplicacao de exemplo--------------------------------------------------------
// Leitor
class Leitor extends Thread {
    int id; //identificador da thread
    int delay; //atraso bobo
    LE monitor;//objeto monitor para coordenar a lógica de execução das threads

    // Construtor
    Leitor (int id, int delayTime, LE m) {
        this.id = id;
        this.delay = delayTime;
        this.monitor = m;
    }

    // Método executado pela thread
    public void run () {
        try {
            for (;;) {
                this.monitor.EntraLeitor(this.id);

                // imprimir valor da variável e se é impar ou par
                System.out.printf("Inicio da leitura pela thread leitora %d\n", this.id);
                System.out.printf("A variável vale %d e é %s.\n",
                        Lab8.variavelCompartilhada,
                        Lab8.variavelCompartilhada % 2 == 0 ? "par" : "impar" );
                System.out.printf("Fim da leitura pela thread leitora %d\n", this.id);
                this.monitor.SaiLeitor(this.id);
                sleep(this.delay);
            }
        } catch (InterruptedException e) { return; }
    }
}

//--------------------------------------------------------
// Escritor
class Escritor extends Thread {
    int id; //identificador da thread
    int delay; //atraso bobo...
    LE monitor; //objeto monitor para coordenar a lógica de execução das threads

    // Construtor
    Escritor (int id, int delayTime, LE m) {
        this.id = id;
        this.delay = delayTime;
        this.monitor = m;
    }

    // Método executado pela thread
    public void run () {

        try {
            for (;;) {
                this.monitor.EntraEscritor(this.id);
                System.out.printf("Inicio da escrita pela thread escritora %d\n", this.id);
                // modificar a variável compartilhada para que valha seu identificador de thread
                System.out.printf("O valor da variável compartilhada era %d.\n", Lab8.variavelCompartilhada);
                Lab8.variavelCompartilhada = this.id;
                System.out.printf("O valor da variável compartilhada agora é %d.\n", Lab8.variavelCompartilhada);
                System.out.printf("Fim da escrita pela thread escritora %d\n", this.id);
                this.monitor.SaiEscritor(this.id);
                sleep(this.delay); //atraso bobo...
            }
        } catch (InterruptedException e) { return; }
    }
}


//--------------------------------------------------------
// Escritora leitora
class LeitorEscritor extends Thread
{
    // atributos
    int id; //identificador da thread
    int delay; //atraso bobo...
    LE monitor; //objeto monitor para coordenar a lógica de execução das threads

    // Construtor
    LeitorEscritor (int id, int delayTime, LE m) {
        this.id = id;
        this.delay = delayTime;
        this.monitor = m;
    }

    // Método executado pela thread
    public void run () {
        try {
            for (;;)
            {
                // leitura da variável compartilhada: imprimir seu valor
                this.monitor.EntraLeitor(this.id);
                System.out.printf("Inicio da leitura pela thread leitora-escritora %d\n", this.id);
                System.out.printf("O valor da variável compartilhada é %d.\n", Lab8.variavelCompartilhada);
                System.out.printf("Fim da leitura pela thread leitora-escritora %d\n", this.id);
                this.monitor.SaiLeitor(this.id);

                // escrita na variável compartilhada

                this.monitor.EntraEscritor(this.id);
                System.out.printf("Inicio da escrita pela thread leitora-escritora %d\n", this.id);
                Lab8.variavelCompartilhada++;
                System.out.printf("O valor da variável compartilhada agora é %d.\n", Lab8.variavelCompartilhada);
                System.out.printf("Fim da escrita pela thread leitora-escritora %d\n", this.id);
                this.monitor.SaiEscritor(this.id);

                sleep(this.delay); //atraso bobo...
            }
        } catch (InterruptedException e) { return; }
    }
}

//--------------------------------------------------------
// Classe principal
class Lab8
{
    static final int L = 4; // representa o número de threads leitoras
    static final int E = 3; // representa o número de threads escritoras
    static final int LE = 4; // representa o número de threads leitoras-escritoras

    public static int variavelCompartilhada; // representa a variável compartilhada entre as threads

    public static void main (String[] args)
    {
        variavelCompartilhada = 0; // inicializar a variável global compartilhada para zero

        // alocar espaço para as threads
        LE monitor = new LE();            // Monitor (objeto compartilhado entre leitores e escritores)
        Leitor[] l = new Leitor[L];       // Threads leitores
        Escritor[] e = new Escritor[E];   // Threads escritores
        LeitorEscritor[] le = new LeitorEscritor[LE]; // Threads leitoras-escritoras

        // Criar e iniciar as threads
        // leitoras
        for (int i = 0; i < L; i++)
        {
            l[i] = new Leitor(i+1, (i+1)*500, monitor);
            l[i].start();
        }
        // escritoras
        for (int i = 0; i < E; i++)
        {
            e[i] = new Escritor(i+1, (i+1)*500, monitor);
            e[i].start();
        }
        // leitoras-escritoras
        for (int i = 0; i < LE; i++)
        {
            le[i] = new LeitorEscritor(i+1, (i+1)*500, monitor);
            le[i].start();
        }
    }
}