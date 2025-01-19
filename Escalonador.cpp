#include <vector>
#include "Fila.cpp"

class Evento {
private:
    double dataHora;        // Tempo do evento em horas desde a referência
    int tipo;               // Tipo do evento (chegada, início procedimento, fim procedimento, etc)
    Paciente* paciente;     // Paciente associado ao evento
    std::string procedimento; // Procedimento associado (se aplicável)

public:
    Evento(double _dataHora, int _tipo, Paciente* _paciente, const std::string& _procedimento = "") 
        : dataHora(_dataHora), tipo(_tipo), paciente(_paciente), procedimento(_procedimento) {}

    double getDataHora() const { return dataHora; }
    int getTipo() const { return tipo; }
    Paciente* getPaciente() const { return paciente; }
    std::string getProcedimento() const { return procedimento; }

    // Operador de comparação para o min-heap
    bool operator>(const Evento& outro) const {
        return dataHora > outro.dataHora;
    }
};

class Escalonador {
private:
    std::vector<Evento> eventos;
    double tempoInicial;     // Tempo de referência para conversão
    double tempoAtual;       // Tempo atual da simulação
    int eventosProcessados;  // Estatística: número total de eventos processados

    // Operações do heap
    void subir(int i) {
        while (i > 0) {
            int pai = (i - 1) / 2;
            if (eventos[pai] > eventos[i]) {
                std::swap(eventos[pai], eventos[i]);
                i = pai;
            } else {
                break;
            }
        }
    }

    void descer(int i) {
        int menor;
        int esq = 2 * i + 1;
        int dir = 2 * i + 2;
        int n = eventos.size();

        while (esq < n) {
            if (dir < n && eventos[dir].getDataHora() < eventos[esq].getDataHora()) {
                menor = dir;
            } else {
                menor = esq;
            }

            if (eventos[i] > eventos[menor]) {
                std::swap(eventos[i], eventos[menor]);
                i = menor;
                esq = 2 * i + 1;
                dir = 2 * i + 2;
            } else {
                break;
            }
        }
    }

public:
    // Tipos de eventos
    enum TipoEvento {
        CHEGADA_PACIENTE = 1,
        INICIO_PROCEDIMENTO = 2,
        FIM_PROCEDIMENTO = 3,
        ATUALIZACAO_SISTEMA = 4  // Para atualizar estatísticas periodicamente
    };

    // Construtor
    Escalonador(double _tempoInicial = 0.0) 
        : tempoInicial(_tempoInicial), tempoAtual(_tempoInicial), eventosProcessados(0) {
        eventos.reserve(1000); // Reserva espaço inicial para 1000 eventos
    }

    // Inicializa o escalonador
    void inicializa() {
        eventos.clear();
        tempoAtual = tempoInicial;
        eventosProcessados = 0;
    }

    // Insere um novo evento
    void insereEvento(double dataHora, int tipo, Paciente* paciente, const std::string& procedimento = "") {
        eventos.push_back(Evento(dataHora, tipo, paciente, procedimento));
        subir(eventos.size() - 1);
    }

    // Retira e retorna o próximo evento
    Evento* retiraProximoEvento() {
        if (eventos.empty()) return nullptr;

        Evento* proximo = new Evento(eventos[0]);
        eventos[0] = eventos.back();
        eventos.pop_back();
        
        if (!eventos.empty()) {
            descer(0);
        }

        tempoAtual = proximo->getDataHora();
        eventosProcessados++;
        return proximo;
    }

    // Finaliza o escalonador e retorna estatísticas
    void finaliza(int& totalEventos, double& tempoTotal) {
        totalEventos = eventosProcessados;
        tempoTotal = tempoAtual - tempoInicial;
        eventos.clear();
    }

    // Getters
    double getTempoAtual() const { return tempoAtual; }
    bool vazio() const { return eventos.empty(); }
    int tamanho() const { return eventos.size(); }
};