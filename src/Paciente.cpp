#include <string>

class Paciente {
private:
    // Informações do prontuário
    int id;
    bool alta;
    int ano;
    int mes;
    int dia;
    int hora;
    int grau;  // 0-Verde, 1-Amarelo, 2-Vermelho
    
    // Quantidade de procedimentos necessários
    int medidasHospitalares;
    int testesLaboratorio;
    int examesImagem;
    int instrumentosMedicamentos;
    
    // Estado atual do paciente
    int estadoAtual;
    
    // Tempos e estatísticas
    double tempoChegada;           // Momento que chegou ao hospital
    double tempoUltimaTransicao;   // Última vez que mudou de estado
    double tempoTotalEspera;       // Tempo total em filas
    double tempoTotalAtendimento;  // Tempo total sendo atendido
    double tempoSaida;             // Momento que saiu do hospital
    
    // Histórico de atendimentos usando lista encadeada
    struct RegistroAtendimento {
        std::string procedimento;
        double inicio;
        double fim;
        bool emEspera;  // true se está na fila, false se está sendo atendido
        RegistroAtendimento* proximo;
        
        RegistroAtendimento(const std::string& proc, double ini, bool espera)
            : procedimento(proc), inicio(ini), fim(0), emEspera(espera), proximo(nullptr) {}
    };
    
    RegistroAtendimento* primeiroRegistro;
    RegistroAtendimento* ultimoRegistro;

public:
    // Enumeração dos estados possíveis
    enum Estado {
        NAO_CHEGOU = 1,
        FILA_TRIAGEM = 2,
        SENDO_TRIADO = 3,
        FILA_ATENDIMENTO = 4,
        SENDO_ATENDIDO = 5,
        FILA_MEDIDAS = 6,
        REALIZANDO_MEDIDAS = 7,
        FILA_TESTES = 8,
        REALIZANDO_TESTES = 9,
        FILA_EXAMES = 10,
        REALIZANDO_EXAMES = 11,
        FILA_INSTRUMENTOS = 12,
        RECEBENDO_INSTRUMENTOS = 13,
        ALTA_HOSPITALAR = 14
    };

    // Construtor
    Paciente(int _id, bool _alta, int _ano, int _mes, int _dia, double _hora, 
             int _grau, int _medidas, int _testes, int _exames, int _instrumentos)
        : id(_id), alta(_alta), ano(_ano), mes(_mes), dia(_dia), hora(_hora),
          grau(_grau), medidasHospitalares(_medidas), testesLaboratorio(_testes),
          examesImagem(_exames), instrumentosMedicamentos(_instrumentos),
          estadoAtual(NAO_CHEGOU), tempoChegada(0), tempoUltimaTransicao(0),
          tempoTotalEspera(0), tempoTotalAtendimento(0), tempoSaida(0),
          primeiroRegistro(nullptr), ultimoRegistro(nullptr) {}


    // Construtor Padrão

    Paciente() {}

    // Destrutor
    ~Paciente() {
        RegistroAtendimento* atual = primeiroRegistro;
        while (atual != nullptr) {
            RegistroAtendimento* proximo = atual->proximo;
            delete atual;
            atual = proximo;
        }
    }

    // Adiciona um novo registro ao histórico
    void adicionarRegistro(const std::string& procedimento, double inicio, bool emEspera) {
        RegistroAtendimento* novoRegistro = new RegistroAtendimento(procedimento, inicio, emEspera);
        
        if (primeiroRegistro == nullptr) {
            primeiroRegistro = novoRegistro;
            ultimoRegistro = novoRegistro;
        } else {
            ultimoRegistro->proximo = novoRegistro;
            ultimoRegistro = novoRegistro;
        }
    }

    // Métodos para transição de estado
    void iniciarAtendimento(const std::string& procedimento, double tempoAtual) {
        if (tempoChegada == 0) {
            tempoChegada = tempoAtual;
        }
        
        // Registra fim do estado anterior se estava em espera
        if (ultimoRegistro != nullptr && ultimoRegistro->emEspera) {
            ultimoRegistro->fim = tempoAtual;
            tempoTotalEspera += (tempoAtual - ultimoRegistro->inicio);
        }
        
        // Atualiza estado com base no procedimento
        if (procedimento == "Triagem") estadoAtual = SENDO_TRIADO;
        else if (procedimento == "Atendimento") estadoAtual = SENDO_ATENDIDO;
        else if (procedimento == "Medidas") estadoAtual = REALIZANDO_MEDIDAS;
        else if (procedimento == "Testes") estadoAtual = REALIZANDO_TESTES;
        else if (procedimento == "Imagem") estadoAtual = REALIZANDO_EXAMES;
        else if (procedimento == "Instrumentos/Medicamentos") estadoAtual = RECEBENDO_INSTRUMENTOS;
        
        // Registra novo atendimento
        adicionarRegistro(procedimento, tempoAtual, false);
        tempoUltimaTransicao = tempoAtual;
    }

    void entrarFila(const std::string& procedimento, double tempoAtual) {
        if (tempoChegada == 0) tempoChegada = tempoAtual;
        
        // Registra fim do atendimento anterior se estava sendo atendido
        if (ultimoRegistro != nullptr && !ultimoRegistro->emEspera) {
            ultimoRegistro->fim = tempoAtual;
            tempoTotalAtendimento += (tempoAtual - ultimoRegistro->inicio);
        }
        
        // Atualiza estado com base no procedimento
        if (procedimento == "Triagem") estadoAtual = FILA_TRIAGEM;
        else if (procedimento == "Atendimento") estadoAtual = FILA_ATENDIMENTO;
        else if (procedimento == "Medidas") estadoAtual = FILA_MEDIDAS;
        else if (procedimento == "Testes") estadoAtual = FILA_TESTES;
        else if (procedimento == "Imagem") estadoAtual = FILA_EXAMES;
        else if (procedimento == "Instrumentos/Medicamentos") estadoAtual = FILA_INSTRUMENTOS;
        
        // Registra entrada na fila
        adicionarRegistro(procedimento, tempoAtual, true);
        tempoUltimaTransicao = tempoAtual;
    }

    void finalizarAtendimento(double tempoAtual) {
        // Registra fim do último estado (seja espera ou atendimento)
        if (ultimoRegistro != nullptr) {
            ultimoRegistro->fim = tempoAtual;
            if (ultimoRegistro->emEspera) {
                tempoTotalEspera += (tempoAtual - ultimoRegistro->inicio);
            } else {
                tempoTotalAtendimento += (tempoAtual - ultimoRegistro->inicio);
            }
        }
        
        tempoSaida = tempoAtual;
        estadoAtual = ALTA_HOSPITALAR;
    }

    // Métodos para verificar necessidade de procedimentos
    bool precisaMedidasHospitalares() const { return medidasHospitalares > 0; }
    bool precisaTestesLaboratorio() const { return testesLaboratorio > 0; }
    bool precisaExamesImagem() const { return examesImagem > 0; }
    bool precisaInstrumentosMedicamentos() const { return instrumentosMedicamentos > 0; }
    bool precisaAlta() const { return alta; }

    // Getters
    int getId() const { return id; }
    int getPrioridade() const { return grau; }
    int getEstadoAtual() const { return estadoAtual; }
    double getTempoChegada() const { return tempoChegada; }
    double getTempoSaida() const { return tempoSaida; }
    double getTempoTotalEspera() const { return tempoTotalEspera; }
    double getTempoTotalAtendimento() const { return tempoTotalAtendimento; }
    double getTempoPermanencia() const { return tempoSaida - tempoChegada; }
    
    int getAno() const { return ano; }
    int getMes() const { return mes; }
    int getDia() const { return dia; }
    int getHora() const { return hora; }

    int getQuantidadeMedidasHospitalares() const { return medidasHospitalares; }
    int getQuantidadeTestesLaboratorio() const { return testesLaboratorio; }
    int getQuantidadeExamesImagem() const { return examesImagem; }
    int getQuantidadeInstrumentosMedicamentos() const { return instrumentosMedicamentos; }

    void setEstadoAtual(int novoEstado) { estadoAtual = novoEstado; }

    // Método para decrementar contadores de procedimentos
    void decrementarProcedimento(const std::string& procedimento) {
        if (procedimento == "Medidas") medidasHospitalares--;
        else if (procedimento == "Testes") testesLaboratorio--;
        else if (procedimento == "Imagem") examesImagem--;
        else if (procedimento == "Instrumentos/Medicamentos") instrumentosMedicamentos--;
    }
};