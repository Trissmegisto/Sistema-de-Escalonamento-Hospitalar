#include <string>

class Procedimento {
private:
    std::string nome;              
    double tempoMedio;             
    int numeroUnidades;            
    
    struct Unidade {
        bool ocupada;
        double tempoOcupadoAte;    
        double tempoTotalOcupado;
        double tempoTotalOcioso;
        int pacienteId;            // ID do paciente que está ocupando a unidade
        int estadoAnterior;        // Estado anterior do paciente
        int estadoAtual;           // Estado atual do paciente
        
        Unidade() : ocupada(false), tempoOcupadoAte(0), tempoTotalOcupado(0), 
                   tempoTotalOcioso(0), pacienteId(-1), estadoAnterior(0), estadoAtual(0) {}
    };
    
    Unidade* unidades;             

public:
    // Enums para mapear estados do paciente aos procedimentos
    enum EstadoProcedimento {
        TRIAGEM = 3,                    // SENDO_TRIADO
        ATENDIMENTO = 5,                // SENDO_ATENDIDO
        MEDIDAS = 7,                    // REALIZANDO_MEDIDAS_HOSPITALARES
        TESTES = 9,                     // REALIZANDO_TESTES_DE_LABORATORIO
        IMAGEM = 11,                    // REALIZANDO_EXAMES_DE_IMAGEM
        INSTRUMENTOS = 13               // SENDO_APLICADOS_INSTRUMENTOS_MEDICAMENTOS
    };

    // Construtor
    Procedimento(const std::string& nome, double tempoMedio, int numeroUnidades) 
        : nome(nome), tempoMedio(tempoMedio), numeroUnidades(numeroUnidades) {
        unidades = new Unidade[numeroUnidades];
    }
    
    // Destrutor
    ~Procedimento() {
        delete[] unidades;
    }
    
    // Verifica se existe alguma unidade disponível no tempo atual
    bool temUnidadeDisponivel(double tempoAtual) const {
        for(int i = 0; i < numeroUnidades; i++) {
            if(!unidades[i].ocupada || tempoAtual >= unidades[i].tempoOcupadoAte) {
                return true;
            }
        }
        return false;
    }
    
    // Retorna o índice da primeira unidade disponível ou -1 se não houver
    int getUnidadeDisponivel(double tempoAtual) const {
        for(int i = 0; i < numeroUnidades; i++) {
            if(!unidades[i].ocupada || tempoAtual >= unidades[i].tempoOcupadoAte) {
                return i;
            }
        }
        return -1;
    }
    
    // Ocupa uma unidade com um paciente específico
    bool ocuparUnidade(int indiceUnidade, double tempoAtual, int pacienteId, int estadoAnterior) {
        if(indiceUnidade >= 0 && indiceUnidade < numeroUnidades) {
            if(!unidades[indiceUnidade].ocupada || tempoAtual >= unidades[indiceUnidade].tempoOcupadoAte) {
                unidades[indiceUnidade].ocupada = true;
                unidades[indiceUnidade].tempoOcupadoAte = tempoAtual + tempoMedio;
                unidades[indiceUnidade].pacienteId = pacienteId;
                unidades[indiceUnidade].estadoAnterior = estadoAnterior;
                unidades[indiceUnidade].estadoAtual = getEstadoProcedimento();
                return true;
            }
        }
        return false;
    }
    
    // Libera uma unidade e retorna o ID do paciente que estava nela
    int liberarUnidade(int indiceUnidade, double tempoAtual) {
        if(indiceUnidade >= 0 && indiceUnidade < numeroUnidades) {
            if(unidades[indiceUnidade].ocupada) {
                double tempoOcupado = tempoAtual - (unidades[indiceUnidade].tempoOcupadoAte - tempoMedio);
                unidades[indiceUnidade].tempoTotalOcupado += tempoOcupado;
                int pacienteId = unidades[indiceUnidade].pacienteId;
                unidades[indiceUnidade].ocupada = false;
                unidades[indiceUnidade].pacienteId = -1;
                return pacienteId;
            }
        }
        return -1;
    }
    
    // Atualiza os tempos ociosos baseado no tempo atual
    void atualizarTemposOciosos(double tempoAtual) {
        for(int i = 0; i < numeroUnidades; i++) {
            if(!unidades[i].ocupada || tempoAtual >= unidades[i].tempoOcupadoAte) {
                unidades[i].tempoTotalOcioso += 1.0/60.0; // Adiciona 1 minuto convertido para hora
            }
        }
    }
    
    // Obtém o estado do procedimento baseado no nome
    int getEstadoProcedimento() const {
        if(nome == "Triagem") return TRIAGEM;
        if(nome == "Atendimento") return ATENDIMENTO;
        if(nome == "Medidas") return MEDIDAS;
        if(nome == "Testes") return TESTES;
        if(nome == "Imagem") return IMAGEM;
        if(nome == "Instrumentos/Medicamentos") return INSTRUMENTOS;
        return 0;
    }
    
    // Getters básicos
    std::string getNome() const { return nome; }
    double getTempoMedio() const { return tempoMedio; }
    int getNumeroUnidades() const { return numeroUnidades; }
    
    // Obter estatísticas de uma unidade
    bool getEstadoUnidade(int indice, double& tempoOcupado, double& tempoOcioso) const {
        if(indice >= 0 && indice < numeroUnidades) {
            tempoOcupado = unidades[indice].tempoTotalOcupado;
            tempoOcioso = unidades[indice].tempoTotalOcioso;
            return true;
        }
        return false;
    }
    
    // Verifica se uma unidade está ocupada no tempo atual
    bool isUnidadeOcupada(int indice, double tempoAtual) const {
        if(indice >= 0 && indice < numeroUnidades) {
            return unidades[indice].ocupada && tempoAtual < unidades[indice].tempoOcupadoAte;
        }
        return false;
    }
    
    // Obtém o tempo até quando a unidade está ocupada
    double getTempoOcupadoAte(int indice) const {
        if(indice >= 0 && indice < numeroUnidades) {
            return unidades[indice].tempoOcupadoAte;
        }
        return 0;
    }

    // Obtém o ID do paciente em uma unidade
    int getPacienteId(int indiceUnidade) const {
        if(indiceUnidade >= 0 && indiceUnidade < numeroUnidades) {
            return unidades[indiceUnidade].pacienteId;
        }
        return -1;
    }

    // Obtém o estado anterior do paciente em uma unidade
    int getEstadoAnterior(int indiceUnidade) const {
        if(indiceUnidade >= 0 && indiceUnidade < numeroUnidades) {
            return unidades[indiceUnidade].estadoAnterior;
        }
        return 0;
    }

    // Obtém o estado atual do paciente em uma unidade
    int getEstadoAtual(int indiceUnidade) const {
        if(indiceUnidade >= 0 && indiceUnidade < numeroUnidades) {
            return unidades[indiceUnidade].estadoAtual;
        }
        return 0;
    }

    // Retorna o índice da unidade ocupada por um paciente específico
    int getUnidadeOcupada(int pacienteId) const {
        for(int i = 0; i < numeroUnidades; i++) {
            if(unidades[i].ocupada && unidades[i].pacienteId == pacienteId) {
                return i;
            }
        }
        return -1;
    }
};

class GerenciadorProcedimentos {
private:
    Procedimento** procedimentos;   
    int numProcedimentos;          

public:
    // Construtor
    GerenciadorProcedimentos() : numProcedimentos(6) {
        procedimentos = new Procedimento*[numProcedimentos];
        procedimentos[0] = new Procedimento("Triagem", 0, 0);
        procedimentos[1] = new Procedimento("Atendimento", 0, 0);
        procedimentos[2] = new Procedimento("Medidas", 0, 0);
        procedimentos[3] = new Procedimento("Testes", 0, 0);
        procedimentos[4] = new Procedimento("Imagem", 0, 0);
        procedimentos[5] = new Procedimento("Instrumentos/Medicamentos", 0, 0);
    }
    
    // Destrutor
    ~GerenciadorProcedimentos() {
        for(int i = 0; i < numProcedimentos; i++) {
            delete procedimentos[i];
        }
        delete[] procedimentos;
    }
    
    // Define o número de unidades para um procedimento
    void setNumeroUnidades(const std::string& nome, double tempo, int numeroUnidades) {
        for(int i = 0; i < numProcedimentos; i++) {
            if(procedimentos[i]->getNome() == nome) {
                Procedimento* novo = new Procedimento(nome, tempo, numeroUnidades);
                delete procedimentos[i];
                procedimentos[i] = novo;
                break;
            }
        }
    }
    
    // Obtém um procedimento pelo nome
    Procedimento* getProcedimento(const std::string& nome) {
        for(int i = 0; i < numProcedimentos; i++) {
            if(procedimentos[i]->getNome() == nome) {
                return procedimentos[i];
            }
        }
        return nullptr;
    }
    
    // Atualiza os tempos ociosos de todos os procedimentos
    void atualizarTemposOciosos(double tempoAtual) {
        for(int i = 0; i < numProcedimentos; i++) {
            procedimentos[i]->atualizarTemposOciosos(tempoAtual);
        }
    }
};