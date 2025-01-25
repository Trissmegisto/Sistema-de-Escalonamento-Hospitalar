#include "Paciente.cpp"
#include "Procedimento.cpp"
#include <string>

class Fila {
private:
    struct No {
        Paciente* paciente;
        double tempoEntradaFila;
        No* proximo;

        No(Paciente* p, double tempo) : paciente(p), tempoEntradaFila(tempo), proximo(nullptr) {}
    };

    No* inicio;
    No* fim;
    int tamanho;
    std::string nomeProcedimento;  // Nome do Procedimento associado a essa fila
    int prioridade;               // Prioridade dessa fila (se aplicável)

    //Estatísticas
    double tempoTotalEspera;
    int pacientesAtendidos;
    int* historicoDeTamanho;    // Array para guardar tamanho da fila em diferentes tempos
    int capacidadeHistorico;
    int posicaoHistorico;

public:
    // Construtor
    Fila(const std::string& procedimento, int prioridade = 0, int capacidadeHistorico = 1000)
        : inicio(nullptr), fim(nullptr), tamanho(0), nomeProcedimento(procedimento),
          prioridade(prioridade), tempoTotalEspera(0), pacientesAtendidos(0),
          capacidadeHistorico(capacidadeHistorico), posicaoHistorico(0) {
            historicoDeTamanho = new int[capacidadeHistorico];
            for(int i = 0; i < capacidadeHistorico; i++) {
                historicoDeTamanho[i] = 0;
            }
          }  

    // Destrutor
    ~ Fila() {
        while(inicio != nullptr) {
            No* temp = inicio;
            inicio = inicio->proximo;
            delete temp;
        }
        delete[] historicoDeTamanho;
    }

    // Inicializa a fila
    void inicializa() {
        while(inicio != nullptr) {
            No* temp = inicio;
            inicio = inicio->proximo;
            delete temp;
        }
        fim = nullptr;
        tamanho = 0;
        tempoTotalEspera = 0;
        pacientesAtendidos = 0;
        posicaoHistorico = 0;
    }

    void enfileira(Paciente* paciente, double tempoAtual) {
        No* novoNo = new No(paciente, tempoAtual);

        if(filaVazia()) {
            inicio = fim = novoNo;
        } else {
            // Se a fila tem prioridade, insere ordenado por prioridade
            if(prioridade > 0) {
                if(paciente->getPrioridade() > inicio->paciente->getPrioridade()) {
                    novoNo->proximo = inicio;
                    inicio = novoNo;
                } else {
                    No* atual = inicio;
                    No* anterior = nullptr;

                    while(atual != nullptr && paciente->getPrioridade() <= atual->paciente->getPrioridade()) {
                            anterior = atual;
                            atual = atual->proximo;
                    }

                    anterior->proximo = novoNo;
                    novoNo->proximo = atual;
                    if(atual == nullptr) fim = novoNo;
                }
            } else {
                fim->proximo = novoNo;
                fim = novoNo;
            }
        }
        tamanho++;
        registraTamanho(tempoAtual);
    }

    Paciente* desenfileira(double tempoAtual) {
        if(filaVazia()) return nullptr;

        No* temp = inicio;
        Paciente* paciente = temp->paciente;
        inicio = inicio->proximo;

        if(inicio == nullptr) fim = nullptr;

        // Registra estatísticas
        tempoTotalEspera += (tempoAtual - temp->tempoEntradaFila);
        pacientesAtendidos++;
        tamanho--;

        delete temp;
        registraTamanho(tempoAtual);

        return paciente;
    }

    // Verifica se a fila está vazia
    bool filaVazia() const {
        return inicio == nullptr;
    }

    // Finaliza a fila e retorna estatísticas
    void finaliza(double tempoTotal, double& tempoMedioEspera, double& tamanhoMedioFila) {
        if(pacientesAtendidos > 0) {
            tempoMedioEspera = tempoTotalEspera / pacientesAtendidos;
            
            // Calcula tamanho médio da fila
            int somaTamanhos = 0;
            for(int i = 0; i < posicaoHistorico; i++) {
                somaTamanhos += historicoDeTamanho[i];
            }
            tamanhoMedioFila = posicaoHistorico > 0 ? 
                              (double)somaTamanhos / posicaoHistorico : 0;
        } else {
            tempoMedioEspera = 0;
            tamanhoMedioFila = 0;
        }
    }

    // Registra o tamanho da fila no histórico
    void registraTamanho(double tempoAtual) {
        if(posicaoHistorico < capacidadeHistorico) {
            historicoDeTamanho[posicaoHistorico++] = tamanho;
        }
    }

    // Getters
    int getTamanho() const { return tamanho; }
    std::string getNomeProcedimento() const { return nomeProcedimento; }
    int getPrioridade() const { return prioridade; }
    
    // Retorna o próximo paciente sem removê-lo da fila
    Paciente* getProximoPaciente() const {
        return filaVazia() ? nullptr : inicio->paciente;
    }

    // Verifica se um paciente específico já está na fila
    bool contemPaciente(Paciente* paciente) const {
        No* atual = inicio;
        while (atual != nullptr) {
            if (atual->paciente == paciente) {
                return true;
            }
            atual = atual->proximo;
        }
        return false;
    }
};

// Classe para gerenciar todas as filas do hospital
class GerenciadorFilas {
private:
    static const int NUM_PROCEDIMENTOS = 6;
    static const int MAX_PRIORIDADES = 3;
    Fila*** filas;  // Matriz de filas [procedimento][prioridade]

public:
    // Construtor
    GerenciadorFilas() {
        filas = new Fila**[NUM_PROCEDIMENTOS];
        for(int i = 0; i < NUM_PROCEDIMENTOS; i++) {
            filas[i] = new Fila*[MAX_PRIORIDADES];
            for(int j = 0; j < MAX_PRIORIDADES; j++) {
                filas[i][j] = nullptr;
            }
        }
        
        // Inicializa as filas necessárias
        std::string procedimentos[] = {
            "Triagem", "Atendimento", "Medidas", 
            "Testes", "Imagem", "Instrumentos/Medicamentos"
        };
        
        for(int i = 0; i < NUM_PROCEDIMENTOS; i++) {
            // Triagem e Atendimento têm filas com prioridade
            if(i <= 1) {
                for(int p = 0; p < MAX_PRIORIDADES; p++) {
                    filas[i][p] = new Fila(procedimentos[i], p + 1);
                }
            } else {
                // Outros procedimentos têm fila única
                filas[i][0] = new Fila(procedimentos[i]);
            }
        }
    }
    
    // Destrutor
    ~GerenciadorFilas() {
        for(int i = 0; i < NUM_PROCEDIMENTOS; i++) {
            for(int j = 0; j < MAX_PRIORIDADES; j++) {
                delete filas[i][j];
            }
            delete[] filas[i];
        }
        delete[] filas;
    }
    
    // Obtém a fila apropriada para um procedimento e prioridade
    Fila* getFila(const std::string& procedimento, int prioridade = 0) {
        int procIndex = getProcedimentoIndex(procedimento);
        if(procIndex >= 0) {
            // Para Triagem e Atendimento, usa a prioridade
            if(procIndex <= 1 && prioridade > 0 && prioridade <= MAX_PRIORIDADES) {
                return filas[procIndex][prioridade - 1];
            }
            // Para outros procedimentos, usa a fila única
            return filas[procIndex][0];
        }
        return nullptr;
    }
    
private:
    // Converte nome do procedimento para índice
    int getProcedimentoIndex(const std::string& procedimento) {
        if(procedimento == "Triagem") return 0;
        if(procedimento == "Atendimento") return 1;
        if(procedimento == "Medidas") return 2;
        if(procedimento == "Testes") return 3;
        if(procedimento == "Imagem") return 4;
        if(procedimento == "Instrumentos/Medicamentos") return 5;
        return -1;
    }
};