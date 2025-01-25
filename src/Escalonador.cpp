#include <iostream>
#include "Fila.cpp"

class Evento {
private:
    double dataHora;
    int tipo;
    Paciente* paciente;
    std::string procedimento;

public:
    Evento(double _dataHora, int tipo, Paciente* _paciente, const std::string& _procedimento = "") 
        : dataHora(_dataHora), tipo(tipo), paciente(_paciente), procedimento(_procedimento) {}

    double getDataHora() const { return dataHora; }
    int getTipo() const { return tipo; }
    Paciente* getPaciente() const { return paciente; }
    std::string getProcedimento() const { return procedimento; }

    bool operator>(const Evento& outro) const {
        return dataHora > outro.dataHora;
    }
};

class EventoArray {           
private:
    Evento** data;
    int capacity;
    int size_;

public:
    EventoArray(int initialCapacity = 1000) : capacity(initialCapacity), size_(0) {
        data = new Evento*[capacity];
    }

    ~EventoArray() {
        for (int i = 0; i < size_; i++) {
            delete data[i];
        }
        delete[] data;
    }

    void push_back(const Evento& elemento) {
        if (size_ == capacity) {
            int newCapacity = capacity * 2;
            Evento** newData = new Evento*[newCapacity];
            
            for (int i = 0; i < size_; i++) {
                newData[i] = data[i];
            }
            
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
        
        data[size_] = new Evento(elemento);
        size_++;
    }

    Evento& operator[](int index) {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return *data[index];
    }

    void pop_back() {
        if (size_ > 0) {
            delete data[size_ - 1];
            size_--;
        }
    }

    Evento& back() {
        if (size_ == 0) {
            throw std::out_of_range("Array is empty");
        }
        return *data[size_ - 1];
    }

    void clear() {
        for (int i = 0; i < size_; i++) {
            delete data[i];
        }
        size_ = 0;
    }

    bool empty() const {
        return size_ == 0;
    }

    int size() const {
        return size_;
    }

    Evento* get_pointer(int index) {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[index];
    }

    void swap(int i, int j) {
        Evento* temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    void remove_at(int index) {
        if (index < 0 || index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        delete data[index];
        data[index] = data[size_ - 1];
        size_--;
    }

    void decrease_size() {
        if (size_ > 0) {
            size_--;
        }
    }
};

class Escalonador {
private:
    EventoArray eventos;
    double tempoInicial;
    double tempoAtual;
    int eventosProcessados;

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
    enum TipoEvento {
        CHEGADA_PACIENTE = 1,
        INICIO_PROCEDIMENTO = 2,
        FIM_PROCEDIMENTO = 3,
        ATUALIZACAO_SISTEMA = 4
    };

    Escalonador(double _tempoInicial = 0.0) 
        : tempoInicial(_tempoInicial), tempoAtual(_tempoInicial), eventosProcessados(0) {
    }

    void inicializa() {
        std::cout << "Inicializando escalonador. Eventos antes: " << eventos.size() << "\n";
        eventos.clear();
        tempoAtual = tempoInicial;
        eventosProcessados = 0;
        std::cout << "Eventos depois de inicializar: " << eventos.size() << "\n";
    }

    void insereEvento(double dataHora, int tipo, Paciente* paciente, const std::string& procedimento = "") {
        eventos.push_back(Evento(dataHora, tipo, paciente, procedimento));
        subir(eventos.size() - 1);
    }

    Evento* retiraProximoEvento() {
        if (eventos.empty()) return nullptr;

        Evento* proximo = eventos.get_pointer(0);
        
        if (eventos.size() > 1) {
            eventos.swap(0, eventos.size() - 1);
        }
        
        eventos.decrease_size();
        
        if (!eventos.empty()) {
            descer(0);
        }

        tempoAtual = proximo->getDataHora();
        eventosProcessados++;
        return proximo;
    }

    void finaliza(int& totalEventos, double& tempoTotal) {
        totalEventos = eventosProcessados;
        tempoTotal = tempoAtual - tempoInicial;
        eventos.clear();
    }

    double getTempoAtual() const { return tempoAtual; }
    bool vazio() const { return eventos.empty(); }
    int tamanho() const { return eventos.size(); }
};