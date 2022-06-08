// -====================================================================================- 
// TRABALHO EM EQUIPE - ESCALONAMENTO DE PROCESSOS (SISTEMAS OPERACIONAIS - S3 - 2021.1)
// INSTITUTO FEDERAL DE EDUCAÇÃO - CAMPUS CEDRO
// 
// EQUIPE:
// Caio de Almeida Araujo
// Luan Fernandes Alves
// Carlos Eduardo do Nascimento
// Italo Leandro da Silva
// Kayky Bezerra da Silva
// -====================================================================================- 


#include <iostream>
// permite manipulacao de arquivos
#include <fstream>
// permite tokenizacao de strings
#include <bits/stdc++.h>
// permite system pauses
#include <cstdlib>

using namespace std;
// -==========================================-
// CÓDIGOS EXTERNOS UTILIZADOS COMO BIBLIOTECAS
// -==========================================-

// funcao que extrai inteiros de strings
string first_numberstring(string const &str)
{
    char const *digits = "0123456789";
    size_t const n = str.find_first_of(digits);
    if (n != string::npos)
    {
        size_t const m = str.find_first_not_of(digits, n);
        return str.substr(n, m != string::npos ? m - n : m);
    }
    return string();
}

// funcao que torna strings em tokens
vector<string> mystrtok(string str, char delim)
{
    vector<string> tokens;
    string temp = "";
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == delim)
        {
            tokens.push_back(temp);
            temp = "";
        }
        else
            temp += str[i];
    }
    tokens.push_back(temp);
    return tokens;
}

// -=======================================-
// STRUCT USADA PARA INSTANCIAR OS PROCESSOS
// -=======================================-

// A estrutura process representa um processo a ser instanciado e todos os seus atributos
struct Process
{
    // id do processo | tempo de chegada | tempo de rajada | prioridade
    int pID, arrival, burst, priority;
    // tempo em que ficou pronto | tempo em que comeca a executar | tempo de turnaround
    int ready, start, turnaround;
    // quantidade de tempo que ja executou
    int executedTime;
    // quantidade de tempo que falta executar
    int remainingTime;
    // booleano para se o processo terminou de executar
    int terminated;
    // ultimo instante em que o processo foi executado (usado para identificar trocas de contexto)
    int lastExecution;
    // quantidade de trocas de contexto que o processo sofreu
    int contextChanges;
    // armazena quantos quantums foram executados
    int quantumTimer;

    // inicializa os atributos do processo
    Process()
    {
        pID = 0;
        arrival = 0;
        burst = 0;
        priority = 0;
        ready = 0;
        start = 0;
        turnaround = 0;
        executedTime = 0;
        remainingTime = 0;
        terminated = 0;
        lastExecution = 0;
        contextChanges = 0;
        quantumTimer = 0;
    };

    // mostra os dados do processo (usado para debug)
    void show()
    {
        cout << pID << " | arrival:  " << arrival << " | burst: " << burst << " | prio: " << priority << " | ready: " << ready << " | start: " << start << " | turnaround: " << turnaround << " | exec: " << executedTime << " | term: " << terminated << " | last: " << lastExecution << " | context: " << contextChanges << " | quantumTimer: " << quantumTimer << endl;
        system("pause");
    }
};

// -==============================================-
// FILTROS UTILIZADOS PARA ORGANIZAR VETORES (SORT)
// -==============================================-

// compara o tempo de chegada de dois processos diferentes (ordem crescente)
struct smallest_arrival_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.arrival < struct2.arrival);
    }
};

// compara o tempo de rajada de dois processos diferentes (ordem crescente)
struct smallest_burst_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.burst < struct2.burst);
    }
};

// compara o tempo de execucao restante de dois processos diferentes (ordem crescente)
struct smallest_remaining_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.remainingTime < struct2.remainingTime);
    }
};

// compara o tempo de finalizacao de dois processos diferentes (ordem crescente)
struct smallest_ready_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.ready < struct2.ready);
    }
};

// compara o tempo de execucao de dois processos diferentes (ordem decrescente)
struct biggest_executed_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.executedTime > struct2.executedTime);
    }
};

// compara o tempo de execucao de dois processos diferentes (ordem crescente)
struct smallest_executed_time
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.executedTime < struct2.executedTime);
    }
};

// compara a prioridade de dois processos diferentes (ordem crescente)
struct smallest_priority_first
{
    inline bool operator()(const Process &struct1, const Process &struct2)
    {
        return (struct1.priority < struct2.priority);
    }
};

// -=====================================================-
// FUNCOES PRINCIPAIS (FCFS, SRTF, SRTFP, RR, PRIO, PRIOP)
// -=====================================================-

// -==================================================================-
// funcao First Come First Served (primeiro a chegar, primeiro servido)
// -==================================================================-
vector<Process> firstComeFirstServed(vector<Process> processes)
{
    // armazena um vetor temporario com todos os processos;
    for (int i = 0; i < processes.size(); i++)
    {
        // marca a troca de contexto inicial
        processes[i].contextChanges++;

        if (i == 0)
        {
            // calcula o tempo em que o processo ficará pronto
            processes[i].ready = processes[i].burst;

            // calcula o tempo de inicio do programa
            processes[i].start = processes[i].arrival;

            // calcula o tempo de retorno do processo
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
        }
        else
        {
            // calcula o tempo em que o processo ficará pronto
            processes[i].ready = processes[i - 1].ready + processes[i].burst;

            // calcula o tempo de inicio do programa
            processes[i].start = processes[i - 1].ready;

            // calcula o tempo de retorno do processo
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
        }
    }

    // filtra o vetor temporario e o retorna para a funcao main
    sort(processes.begin(), processes.end(), smallest_arrival_time());

    return processes;
}
// -===================================================================================-
// funcao shortestRemainingTimeFirst (SRTF - menor tempo restante de execucao primeiro)
// -===================================================================================-
vector<Process> shortestRemainingTimeFirst(vector<Process> processes, int queueTime)
{
    // armazena os processos que ja chegaram no momento queueTime
    vector<Process> arrieved;

    for (int i = 0; i < processes.size(); i++)
    {
        // atualiza o tempo restante de processamento
        processes[i].remainingTime = processes[i].burst - processes[i].executedTime;

        // checa se o processo foi completado e atualiza os dados caso tenha sido
        if (processes[i].remainingTime == 0 && !processes[i].terminated)
        {
            processes[i].ready = queueTime - 1;
            processes[i].terminated = true;
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
            continue;
        }

        // popula o vetor arrieved com processos que nao tenham sido terminados
        if (queueTime >= processes[i].arrival && !processes[i].terminated)
        {
            arrieved.push_back(processes[i]);
        }
    }

    // filtra o vetor temporario e identifica o processo que devera ser executado no instante queueTime
    sort(arrieved.begin(), arrieved.end(), smallest_remaining_time());
    sort(arrieved.begin(), arrieved.end(), biggest_executed_time());

    if (arrieved.size())
    {
        // armazena o indice do processo que sera executado no instante queueTime
        int index = arrieved[0].pID - 1;

        // marca o tempo que o processo comecou a ser executado e a troca de contexto
        if (!processes[index].start)
        {
            processes[index].start = queueTime;
            processes[index].contextChanges++;
        }

        // procura se houve troca de contexto apos a inicial
        if (processes[index].lastExecution != (queueTime - 1) && processes[index].lastExecution != 0)
        {
            processes[index].contextChanges++;
        }

        // aumenta o tempo de execucao do processo em 1
        processes[index].executedTime++;

        // atualiza o lastExecution
        processes[index].lastExecution = queueTime;
    }

    return processes;
}
// -========================================================================================================-
// funcao shortestRemainingTimeFirstPreemptive (SRTFP - menor tempo restante de execucao primeiro preemptivo)
// -========================================================================================================-
vector<Process> shortestRemainingTimeFirstPreemptive(vector<Process> processes, int queueTime)
{
    // armazena os processos que ja chegaram no momento queueTime
    vector<Process> arrieved;

    for (int i = 0; i < processes.size(); i++)
    {
        // atualiza o tempo restante de processamento
        processes[i].remainingTime = processes[i].burst - processes[i].executedTime;

        // checa se o processo foi completado e atualiza os dados caso tenha sido
        if (processes[i].remainingTime == 0 && !processes[i].terminated)
        {
            processes[i].ready = queueTime - 1;
            processes[i].terminated = true;
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
            continue;
        }

        // popula o vetor arrieved com processos que nao tenham sido completados
        if (queueTime >= processes[i].arrival && !processes[i].terminated)
        {
            arrieved.push_back(processes[i]);
        }
    }

    // filtra o vetor temporario e identifica o processo que devera ser executado no instante queueTime
    sort(arrieved.begin(), arrieved.end(), smallest_remaining_time());
    if (arrieved.size())
    {
        int index = arrieved[0].pID - 1;

        // marca o tempo que o processo comecou a ser executado e a troca de contexto
        if (!processes[index].start)
        {
            processes[index].start = queueTime;
            processes[index].contextChanges++;
        }

        // procura se houve troca de contexto apos a inicial
        if (processes[index].lastExecution != (queueTime - 1) && processes[index].lastExecution != 0)
        {
            processes[index].contextChanges++;
        }

        // aumenta o tempo de execucao do processo em 1
        processes[index].executedTime++;

        // atualiza o lastExecution
        processes[index].lastExecution = queueTime;
    }

    return processes;
}
// -=========================================================================-
// funcao roundRobin (RR - processos com tempo de execucao definido (quantum))
// -=========================================================================-
vector<Process> roundRobin(vector<Process> processes, int queueTime)
{
    // armazena os processos que ja chegaram
    vector<Process> arrieved;

    // marca se o loop de execucao deve executar novamente;
    int runAgain;
    do
    {
        runAgain = processes.size();

        // atualiza o valor do marcador de loop
        for (int i = 0; i < processes.size(); i++)
        {
            // atualiza o tempo restante de processamento
            processes[i].remainingTime = processes[i].burst - processes[i].executedTime;

            // checa se o processo foi completado e atualiza os dados caso tenha sido
            if (processes[i].remainingTime == 0 && !processes[i].terminated)
            {
                processes[i].ready = queueTime - 1;
                processes[i].terminated = true;
                processes[i].turnaround = processes[i].ready - processes[i].arrival;
                continue;
            }

            // popula o vetor arrieved com processos que nao tenham sido terminados
            if (queueTime == processes[i].arrival && !processes[i].terminated)
            {
                arrieved.push_back(processes[i]);
            }

            // checa se a execucao deve continuar
            runAgain -= processes[i].terminated;
        }

        if (arrieved.size())
        {
            // armazena o indice do processo que sera executado no instante queueTime
            int index = arrieved[0].pID - 1;

            // detecta se o processo acabou de executar
            if (processes[index].terminated)
            {
                arrieved.erase(arrieved.begin());
                index = arrieved[0].pID - 1;
            }

            // detecta se o processo chegou ao limite de quantums e o envia ao fim da fila de chegadas
            if (processes[index].quantumTimer == 4)
            {
                arrieved.erase(arrieved.begin());
                arrieved.push_back(processes[index]);

                processes[index].quantumTimer = 0;
                index = arrieved[0].pID - 1;
            }

            // marca o tempo que o processo comecou a ser executado e a troca de contexto
            if (!processes[index].start)
            {
                processes[index].start = queueTime;
                processes[index].contextChanges++;
            }

            // procura se houve troca de contexto apos a inicial
            if (processes[index].lastExecution != (queueTime - 1) && processes[index].lastExecution != 0)
            {
                processes[index].contextChanges++;
            }

            // aumenta o tempo de execucao do processo em 1
            processes[index].executedTime++;
            processes[index].quantumTimer++;

            // atualiza o lastExecution
            processes[index].lastExecution = queueTime;

            // cout << queueTime << " : ";
            // processes[index].show();
        }

        queueTime++;
    } while (runAgain);

    return processes;
}
// -==============================================================-
// funcao priority (PRIO - menor valor prioridade executa primeiro)
// -==============================================================-
vector<Process> priority(vector<Process> processes, int queueTime)
{
    // armazena os processos que ja chegaram no momento queueTime
    vector<Process> arrieved;

    for (int i = 0; i < processes.size(); i++)
    {
        // atualiza o tempo restante de processamento
        processes[i].remainingTime = processes[i].burst - processes[i].executedTime;

        // checa se o processo foi completado e atualiza os dados caso tenha sido
        if (processes[i].remainingTime == 0 && !processes[i].terminated)
        {
            processes[i].ready = queueTime - 1;
            processes[i].terminated = true;
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
            continue;
        }

        // popula o vetor arrieved com processos que nao tenham sido terminados
        if (queueTime >= processes[i].arrival && !processes[i].terminated)
        {
            arrieved.push_back(processes[i]);
        }
    }

    // filtra o vetor temporario e identifica o processo que devera ser executado no instante queueTime
    sort(arrieved.begin(), arrieved.end(), smallest_priority_first());
    sort(arrieved.begin(), arrieved.end(), biggest_executed_time());

    if (arrieved.size())
    {
        // armazena o indice do processo que sera executado no instante queueTime
        int index = arrieved[0].pID - 1;

        // marca o tempo que o processo comecou a ser executado e a troca de contexto
        if (!processes[index].start)
        {
            processes[index].start = queueTime;
            processes[index].contextChanges++;
        }

        // procura se houve troca de contexto apos a inicial
        if (processes[index].lastExecution != (queueTime - 1) && processes[index].lastExecution != 0)
        {
            processes[index].contextChanges++;
        }

        // aumenta o tempo de execucao do processo em 1
        processes[index].executedTime++;

        // atualiza o lastExecution
        processes[index].lastExecution = queueTime;
    }

    return processes;
}
// -====================================================================================-
// funcao priorityPreemptive (PRIOP - menor valor prioridade executa primeiro preemptivo)
// -====================================================================================-
vector<Process> priorityPreemptive(vector<Process> processes, int queueTime)
{
    // armazena os processos que ja chegaram no momento queueTime
    vector<Process> arrieved;

    for (int i = 0; i < processes.size(); i++)
    {
        // atualiza o tempo restante de processamento
        processes[i].remainingTime = processes[i].burst - processes[i].executedTime;

        // checa se o processo foi completado e atualiza os dados caso tenha sido
        if (processes[i].remainingTime == 0 && !processes[i].terminated)
        {
            processes[i].ready = queueTime - 1;
            processes[i].terminated = true;
            processes[i].turnaround = processes[i].ready - processes[i].arrival;
            continue;
        }

        // popula o vetor arrieved com processos que nao tenham sido completados
        if (queueTime >= processes[i].arrival && !processes[i].terminated)
        {
            arrieved.push_back(processes[i]);
        }
    }

    // filtra o vetor temporario e identifica o processo que devera ser executado no instante queueTime
    sort(arrieved.begin(), arrieved.end(), smallest_priority_first());
    if (arrieved.size())
    {
        int index = arrieved[0].pID - 1;

        // marca o tempo que o processo comecou a ser executado e a troca de contexto
        if (!processes[index].start)
        {
            processes[index].start = queueTime;
            processes[index].contextChanges++;
        }

        // procura se houve troca de contexto apos a inicial
        if (processes[index].lastExecution != (queueTime - 1) && processes[index].lastExecution != 0)
        {
            processes[index].contextChanges++;
        }

        // aumenta o tempo de execucao do processo em 1
        processes[index].executedTime++;

        // atualiza o lastExecution
        processes[index].lastExecution = queueTime;
    }

    return processes;
}

int main(int argc, char **argv)
{
    // -==============================-
    // COLETA DOS ARGUMENTOS DA CHAMADA
    // -==============================-

    // armazena o nome do arquivo csv
    string nomeArquivo = argv[1];

    // modo de escalonamento
    string modoEscalonamento = argv[2];

    // modo de execucao
    string modoExecucao = argv[3];

    // representa o arquivo
    ifstream csv;

    // armazena o tempo atual da fila
    int queueTime = 1;

    // -=====================-
    // ABERTURA DO ARQUIVO CSV
    // -=====================-

    // abre o arquivo com base no comando passado pelo usuario no cmd
    csv.open(nomeArquivo, ios::in);

    // avisa caso nao seja possivel abrir o arquivo
    if (!csv)
    {
        cout << "Arquivo nao encontrado";
    }
    else
    {
        // caso seja possivel, testa se o arquivo abriu com sucesso
        if (csv.is_open())
        {

            // -==========================-
            // INSTANCIAMENTO DOS PROCESSOS
            // -==========================-

            // armazena uma instancia temporaria de Process
            Process temp;
            // armazena nove instancias de Process, cada instancia representando um processo individual
            vector<Process> processes;

            // armazena o conteudo de uma linha do arquivo csv
            string line;

            // armazena a linha atual do csv
            int lineCounter = 0;

            while (getline(csv, line))
            {
                // pula a primeira linha do documento (nomes das colunas)
                if (lineCounter == 0)
                {
                    lineCounter++;
                    continue;
                };

                // armazena a coluna atual da linha que esta sendo lida
                int columnCounter = 0;

                // reparte a line a cada virgula
                vector<string> tokens = mystrtok(line, ',');

                for (string line : tokens)
                {
                    // armazena o conteudo do arquivo na memoria por meio dos processes

                    switch (columnCounter)
                    {
                    case 0:
                        temp.pID = stoi(first_numberstring(line));
                        break;

                    case 1:
                        temp.arrival = stoi(first_numberstring(line));
                        break;

                    case 2:
                        temp.burst = stoi(first_numberstring(line));
                        break;

                    case 3:
                        temp.priority = stoi(first_numberstring(line));
                        break;

                    default:
                        cout << "Caso nao encontrado " << endl;
                        break;
                    }

                    columnCounter++;
                }

                processes.push_back(temp);
                lineCounter++;

                system("CLS");
            }
            csv.close();

            // -================-
            // FUNCAO FCFS MODO 1
            // -================-

            // comeca o processamento
            if (modoEscalonamento == "FCFS" && modoExecucao == "1")
            {
                // recebe o vetor filtrado
                processes = firstComeFirstServed(processes);

                cout << "-----------------------------------------" << endl;
                cout << "FUNCAO FIRST COME FIRST SERVED (FCFS - 1)" << endl;
                cout << "-----------------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -================-
            // FUNCAO FCFS MODO 2
            // -================-

            if (modoEscalonamento == "FCFS" && modoExecucao == "2")
            {
                // recebe o vetor filtrado
                processes = firstComeFirstServed(processes);

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;
                };

                cout << "-----------------------------------------" << endl;
                cout << "FUNCAO FIRST COME FIRST SERVED (FCFS - 2)" << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
            };

            // -================-
            // FUNCAO SRTF MODO 1
            // -================-

            if (modoEscalonamento == "SRTF" && modoExecucao == "1")
            {
                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = shortestRemainingTimeFirst(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;

                } while (runAgain);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // exibe os resultados
                cout << "-----------------------------------------------" << endl;
                cout << "FUNCAO SHORTEST REMAINING TIME FIRST (SRTF - 1)" << endl;
                cout << "-----------------------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -================-
            // FUNCAO SRTF MODO 2
            // -================-

            if (modoEscalonamento == "SRTF" && modoExecucao == "2")
            {
                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = shortestRemainingTimeFirst(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;

                } while (runAgain);

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;

                    // processes[i].show();
                };

                cout << "-----------------------------------------------" << endl;
                cout << "FUNCAO SHORTEST REMAINING TIME FIRST (SRTF - 2)" << endl;
                cout << "-----------------------------------------------" << endl;;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
            };

            // -=================-
            // FUNCAO SRTFP MODO 1
            // -=================-

            if (modoEscalonamento == "SRTFP" && modoExecucao == "1")
            {

                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = shortestRemainingTimeFirstPreemptive(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;

                } while (runAgain);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // exibe os resultados
                cout << "-----------------------------------------------------------" << endl;
                cout << "FUNCAO SHORTEST REMAINING TIME FIRST PREEMPTIVE (SRTFP - 1)" << endl;
                cout << "-----------------------------------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -=================-
            // FUNCAO SRTFP MODO 2
            // -=================-

            if (modoEscalonamento == "SRTFP" && modoExecucao == "2")
            {
                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = shortestRemainingTimeFirstPreemptive(processes, queueTime);

                    // atualiza o valor do marcador de loop
                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;
                } while (runAgain);

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;

                    // processes[i].show();
                };

                cout << "-----------------------------------------------------------" << endl;
                cout << "FUNCAO SHORTEST REMAINING TIME FIRST PREEMPTIVE (SRTFP - 2)" << endl;
                cout << "-----------------------------------------------------------" << endl;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
            };

            // -=================-
            // FUNCAO RR MODO 1
            // -=================-

            if (modoEscalonamento == "RR" && modoExecucao == "1")
            {

                processes = roundRobin(processes, queueTime);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // exibe os resultados
                cout << "----------------------------------" << endl;
                cout << "FUNCAO ROUND ROBIN (RR - 1)" << endl;
                cout << "----------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -=================-
            // FUNCAO RR MODO 2
            // -=================-

            if (modoEscalonamento == "RR" && modoExecucao == "2")
            {

                processes = roundRobin(processes, queueTime);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;

                    // processes[i].show();
                };

                cout << "-----------------------------------------" << endl;
                cout << "FUNCAO ROUND ROBIN (RR - 2)" << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
                
            };

            // -=================-
            // FUNCAO PRIO MODO 1
            // -=================-

            if (modoEscalonamento == "PRIO" && modoExecucao == "1")
            {

                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = priority(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                        // processes[i].show();
                    }

                    queueTime++;

                } while (runAgain);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // exibe os resultados
                cout << "----------------------------------" << endl;
                cout << "FUNCAO PRIORITY (PRIO - 1)" << endl;
                cout << "----------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -=================-
            // FUNCAO SRTFP MODO 2
            // -=================-

            if (modoEscalonamento == "PRIO" && modoExecucao == "2")
            {
                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = priority(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                        // processes[i].show();
                    }

                    queueTime++;

                } while (runAgain);

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;

                    // processes[i].show();
                };

                cout << "-----------------------------------------" << endl;
                cout << "FUNCAO PRIORITY (PRIO - 1)" << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
            };

            // -=================-
            // FUNCAO PRIOP MODO 1
            // -=================-

            if (modoEscalonamento == "PRIOP" && modoExecucao == "1")
            {

                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = priorityPreemptive(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;

                } while (runAgain);

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                // exibe os resultados
                cout << "--------------------------------------" << endl;
                cout << "FUNCAO PRIORITY PREEMPTIVE (PRIOP - 1)" << endl;
                cout << "--------------------------------------" << endl;
                cout << "PID    |CHEGADA | RAJADA | TERMINO" << endl;

                for (int i = 0; i < processes.size(); i++)
                {
                    cout.width(6);
                    cout << processes[i].pID;
                    cout << " | " ;
                    cout.width(6);
                    cout << processes[i].arrival;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].burst;
                    cout << " | ";
                    cout.width(6);
                    cout << processes[i].ready;
                    cout << endl;
                }
            };

            // -=================-
            // FUNCAO PRIOP MODO 2
            // -=================-

            if (modoEscalonamento == "PRIOP" && modoExecucao == "2")
            {
                // marca se o loop de execucao deve executar novamente;
                int runAgain;
                do
                {
                    runAgain = processes.size();

                    // recebe o vetor filtrado
                    processes = priorityPreemptive(processes, queueTime);

                    for (int i = 0; i < processes.size(); i++)
                    {
                        runAgain -= processes[i].terminated;
                    }

                    queueTime++;

                } while (runAgain);

                // armazena o tempo total de processameto
                int totalProcessTime = 0;

                // soma os tempos de espera dos processos
                int totalWaitTime = 0;

                // soma os tempos de retorno dos processos
                int totalTurnaroundTime = 0;

                // soma a quantidade de trocas de contexto
                int totalContextChanges = 0;

                // organiza os processos em ordem crescente de tempo de finalização
                sort(processes.begin(), processes.end(), smallest_ready_time());

                for (int i = 0; i < processes.size(); i++)
                {
                    totalProcessTime = processes[i].ready;
                    totalTurnaroundTime += processes[i].turnaround;
                    totalWaitTime += processes[i].start;
                    totalContextChanges += processes[i].contextChanges;

                    // processes[i].show();
                };

                cout << "-----------------------------------------" << endl;
                cout << "FUNCAO PRIORITY PREEMPTIVE (PRIOP - 2)" << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Tempo total de espera             = " << (totalProcessTime) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de retorno         = " << (totalTurnaroundTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Media do tempo de espera          = " << (totalWaitTime / processes.size()) << endl;
                cout << "-----------------------------------------" << endl;
                cout << "Quantidade de trocas de contexto  = " << (totalContextChanges) << endl;
                cout << "-----------------------------------------" << endl;
            } 

            // Filtra entradas invalidas e retorna ao usuario uma mensagem
            if (modoEscalonamento != "FCFS" && modoEscalonamento != "SRTF" && modoEscalonamento != "SRTFP" && modoEscalonamento != "RR" && modoEscalonamento != "PRIO" && modoEscalonamento != "PRIOP" || (modoExecucao != "1" && modoExecucao != "2")){
                cout << "---------------------------------------------------------------------------------------------------------" << endl;
                cout << "O escalonador " << modoEscalonamento << " no modo de execucao " << modoExecucao << " nao esta disponivel." << endl;
                cout << "Os escalonadores disponiveis sao (FCFS, SRTF, SRTFP, RR, PRIO e PRIOP), nos modos de execucao 1 e 2." << endl;
                cout << "---------------------------------------------------------------------------------------------------------" << endl;
            };
        }
        else
        {
            cout << "O arquivo nao pode ser aberto";
        }
        return 0;
    }
}
