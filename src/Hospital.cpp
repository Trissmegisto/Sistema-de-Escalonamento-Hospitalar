#include <iostream>
#include <string>
#include <fstream>

#include "Escalonador.cpp"

using namespace std;

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
        //         << "h, tipo: " << evento->getTipo() << std::endl << std::endl;
        
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
        
        // Só verifica as filas após chegada ou fim de procedimento
        if (evento->getTipo() != Escalonador::INICIO_PROCEDIMENTO) {
            verificaFilasEEscalona(tempoAtual);
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
            // std::cout << "ERRO: Fila de triagem não encontrada!\n";
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
            // Primeiro ocupa a unidade
            proc->ocuparUnidade(unidadeDisponivel, tempoAtual, paciente->getId(), paciente->getEstadoAtual());
            
            // Depois inicia o atendimento do paciente
            paciente->iniciarAtendimento(nomeProcedimento, tempoAtual);
            
            // Agenda o fim do procedimento
            double tempoFim = tempoAtual + proc->getTempoMedio();
            escalonador->insereEvento(tempoFim, Escalonador::FIM_PROCEDIMENTO, paciente, nomeProcedimento);
            
            std::cout << "Iniciando " << nomeProcedimento << " para paciente " << paciente->getId() 
                     << " no tempo " << tempoAtual << std::endl;
        } else {
            std::cout << "Não há unidade disponível para o procedimento " << nomeProcedimento 
                     << " para o paciente " << paciente->getId() 
                     << " no tempo " << tempoAtual << std::endl;
            
            // Se não há unidade disponível, coloca o paciente de volta na fila
            Fila* fila = gerenciadorFilas->getFila(nomeProcedimento, paciente->getPrioridade());
            if (fila && !fila->contemPaciente(paciente)) {
                // Não registrar novo tempo de espera ao retornar para a fila
                fila->enfileira(paciente, tempoAtual);
                
                // Agenda uma nova tentativa após um intervalo
                double proximaTentativa = tempoAtual;
                escalonador->insereEvento(proximaTentativa, Escalonador::INICIO_PROCEDIMENTO, 
                                        paciente, nomeProcedimento);
            }
            
            return;
        }
    }

    void processaFimProcedimento(Paciente* paciente, double tempoAtual, const std::string& nomeProcedimento) {
        std::cout << "Fim do procedimento " << nomeProcedimento 
                  << " para paciente " << paciente->getId()
                  << " no tempo " << tempoAtual 
                  << " (Tempo total atendimento até agora: " << paciente->getTempoTotalAtendimento() 
                  << ", Procedimentos restantes: "
                  << "Medidas=" << paciente->precisaMedidasHospitalares()
                  << ", Testes=" << paciente->precisaTestesLaboratorio()
                  << ", Imagem=" << paciente->precisaExamesImagem()
                  << ", Instrumentos=" << paciente->precisaInstrumentosMedicamentos()
                  << ")" << std::endl;    

        // Decrementa o contador do procedimento realizado
        paciente->decrementarProcedimento(nomeProcedimento);
        
        // Adiciona verificação específica para triagem
        if(nomeProcedimento == "Triagem") {
            encaminhaParaFila(paciente, "Atendimento", tempoAtual);
        }
        // Após atendimento, verifica se precisa alta
        else if(nomeProcedimento == "Atendimento") {
            if(paciente->precisaAlta()) {
                // Garante que o tempo do atendimento seja contabilizado antes da alta
                paciente->finalizarAtendimento(tempoAtual);
                std::cout << "Alta do paciente " << paciente->getId() 
                          << " no tempo " << tempoAtual 
                          << "\nTempo total de atendimento: " << paciente->getTempoTotalAtendimento() 
                          << "\nTempo total de espera: " << paciente->getTempoTotalEspera() 
                          << "\nProcedimentos realizados: Triagem, Atendimento" << std::endl;
                return;
            }
            // Se não precisa alta, continua para o próximo procedimento disponível
            if(paciente->precisaMedidasHospitalares()) {
                encaminhaParaFila(paciente, "Medidas", tempoAtual);
            } else if(paciente->precisaTestesLaboratorio()) {
                encaminhaParaFila(paciente, "Testes", tempoAtual);
            } else if(paciente->precisaExamesImagem()) {
                encaminhaParaFila(paciente, "Imagem", tempoAtual);
            } else if(paciente->precisaInstrumentosMedicamentos()) {
                encaminhaParaFila(paciente, "Instrumentos/Medicamentos", tempoAtual);
            }
        }
        // Para outros procedimentos, verifica se precisa retornar à mesma fila
        else {
            // Verifica se ainda precisa do mesmo procedimento
            if(nomeProcedimento == "Medidas" && paciente->precisaMedidasHospitalares()) {
                encaminhaParaFila(paciente, "Medidas", tempoAtual);
            } else if(nomeProcedimento == "Testes" && paciente->precisaTestesLaboratorio()) {
                encaminhaParaFila(paciente, "Testes", tempoAtual);
            } else if(nomeProcedimento == "Imagem" && paciente->precisaExamesImagem()) {
                encaminhaParaFila(paciente, "Imagem", tempoAtual);
            } else if(nomeProcedimento == "Instrumentos/Medicamentos" && paciente->precisaInstrumentosMedicamentos()) {
                encaminhaParaFila(paciente, "Instrumentos/Medicamentos", tempoAtual);
            }
            // Se não precisa mais do procedimento atual, verifica o próximo
            else if(paciente->precisaMedidasHospitalares()) {
                encaminhaParaFila(paciente, "Medidas", tempoAtual);
            } else if(paciente->precisaTestesLaboratorio()) {
                encaminhaParaFila(paciente, "Testes", tempoAtual);
            } else if(paciente->precisaExamesImagem()) {
                encaminhaParaFila(paciente, "Imagem", tempoAtual);
            } else if(paciente->precisaInstrumentosMedicamentos()) {
                encaminhaParaFila(paciente, "Instrumentos/Medicamentos", tempoAtual);
            } else {
                paciente->finalizarAtendimento(tempoAtual);
                std::cout << "Alta do paciente " << paciente->getId() 
                          << " no tempo " << tempoAtual 
                          << "\nTempo total de atendimento: " << paciente->getTempoTotalAtendimento() 
                          << "\nTempo total de espera: " << paciente->getTempoTotalEspera() 
                          << "\nProcedimentos realizados: " << nomeProcedimento << std::endl;
            }
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
            std::cout << "Escalonando início de " << proc->getNome() 
                    << " para paciente " << paciente->getId() 
                    << " no tempo " << tempoAtual << "\n";
                    
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
            
            // std::cout << "Criando evento de chegada para paciente " << id 
            //         << " no tempo " << paciente->getAnoChegado() << " " << paciente->getMesChegado() << " " << paciente->getDiaChegado() << " " << paciente->getHoraChegada() << "\n";
                    
            escalonador->insereEvento(paciente->getHoraChegada(), Escalonador::CHEGADA_PACIENTE, paciente);
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
            
            processaEvento(evento);
            
            // Atualiza estatísticas
            gerenciadorProcedimentos->atualizarTemposOciosos(escalonador->getTempoAtual());
        }
        
        std::cout << "Simulação finalizada\n";
    }

    std::string formatarTimestamp(long int timestamp) {
        time_t tempo = static_cast<time_t>(timestamp);
        struct tm* tm = localtime(&tempo);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%a %b %-d %H:%M:%S %Y", tm);
        return std::string(buffer);
    }
    
    void geraRelatorio() {
        for (int i = 0; i < pacientes.size(); i++) {
            Paciente* paciente = pacientes[i];
            
            // Get admission time components
            tm admissionTime = {};
            time_t admissionSeconds = (paciente->getHoraChegada()) * 3600 - 71760 + // hours to seconds
                                    paciente->getDiaChegado() * 24 * 3600 + // days to seconds
                                    (paciente->getMesChegado() - 2) * 30 * 24 * 3600 + // months to seconds (approximate)
                                    (paciente->getAnoChegado() + 73) * 365 * 24 * 3600; // years to seconds

            formatarTimestamp(admissionSeconds);
                        
            // Get discharge time
            double totalTime = paciente->getTempoTotalEspera() + paciente->getTempoTotalAtendimento();
            // time_t dischargeSeconds = admissionSeconds + (time_t)(totalTime * 3600);
            // tm dischargeTime = *localtime(&dischargeSeconds);
            
            // // Format day names
            // char admissionDay[4], dischargeDay[4];
            // strftime(admissionDay, sizeof(admissionDay), "%a", &admissionTime);
            // strftime(dischargeDay, sizeof(dischargeDay), "%a", &dischargeTime);
            
            // // Format month names
            // char admissionMonth[4], dischargeMonth[4];
            // strftime(admissionMonth, sizeof(admissionMonth), "%b", &admissionTime);
            // strftime(dischargeMonth, sizeof(dischargeMonth), "%b", &dischargeTime);
            
            // Print formatted output
            cout << paciente->getId() << " " << formatarTimestamp(admissionSeconds) << " " << totalTime << " " << paciente->getTempoTotalAtendimento() << " " <<
            paciente->getTempoTotalEspera() << endl;
        }
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

    std::cout << "Gerando relatório\n";
    simulador.geraRelatorio();

    std::cout << "Programa finalizado\n";
    return 0;
};