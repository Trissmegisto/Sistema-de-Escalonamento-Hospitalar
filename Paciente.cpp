class Paciente {
    int id, alta, ano, mes, dia, hora, prioridade, numTestesDeLaboratorio, numExamesDeImagem, numMedicamentos;
    int horarioDoAtendimento;
    int tempoNaFila;
    int procedimentos;

    // Tempos
    int tempo_atual, tempo_ocioso, tempo_atendido, data_chegada;

    enum EstadoHospitalar {
        NAO_CHEGOU_AO_HOSPITAL = 1,
        NA_FILA_DE_TRIAGEM = 2,
        SENDO_TRIADO = 3,
        NA_FILA_DE_ATENDIMENTO = 4,
        SENDO_ATENDIDO = 5,
        NA_FILA_DE_MEDIDAS_HOSPITALARES = 6,
        REALIZANDO_MEDIDAS_HOSPITALARES = 7,
        NA_FILA_DE_TESTES_DE_LABORATORIO = 8,
        REALIZANDO_TESTES_DE_LABORATORIO = 9,
        NA_FILA_DE_EXAMES_DE_IMAGEM = 10,
        REALIZANDO_EXAMES_DE_IMAGEM = 11,
        NA_FILA_PARA_INSTRUMENTOS_MEDICAMENTOS = 12,
        SENDO_APLICADOS_INSTRUMENTOS_MEDICAMENTOS = 13,
        ALTA_HOSPITALAR = 14
    };

    void lerDados();
    void getTempo();
    void getId();   
    void getProcedimentos();
    void getAlta();

    void getStatus(); // getStatus inicial sempre será 1 = não chegou ao hospital
    void setStatus();

    Paciente();
    Paciente(int id, int alta, int ano, int mes, int dia, int hora, int prioridade, int numTestesDeLaboratorio, int numExamesDeImagem, int numMedicamentos);
};