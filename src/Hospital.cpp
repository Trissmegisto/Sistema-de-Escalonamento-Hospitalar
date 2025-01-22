#include <iostream>
#include <string>
#include <fstream>

#include "Escalonador.cpp"

using namespace std;

class PacienteArray {
private:
    Paciente** data;
    int capacity;
    int size_;

public:
    PacienteArray(int initialCapacity = 1000) : capacity(initialCapacity), size_(0) {
        data = new Paciente*[capacity];
    }

    ~PacienteArray() {
        for (int i = 0; i < size_; i++) {
            delete data[i];
        }
        delete[] data;
    }

    void push_back(Paciente* paciente) {
        if (size_ == capacity) {
            int newCapacity = capacity * 2;
            Paciente** newData = new Paciente*[newCapacity];
            
            for (int i = 0; i < size_; i++) {
                newData[i] = data[i];
            }
            
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
        
        data[size_] = paciente;
        size_++;
    }

    Paciente* operator[](int index) {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[index];
    }

    int size() const {
        return size_;
    }
};

class Hospital {
private:
    GerenciadorFilas* gerenciadorFilas;
    GerenciadorProcedimentos* gerenciadorProcedimentos;
    Escalonador* escalonador;
    PacienteArray pacientes;

    double relogio;


    struct Config {
        double tempo;
        int unidades;
    };

    void configurarProcedimentos(Config* configs) {
        gerenciadorProcedimentos->setNumeroUnidades("Triagem", configs[0].tempo, configs[0].unidades);
        gerenciadorProcedimentos->setNumeroUnidades("Atendimento", configs[1].tempo, configs[1].unidades);
        gerenciadorProcedimentos->setNumeroUnidades("Medidas", configs[2].tempo, configs[2].unidades);
        gerenciadorProcedimentos->setNumeroUnidades("Testes", configs[3].tempo, configs[3].unidades);
        gerenciadorProcedimentos->setNumeroUnidades("Imagem", configs[4].tempo, configs[4].unidades);
        gerenciadorProcedimentos->setNumeroUnidades("Instrumentos/Medicamentos", configs[5].tempo, configs[5].unidades);
    }

     // Processa um evento do escalonador
    void processaEvento(Evento* evento) {
        Paciente* paciente = evento->getPaciente();
        double tempoAtual = evento->getDataHora();
        
        // std::cout << "Processando evento para paciente " << paciente->getId() 
        //         << " no tempo " << tempoAtual 
        //         << "h, tipo: " << evento->getTipo() << std::endl;
        
        switch(evento->getTipo()) {
            case Escalonador::CHEGADA_PACIENTE:
                // std::cout << "Chegada do paciente " << paciente->getId() << std::endl;
                processaChegada(paciente, tempoAtual);
                break;
                
            case Escalonador::INICIO_PROCEDIMENTO:
                // std::cout << "Início de " << evento->getProcedimento() 
                //         << " para paciente " << paciente->getId() << std::endl;
                processaInicioProcedimento(paciente, tempoAtual, evento->getProcedimento());
                break;
                
            case Escalonador::FIM_PROCEDIMENTO:
                // std::cout << "Fim de " << evento->getProcedimento() 
                //         << " para paciente " << paciente->getId() << std::endl;
                processaFimProcedimento(paciente, tempoAtual, evento->getProcedimento());
                break;
        }
        
        delete evento;
    }

    void processaChegada(Paciente* paciente, double tempoAtual) {
        // std::cout << "Processando chegada do paciente " << paciente->getId() 
        //         << " no tempo " << tempoAtual 
        //         << " com prioridade " << paciente->getPrioridade() << "\n";
        
        // Coloca o paciente na fila de triagem
        Fila* filaTriagem = gerenciadorFilas->getFila("Triagem", paciente->getPrioridade());
        if (filaTriagem == nullptr) {
            std::cout << "ERRO: Fila de triagem não encontrada!\n";
            return;
        }
        
        paciente->entrarFila("Triagem", tempoAtual);
        filaTriagem->enfileira(paciente, tempoAtual);
        // std::cout << "Paciente " << paciente->getId() << " entrou na fila de triagem\n";
        
        // Verifica se pode começar o atendimento imediatamente
        verificaFilasEEscalona(tempoAtual);
    }

    void processaInicioProcedimento(Paciente* paciente, double tempoAtual, const std::string& nomeProcedimento) {
        Procedimento* proc = gerenciadorProcedimentos->getProcedimento(nomeProcedimento);
        int unidadeDisponivel = proc->getUnidadeDisponivel(tempoAtual);
        
        if(unidadeDisponivel >= 0) {
            proc->ocuparUnidade(unidadeDisponivel, tempoAtual, paciente->getId(), paciente->getEstadoAtual());
            paciente->iniciarAtendimento(nomeProcedimento, tempoAtual);
            
            // Agenda o fim do procedimento
            double tempoFim = tempoAtual + proc->getTempoMedio();
            escalonador->insereEvento(tempoFim, Escalonador::FIM_PROCEDIMENTO, paciente, nomeProcedimento);
        }
    }

    void processaFimProcedimento(Paciente* paciente, double tempoAtual, const std::string& nomeProcedimento) {
        // Decrementa o contador do procedimento realizado
        paciente->decrementarProcedimento(nomeProcedimento);
        
        // Verifica próximo procedimento necessário
        if(paciente->precisaMedidasHospitalares()) {
            encaminhaParaFila(paciente, "Medidas", tempoAtual);
        } else if(paciente->precisaTestesLaboratorio()) {
            encaminhaParaFila(paciente, "Testes", tempoAtual);
        } else if(paciente->precisaExamesImagem()) {
            encaminhaParaFila(paciente, "Imagem", tempoAtual);
        } else if(paciente->precisaInstrumentosMedicamentos()) {
            encaminhaParaFila(paciente, "Instrumentos/Medicamentos", tempoAtual);
        } else if(paciente->precisaAlta()) {
            paciente->finalizarAtendimento(tempoAtual);
        }
        
        verificaFilasEEscalona(tempoAtual);
    }

    void encaminhaParaFila(Paciente* paciente, const std::string& procedimento, double tempoAtual) {
        Fila* fila = gerenciadorFilas->getFila(procedimento, paciente->getPrioridade());
        paciente->entrarFila(procedimento, tempoAtual);
        fila->enfileira(paciente, tempoAtual);
    }

    void verificaFilasEEscalona(double tempoAtual) {
        // Verifica todas as filas em ordem de prioridade
        std::string procedimentos[] = {
            "Triagem", "Atendimento", "Medidas", 
            "Testes", "Imagem", "Instrumentos/Medicamentos"
        };
        
        for(const auto& nomeProcedimento : procedimentos) {
            Procedimento* proc = gerenciadorProcedimentos->getProcedimento(nomeProcedimento);
            
            // Para triagem e atendimento, verifica filas por prioridade
            if(nomeProcedimento == "Triagem" || nomeProcedimento == "Atendimento") {
                for(int prioridade = 2; prioridade >= 0; prioridade--) {
                    Fila* fila = gerenciadorFilas->getFila(nomeProcedimento, prioridade);
                    verificaFilaEEscalona(fila, proc, tempoAtual);
                }
            } else {
                Fila* fila = gerenciadorFilas->getFila(nomeProcedimento);
                verificaFilaEEscalona(fila, proc, tempoAtual);
            }
        }
    }

    void verificaFilaEEscalona(Fila* fila, Procedimento* proc, double tempoAtual) {
        if (fila == nullptr || proc == nullptr) {
            std::cout << "ERRO: Fila ou procedimento nulo!\n";
            return;
        }
        
        while(!fila->filaVazia() && proc->temUnidadeDisponivel(tempoAtual)) {
            Paciente* paciente = fila->desenfileira(tempoAtual);
            // std::cout << "Escalonando início de " << proc->getNome() 
            //         << " para paciente " << paciente->getId() 
            //         << " no tempo " << tempoAtual << "\n";
                    
            escalonador->insereEvento(tempoAtual, Escalonador::INICIO_PROCEDIMENTO, 
                                    paciente, proc->getNome());
        }
    }

public:
    Hospital() {
        gerenciadorFilas = new GerenciadorFilas();
        gerenciadorProcedimentos = new GerenciadorProcedimentos();
        escalonador = new Escalonador();
    }
    
    ~Hospital() {
        delete gerenciadorFilas;
        delete gerenciadorProcedimentos;
        delete escalonador;
    }
    
    void carregaArquivo(const std::string& nomeArquivo) {
        std::ifstream arquivo(nomeArquivo);
        if(!arquivo.is_open()) {
            std::cerr << "Erro ao abrir arquivo: " << nomeArquivo << std::endl;
            return;
        }
        
        std::cout << "Arquivo aberto com sucesso\n";
        
        // Lê configurações do hospital
        Config* configuracoes = new Config[6];
        for(int i = 0; i < 6; i++) {
            arquivo >> configuracoes[i].tempo >> configuracoes[i].unidades;
            std::cout << "Configuração " << i << ": tempo=" << configuracoes[i].tempo << ", unidades=" << configuracoes[i].unidades << "\n";
        }
        
        configurarProcedimentos(configuracoes);
        delete[] configuracoes;
        
        int numPacientes;
        arquivo >> numPacientes;
        std::cout << "Número de pacientes a serem lidos: " << numPacientes << "\n";
        
        // Lê todos os pacientes
        for(int i = 0; i < numPacientes; i++) {
            int id, alta, ano, mes, dia, hora, grau;
            int medidas, testes, imagem, instrumentos;
            
            arquivo >> id >> alta >> ano >> mes >> dia >> hora 
                >> grau >> medidas >> testes >> imagem >> instrumentos;
                        
            Paciente* paciente = new Paciente(id, alta, ano, mes, dia, hora,
                                            grau, medidas, testes, imagem, instrumentos);
            
            std::cout << "Criando evento de chegada para paciente " << id 
                    << " no tempo " << paciente->getAno() << " " << paciente->getMes() << " " << paciente->getDia() << " " << paciente->getHora() << "\n";
                    
            escalonador->insereEvento(paciente->getHora(), Escalonador::CHEGADA_PACIENTE, paciente);
            pacientes.push_back(paciente);
        }
        
        std::cout << "Total de eventos após carregar: " << escalonador->tamanho() << "\n";
        
        arquivo.close();
    }
    
    void executaSimulacao() {
        std::cout << "Iniciando simulação\n";

        std::cout << "Número de eventos inicial: " << escalonador->tamanho() << "\n";
        
        while(!escalonador->vazio()) {
            Evento* evento = escalonador->retiraProximoEvento();
            std::cout << "\nProcessando evento: "
                    << "Nome do Evento= " << evento->getProcedimento()
                    << "Tempo=" << evento->getDataHora()
                    << ", Tipo=" << evento->getTipo()
                    << ", Paciente=" << evento->getPaciente()->getId() << "\n";
            
            processaEvento(evento);
            
            // Atualiza estatísticas
            gerenciadorProcedimentos->atualizarTemposOciosos(escalonador->getTempoAtual());
        }
        
        std::cout << "Simulação finalizada\n";
    }
};

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada>" << std::endl;
        return 1;
    }

    std::cout << "Iniciando programa\n";
    Hospital simulador;
    
    std::cout << "Carregando arquivo\n";
    simulador.carregaArquivo(argv[1]);
    
    std::cout << "Executando simulação\n";
    simulador.executaSimulacao();
    
    std::cout << "Programa finalizado\n";
    return 0;
}

