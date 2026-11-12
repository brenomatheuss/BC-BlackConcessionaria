// ============================================================
// BC - Black Concessionaria
// Sistema de Vendas de Veiculos (CLI em C, dados em memoria)
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

// Capacidade maxima de cada "tabela" em memoria
#define MAX_CLIENTES   100
#define MAX_VEICULOS   200
#define MAX_VENDAS     500
#define MAX_ITENS      500

// Cadastro de cliente
typedef struct {
    int    id_cliente;
    char   nome[100];
    char   email[150];
    char   telefone[16];
    char   cpf[12];
    char   endereco[200];
    int    ativo;           // 1 = ativo, 0 = removido (soft delete)
} Cliente;

// Cadastro de veiculo (carro ou moto)
typedef struct {
    int    id_veiculo;
    char   tipo[11];        // "carro" ou "moto"
    char   marca[50];
    char   modelo[50];
    int    ano;
    double preco;
    int    quilometragem;
    char   status[20];      // "disponivel", "vendido" ou "reservado"
} Veiculo;

// Registro de uma venda concluida
typedef struct {
    int    id_venda;
    int    id_cliente;
    int    id_veiculo;
    char   data_venda[20];
    double valor_total;
    char   forma_pagamento[20];
} Venda;

// Item ligado a uma venda (relaciona venda -> veiculo)
typedef struct {
    int id_item;
    int id_venda;
    int id_veiculo;
} ItemVenda;

// "Banco de dados" em memoria: vetores fixos que guardam todos os registros
static Cliente   clientes[MAX_CLIENTES];
static Veiculo   veiculos[MAX_VEICULOS];
static Venda     vendas[MAX_VENDAS];
static ItemVenda itens[MAX_ITENS];

// Quantidade de registros validos atualmente armazenados em cada vetor
static int total_clientes = 0;
static int total_veiculos = 0;
static int total_vendas   = 0;
static int total_itens    = 0;

// Contadores usados para gerar o proximo ID de cada entidade
static int proximo_id_cliente = 1;
static int proximo_id_veiculo = 1;
static int proximo_id_venda   = 1;
static int proximo_id_item    = 1;

// ------------------------------------------------------------
// Funcoes utilitarias de entrada/saida e data/hora
// ------------------------------------------------------------

// Descarta os caracteres restantes no buffer de entrada (stdin)
// até encontrar uma quebra de linha ou o fim do arquivo.
// Usado para "limpar" o Enter deixado depois de leituras com scanf-like.
void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Exibe "prompt" e le uma linha de texto do usuario para "destino".
// Remove o '\n' final que o fgets deixa, se houver.
void ler_string(const char *prompt, char *destino, int tamanho) {
    printf("%s", prompt);
    fgets(destino, tamanho, stdin);
    size_t len = strlen(destino);
    if (len > 0 && destino[len - 1] == '\n')
        destino[len - 1] = '\0';
}

// Exibe "prompt" e le um numero inteiro, validando a entrada.
// Repete a pergunta enquanto o usuario nao digitar um inteiro valido.
int ler_inteiro(const char *prompt) {
    int valor;
    char buf[32];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin)) {
            if (sscanf(buf, "%d", &valor) == 1)
                return valor;
        }
        printf("  [!] Entrada invalida. Digite um numero inteiro.\n");
    }
}

// Exibe "prompt" e le um numero decimal (double), validando a entrada.
// Repete a pergunta enquanto o usuario nao digitar um numero valido.
double ler_double(const char *prompt) {
    double valor;
    char buf[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin)) {
            if (sscanf(buf, "%lf", &valor) == 1)
                return valor;
        }
        printf(" [!] Entrada invalida. Digite um numero decimal.\n");
    }
}

// Formata a data/hora atual do sistema como "dd/mm/aaaa hh:mm"
// e grava o resultado em "destino".
void data_hora_atual(char *destino, int tamanho) {
    time_t agora = time(NULL);
    struct tm *t  = localtime(&agora);
    snprintf(destino, tamanho, "%02d/%02d/%04d %02d:%02d",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min);
}

// Retorna o ano atual do sistema (usado para validar o ano do veiculo).
int ano_atual(void) {
    time_t agora = time(NULL);
    struct tm *t  = localtime(&agora);
    return t->tm_year + 1900;
}

// Converte uma string para minusculas, alterando-a no proprio lugar.
void para_minusculo(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

// Imprime uma linha separadora usada para organizar visualmente o menu.
void separador(void) {
    printf("\n============================================================\n");
}

// Pausa a execucao ate o usuario pressionar Enter,
// dando tempo de ler a tela antes de voltar ao menu.
void pausar(void) {
    printf("\nPressione Enter para continuar...");
    limpar_buffer();
}

// ------------------------------------------------------------
// Funcoes de validacao de dados de entrada
// ------------------------------------------------------------

// Verifica se o email tem um '@' seguido, em algum ponto, de um '.'
// (validacao simples de formato, nao garante que o email exista).
int validar_email(const char *email) {
    const char *arroba = strchr(email, '@');
    if (!arroba) return 0;
    const char *ponto = strchr(arroba, '.');
    if (!ponto || ponto == arroba + 1) return 0;
    return 1;
}

// Verifica se o CPF possui exatamente 11 caracteres, todos digitos.
int validar_cpf(const char *cpf) {
    if (strlen(cpf) != 11) return 0;
    for (int i = 0; i < 11; i++)
        if (!isdigit((unsigned char)cpf[i])) return 0;
    return 1;
}

// Verifica se o telefone nao esta vazio e contem apenas digitos.
int validar_telefone(const char *tel) {
    if (strlen(tel) == 0) return 0;
    for (size_t i = 0; i < strlen(tel); i++)
        if (!isdigit((unsigned char)tel[i])) return 0;
    return 1;
}

// Confere se "email" ja esta em uso por outro cliente ativo.
// "ignorar_id" permite ignorar o proprio cliente ao editar seus dados.
int email_unico(const char *email, int ignorar_id) {
    for (int i = 0; i < total_clientes; i++) {
        if (!clientes[i].ativo) continue;
        if (clientes[i].id_cliente == ignorar_id) continue;
        if (strcmp(clientes[i].email, email) == 0) return 0;
    }
    return 1;
}

// Confere se "cpf" ja esta em uso por outro cliente ativo.
// "ignorar_id" permite ignorar o proprio cliente ao editar seus dados.
int cpf_unico(const char *cpf, int ignorar_id) {
    for (int i = 0; i < total_clientes; i++) {
        if (!clientes[i].ativo) continue;
        if (clientes[i].id_cliente == ignorar_id) continue;
        if (strcmp(clientes[i].cpf, cpf) == 0) return 0;
    }
    return 1;
}

// ------------------------------------------------------------
// Funcoes de busca (percorrem os vetores em memoria)
// ------------------------------------------------------------

// Procura um cliente ATIVO pelo ID. Retorna o indice no vetor "clientes"
// ou -1 se nao encontrar (ou se o cliente estiver inativo/removido).
int buscar_cliente_por_id(int id) {
    for (int i = 0; i < total_clientes; i++)
        if (clientes[i].id_cliente == id && clientes[i].ativo)
            return i;
    return -1;
}

// Procura um veiculo pelo ID. Retorna o indice no vetor "veiculos"
// ou -1 se nao encontrar.
int buscar_veiculo_por_id(int id) {
    for (int i = 0; i < total_veiculos; i++)
        if (veiculos[i].id_veiculo == id)
            return i;
    return -1;
}

// ============================================================
// MODULO 1 - Cadastro de Clientes
// ============================================================

// Cadastra um novo cliente: pede nome, email, telefone, CPF e endereco,
// validando cada campo (obrigatoriedade, formato e unicidade) antes de
// aceitar. Ao final, salva o cliente no vetor e gera um novo ID.
void cadastrar_cliente(void) {
    separador();
    printf("  CADASTRO DE CLIENTE\n");
    separador();

    // Impede cadastro se o vetor de clientes ja estiver cheio
    if (total_clientes >= MAX_CLIENTES) {
        printf("  [!] Limite de clientes atingido.\n");
        pausar();
        return;
    }

    Cliente c;
    memset(&c, 0, sizeof(c));
    c.id_cliente = proximo_id_cliente;
    c.ativo = 1;

    // Nome: obrigatorio, repete a pergunta enquanto estiver vazio
    do {
        ler_string("  Nome        : ", c.nome, sizeof(c.nome));
        if (strlen(c.nome) == 0)
            printf("  [!] Nome e obrigatorio.\n");
    } while (strlen(c.nome) == 0);

    // Email: precisa ter formato valido e ainda nao estar cadastrado
    do {
        ler_string("  Email       : ", c.email, sizeof(c.email));
        if (!validar_email(c.email))
            printf("  [!] Email invalido. Deve conter '@' e dominio valido.\n");
        else if (!email_unico(c.email, -1))
            printf("  [!] Email ja cadastrado.\n");
        else break;
    } while (1);

    // Telefone: precisa conter apenas digitos
    do {
        ler_string("  Telefone    : ", c.telefone, sizeof(c.telefone));
        if (!validar_telefone(c.telefone))
            printf("  [!] Telefone deve conter apenas numeros.\n");
        else break;
    } while (1);

    // CPF: precisa ter 11 digitos numericos e ainda nao estar cadastrado
    do {
        ler_string("  CPF (11 dig): ", c.cpf, sizeof(c.cpf));
        if (!validar_cpf(c.cpf))
            printf("  [!] CPF invalido. Deve ter exatamente 11 digitos numericos.\n");
        else if (!cpf_unico(c.cpf, -1))
            printf("  [!] CPF ja cadastrado.\n");
        else break;
    } while (1);

    // Endereco: campo livre, sem validacao
    ler_string("  Endereco    : ", c.endereco, sizeof(c.endereco));

    // Persiste o novo cliente no vetor e avanca o contador de IDs
    clientes[total_clientes++] = c;
    proximo_id_cliente++;

    printf("\n  [OK] Cliente '%s' cadastrado com ID #%d.\n", c.nome, c.id_cliente);
    pausar();
}

// Lista todos os clientes ativos em formato de tabela.
// Clientes marcados como inativos (excluidos) nao aparecem.
void listar_clientes(void) {
    separador();
    printf(" \n");
    printf("  LISTA DE CLIENTES\n");
    separador();
    printf(" \n");

    // Cabecalho da tabela
    printf("  %-4s  %-30s  %-20s  %-15s\n", "ID", "Nome", "Email", "Telefone");
    printf("  %-4s  %-30s  %-20s  %-15s\n", "----", "------------------------------",
           "--------------------", "---------------");

    // Percorre o vetor de clientes imprimindo apenas os ativos
    int encontrou = 0;
    for (int i = 0; i < total_clientes; i++) {
        if (!clientes[i].ativo) continue;
        printf("  %-4d  %-30s  %-20s  %-15s\n",
               clientes[i].id_cliente,
               clientes[i].nome,
               clientes[i].email,
               clientes[i].telefone);
        encontrou = 1;
    }
    if (!encontrou) printf("  Nenhum cliente cadastrado.\n");
    pausar();
}

// Edita os dados de um cliente existente. Para cada campo, mostra o
// valor atual entre colchetes; se o usuario apenas pressionar Enter
// (linha vazia), o valor atual e mantido. Email e telefone passam
// pelas mesmas validacoes do cadastro.
void editar_cliente(void) {
    separador();
    printf("  EDITAR CLIENTE\n");
    separador();

    // Localiza o cliente pelo ID informado
    int id = ler_inteiro("  ID do cliente: ");
    int idx = buscar_cliente_por_id(id);
    if (idx < 0) {
        printf("  [!] Cliente nao encontrado.\n");
        pausar();
        return;
    }
    Cliente *c = &clientes[idx];
    printf("  Editando: %s\n", c->nome);
    printf("  (Deixe em branco para manter o valor atual)\n\n");

    char buf[200];

    // Nome: se digitar algo, substitui; se deixar vazio, mantem o atual
    printf("  Nome atual [%s]: ", c->nome);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strncpy(c->nome, buf, sizeof(c->nome) - 1);

    // Email: valida formato e unicidade antes de aceitar a alteracao
    do {
        printf("  Email atual [%s]: ", c->email);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) break;
        if (!validar_email(buf)) { printf("  [!] Email invalido.\n"); continue; }
        if (!email_unico(buf, c->id_cliente)) { printf("  [!] Email ja em uso.\n"); continue; }
        strncpy(c->email, buf, sizeof(c->email) - 1);
        break;
    } while (1);

    // Telefone: valida formato antes de aceitar a alteracao
    do {
        printf("  Telefone atual [%s]: ", c->telefone);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) break;
        if (!validar_telefone(buf)) { printf("  [!] Telefone invalido.\n"); continue; }
        strncpy(c->telefone, buf, sizeof(c->telefone) - 1);
        break;
    } while (1);

    // Endereco: campo livre, sem validacao
    printf("  Endereco atual [%s]: ", c->endereco);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strncpy(c->endereco, buf, sizeof(c->endereco) - 1);

    printf("\n  [OK] Cliente atualizado com sucesso.\n");
    pausar();
}

// Remove um cliente do sistema (exclusao logica/soft delete):
// apenas marca "ativo = 0", sem apagar o registro do vetor.
void excluir_cliente(void) {
    separador();
    printf("  EXCLUIR CLIENTE\n");
    separador();
    int id = ler_inteiro("  ID do cliente: ");
    int idx = buscar_cliente_por_id(id);
    if (idx < 0) {
        printf("  [!] Cliente nao encontrado.\n");
        pausar();
        return;
    }
    clientes[idx].ativo = 0;
    printf("  [OK] Cliente #%d removido.\n", id);
    pausar();
}

// Exibe o submenu de Clientes e direciona para a opcao escolhida
// (cadastrar, listar, editar, excluir) at o usuario optar por voltar.
void menu_clientes(void) {
    int op;
    do {
        separador();
        printf(" \n");
        printf("  MENU - Clientes\n");
        separador();
        printf(" \n");
        printf("  1. Cadastrar cliente\n");
        printf("  2. Listar clientes\n");
        printf("  3. Editar cliente\n");
        printf("  4. Excluir cliente\n");
        printf("  0. Voltar\n");
        separador();
        op = ler_inteiro("  Opcao: ");
        switch (op) {
            case 1: cadastrar_cliente(); break;
            case 2: listar_clientes();   break;
            case 3: editar_cliente();    break;
            case 4: excluir_cliente();   break;
            case 0: break;
            default: printf("  [!] Opcao invalida.\n");
        }
    } while (op != 0);
}

// ============================================================
// MODULO 2 - Cadastro de Veiculos
// ============================================================

// Cadastra um novo veiculo (carro ou moto): pede tipo, marca, modelo,
// ano, preco e quilometragem, validando cada campo. O veiculo novo
// sempre comeca com status "disponivel".
void cadastrar_veiculo(void) {
    separador();
    printf("  CADASTRO DE VEICULO\n");
    separador();

    // Impede cadastro se o vetor de veiculos ja estiver cheio
    if (total_veiculos >= MAX_VEICULOS) {
        printf("  [!] Limite de veiculos atingido.\n");
        pausar();
        return;
    }

    Veiculo v;
    memset(&v, 0, sizeof(v));
    v.id_veiculo = proximo_id_veiculo;
    strcpy(v.status, "disponivel");

    // Tipo: aceita apenas "carro" ou "moto" (normalizado para minusculas)
    do {
        ler_string("  Tipo (carro/moto): ", v.tipo, sizeof(v.tipo));
        para_minusculo(v.tipo);
        if (strcmp(v.tipo, "carro") != 0 && strcmp(v.tipo, "moto") != 0)
            printf("  [!] Tipo deve ser 'carro' ou 'moto'.\n");
        else break;
    } while (1);

    // Marca: obrigatoria
    do {
        ler_string("  Marca        : ", v.marca, sizeof(v.marca));
        if (strlen(v.marca) == 0) printf("  [!] Marca e obrigatoria.\n");
        else break;
    } while (1);

    // Modelo: obrigatorio
    do {
        ler_string("  Modelo       : ", v.modelo, sizeof(v.modelo));
        if (strlen(v.modelo) == 0) printf("  [!] Modelo e obrigatorio.\n");
        else break;
    } while (1);

    // Ano: precisa estar entre 1901 e o ano atual do sistema
    int ano_max = ano_atual();
    do {
        v.ano = ler_inteiro("  Ano          : ");
        if (v.ano <= 1900 || v.ano > ano_max)
            printf("  [!] Ano deve ser entre 1901 e %d.\n", ano_max);
        else break;
    } while (1);

    // Preco: precisa ser maior que zero
    do {
        v.preco = ler_double("  Preco (R$)   : ");
        if (v.preco <= 0) printf("  [!] Preco deve ser maior que zero.\n");
        else break;
    } while (1);

    // Quilometragem: nao pode ser negativa
    do {
        v.quilometragem = ler_inteiro("  Quilometragem: ");
        if (v.quilometragem < 0) printf("  [!] Quilometragem nao pode ser negativa.\n");
        else break;
    } while (1);

    // Persiste o novo veiculo no vetor e avanca o contador de IDs
    veiculos[total_veiculos++] = v;
    proximo_id_veiculo++;

    printf("\n  [OK] Veiculo '%s %s' cadastrado com ID #%d e status 'disponivel'.\n",
           v.marca, v.modelo, v.id_veiculo);
    pausar();
}

// Lista veiculos em formato de tabela. Se "filtro_status" for NULL,
// lista todos; caso contrario, mostra apenas os veiculos cujo status
// seja exatamente igual ao filtro (ex.: "disponivel", "reservado").
void listar_veiculos(const char *filtro_status) {
    separador();
    if (filtro_status)
        printf("  VEICULOS - Status: %s\n", filtro_status);
    else
        printf("  TODOS OS VEICULOS\n");
    separador();

    // Cabecalho da tabela
    printf("  %-4s  %-6s  %-15s  %-15s  %-4s  %12s  %-8s  %-12s\n",
           "ID", "Tipo", "Marca", "Modelo", "Ano", "Preco(R$)", "KM", "Status");
    printf("  %-4s  %-6s  %-15s  %-15s  %-4s  %12s  %-8s  %-12s\n",
           "----", "------", "---------------", "---------------",
           "----", "------------", "--------", "------------");

    // Percorre o vetor de veiculos aplicando o filtro de status, se houver
    int encontrou = 0;
    for (int i = 0; i < total_veiculos; i++) {
        if (filtro_status && strcmp(veiculos[i].status, filtro_status) != 0) continue;
        printf("  %-4d  %-6s  %-15s  %-15s  %-4d  %12.2f  %-8d  %-12s\n",
               veiculos[i].id_veiculo,
               veiculos[i].tipo,
               veiculos[i].marca,
               veiculos[i].modelo,
               veiculos[i].ano,
               veiculos[i].preco,
               veiculos[i].quilometragem,
               veiculos[i].status);
        encontrou = 1;
    }
    if (!encontrou) printf("  Nenhum veiculo encontrado.\n");
    pausar();
}

// Altera manualmente o status de um veiculo entre "disponivel" e
// "reservado". Veiculos com status "vendido" nao podem ter o status
// alterado por aqui (a venda e o unico caminho que define "vendido").
void atualizar_status_veiculo(void) {
    separador();
    printf("  ATUALIZAR STATUS DE VEICULO\n");
    separador();
    int id = ler_inteiro("  ID do veiculo: ");
    int idx = buscar_veiculo_por_id(id);
    if (idx < 0) {
        printf("  [!] Veiculo nao encontrado.\n");
        pausar();
        return;
    }
    Veiculo *v = &veiculos[idx];
    printf("  Status atual: %s\n", v->status);

    // Bloqueia alteracao se o veiculo ja foi vendido
    if (strcmp(v->status, "vendido") == 0) {
        printf("  [!] Veiculo vendido nao pode ter status alterado automaticamente.\n");
        pausar();
        return;
    }

    // So aceita "disponivel" ou "reservado" como novo status
    char novo[20];
    do {
        ler_string("  Novo status (disponivel/reservado): ", novo, sizeof(novo));
        para_minusculo(novo);
        if (strcmp(novo, "disponivel") != 0 && strcmp(novo, "reservado") != 0)
            printf("  [!] Status invalido.\n");
        else break;
    } while (1);

    strncpy(v->status, novo, sizeof(v->status) - 1);
    printf("  [OK] Status atualizado para '%s'.\n", v->status);
    pausar();
}

// Exibe o submenu de Veiculos e direciona para a opcao escolhida
// (cadastrar, listar com diferentes filtros, atualizar status)
// at o usuario optar por voltar.
void menu_veiculos(void) {
    int op;
    do {
        separador();
        printf(" \n");
        printf("  MENU - Veiculos\n");
        separador();
        printf(" \n");
        printf("  1. Cadastrar veiculo\n");
        printf("  2. Listar todos os veiculos\n");
        printf("  3. Listar disponiveis\n");
        printf("  4. Listar reservados\n");
        printf("  5. Listar vendidos\n");
        printf("  6. Atualizar status manualmente\n");
        printf("  0. Voltar\n");
        separador();
        op = ler_inteiro("  Opcao: ");
        switch (op) {
            case 1: cadastrar_veiculo();                     break;
            case 2: listar_veiculos(NULL);                   break;
            case 3: listar_veiculos("disponivel");           break;
            case 4: listar_veiculos("reservado");            break;
            case 5: listar_veiculos("vendido");              break;
            case 6: atualizar_status_veiculo();              break;
            case 0: break;
            default: printf("  [!] Opcao invalida.\n");
        }
    } while (op != 0);
}

// ============================================================
// MODULO 3 - Realizar Venda
// ============================================================

// Conduz o fluxo completo de uma venda:
//   1) escolhe um cliente existente;
//   2) mostra apenas os veiculos disponiveis e escolhe um;
//   3) escolhe a forma de pagamento;
//   4) registra a Venda e o ItemVenda correspondente;
//   5) marca o veiculo vendido como "vendido".
void realizar_venda(void) {
    separador();
    printf(" \n");
    printf("  REALIZAR VENDA\n");
    separador();

    // Impede a venda se o vetor de vendas ja estiver cheio
    if (total_vendas >= MAX_VENDAS) {
        printf("  [!] Limite de vendas atingido.\n");
        pausar();
        return;
    }

    // Passo 1: selecionar o cliente
    listar_clientes();
    int id_cli = ler_inteiro("  ID do cliente: ");
    int idx_cli = buscar_cliente_por_id(id_cli);
    if (idx_cli < 0) {
        printf("  [!] Cliente nao encontrado.\n");
        pausar();
        return;
    }

    // Passo 2: mostrar e selecionar um veiculo disponivel
    separador();
    printf(" \n");
    printf("  VEICULOS DISPONIVEIS\n");
    separador();
    printf("  %-4s  %-6s  %-15s  %-15s  %-4s  %12s\n",
           "ID", "Tipo", "Marca", "Modelo", "Ano", "Preco(R$)");
    printf("  %-4s  %-6s  %-15s  %-15s  %-4s  %12s\n",
           "----","------","---------------","---------------","----","------------");

    // Lista somente os veiculos com status "disponivel"
    int tem_disponivel = 0;
    for (int i = 0; i < total_veiculos; i++) {
        if (strcmp(veiculos[i].status, "disponivel") == 0) {
            printf("  %-4d  %-6s  %-15s  %-15s  %-4d  %12.2f\n",
                   veiculos[i].id_veiculo, veiculos[i].tipo,
                   veiculos[i].marca, veiculos[i].modelo,
                   veiculos[i].ano, veiculos[i].preco);
            tem_disponivel = 1;
        }
    }
    if (!tem_disponivel) {
        printf("  Nenhum veiculo disponivel no momento.\n");
        pausar();
        return;
    }

    int id_vei = ler_inteiro("  ID do veiculo: ");
    int idx_vei = buscar_veiculo_por_id(id_vei);
    if (idx_vei < 0) {
        printf("  [!] Veiculo nao encontrado.\n");
        pausar();
        return;
    }
    // Garante que o veiculo escolhido continua disponivel
    if (strcmp(veiculos[idx_vei].status, "disponivel") != 0) {
        printf("  [!] Veiculo nao esta disponivel para venda.\n");
        pausar();
        return;
    }

    // Passo 3: forma de pagamento (apenas 3 opcoes aceitas)
    char forma[20];
    do {
        ler_string("  Forma de pagamento (a vista/financiamento/consorcio): ",
                   forma, sizeof(forma));
        para_minusculo(forma);
        if (strcmp(forma, "a vista")      != 0 &&
            strcmp(forma, "financiamento") != 0 &&
            strcmp(forma, "consorcio")     != 0)
            printf("  [!] Forma invalida. Use: a vista, financiamento ou consorcio.\n");
        else break;
    } while (1);

    double valor = veiculos[idx_vei].preco;
    if (valor <= 0) {
        printf("  [!] Valor invalido.\n");
        pausar();
        return;
    }

    // Passo 4: monta e salva o registro de Venda
    Venda venda;
    memset(&venda, 0, sizeof(venda));
    venda.id_venda    = proximo_id_venda;
    venda.id_cliente  = id_cli;
    venda.id_veiculo  = id_vei;
    venda.valor_total = valor;
    data_hora_atual(venda.data_venda, sizeof(venda.data_venda));
    strncpy(venda.forma_pagamento, forma, sizeof(venda.forma_pagamento) - 1);

    vendas[total_vendas++] = venda;

    // Cria o ItemVenda que liga a venda ao veiculo vendido
    ItemVenda item;
    item.id_item    = proximo_id_item;
    item.id_venda   = proximo_id_venda;
    item.id_veiculo = id_vei;
    itens[total_itens++] = item;
    proximo_id_item++;

    // Passo 5: marca o veiculo como vendido para sair da lista de disponiveis
    strcpy(veiculos[idx_vei].status, "vendido");

    proximo_id_venda++;

    // Exibe o resumo da venda concluida
    separador();
    printf(" \n");
    printf("  [OK] VENDA REGISTRADA COM SUCESSO!\n");
    printf("  Venda ID  : #%d\n",  venda.id_venda);
    printf("  Cliente   : %s\n",   clientes[idx_cli].nome);
    printf("  Veiculo   : %s %s\n",veiculos[idx_vei].marca, veiculos[idx_vei].modelo);
    printf("  Data      : %s\n",   venda.data_venda);
    printf("  Valor     : R$ %.2f\n", venda.valor_total);
    printf("  Pagamento : %s\n",   venda.forma_pagamento);
    separador();
    pausar();
}

// ============================================================
// MODULO 4 - Simulador de Financiamento
// ============================================================

// Simula um financiamento a partir do valor do veiculo, entrada,
// numero de parcelas e taxa de juros mensal. Se a taxa for 0%, divide
// o saldo igualmente entre as parcelas; caso contrario, usa a Tabela
// Price (sistema de prestacoes fixas) para calcular o valor da parcela.
// Ao final, pergunta se o usuario quer simular novamente.
void simular_financiamento(void) {
    int continuar = 1;
    while (continuar) {
        separador();
        printf(" \n");
        printf("  SIMULADOR DE FINANCIAMENTO\n");
        separador();

        double valor_veiculo, entrada, taxa_juros;
        int parcelas;

        // Valor do veiculo: precisa ser positivo
        do {
            printf(" \n");
            valor_veiculo = ler_double("  Valor do veiculo (R$): ");
            if (valor_veiculo <= 0) printf("  [!] Valor deve ser positivo.\n");
            else break;
        } while (1);

        // Entrada: nao pode ser negativa nem maior que o valor do veiculo
        do {
            entrada = ler_double("  Valor de entrada (R$): ");
            if (entrada < 0)
                printf("  [!] Entrada nao pode ser negativa.\n");
            else if (entrada > valor_veiculo)
                printf("  [!] Entrada nao pode ser maior que o valor do veiculo.\n");
            else break;
        } while (1);

        // Numero de parcelas: precisa ser maior que zero
        do {
            parcelas = ler_inteiro("  Numero de parcelas : ");
            if (parcelas <= 0) printf("  [!] Parcelas deve ser maior que zero.\n");
            else break;
        } while (1);

        // Taxa de juros mensal: nao pode ser negativa (0% e permitido)
        do {
            taxa_juros = ler_double("  Taxa de juros mensal (%): ");
            if (taxa_juros < 0) printf("  [!] Taxa nao pode ser negativa.\n");
            else break;
        } while (1);

        double valor_financiado = valor_veiculo - entrada;
        double valor_parcela;

        if (taxa_juros == 0.0) {
            // Sem juros: apenas divide o valor financiado pelas parcelas
            valor_parcela = valor_financiado / parcelas;
        } else {
            // Com juros: aplica a Formula Price: PMT = PV * i / (1 - (1+i)^-n)
            double i = taxa_juros / 100.0;
            valor_parcela = valor_financiado * i / (1.0 - pow(1.0 + i, -parcelas));
        }

        double total_pago = entrada + valor_parcela * parcelas;

        // Exibe o resultado da simulacao
        separador();
        printf(" \n");
        printf("  RESULTADO DA SIMULACAO\n");
        separador();
        printf(" \n");
        printf("  Valor do veiculo  : R$ %12.2f\n", valor_veiculo);
        printf("  Entrada           : R$ %12.2f\n", entrada);
        printf("  Valor financiado  : R$ %12.2f\n", valor_financiado);
        printf("  Taxa mensal       : %.2f%%\n",     taxa_juros);
        printf("  Parcelas          : %d x R$ %.2f\n", parcelas, valor_parcela);
        printf("  Total a pagar     : R$ %12.2f\n", total_pago);
        separador();

        // Pergunta se o usuario quer fazer outra simulacao
        char resp[4];
        ler_string("  Realizar nova simulacao? (s/n): ", resp, sizeof(resp));
        para_minusculo(resp);
        continuar = (resp[0] == 's');
    }
}

// ============================================================
// MODULO 5 - Controle de Estoque
// ============================================================

// Exibe o submenu de Estoque: permite listar todo o estoque ou
// filtrar por status, alem de redirecionar para a atualizacao manual
// de status. Reaproveita as funcoes do modulo de Veiculos.
void menu_estoque(void) {
    int op;
    do {
        separador();
        printf(" \n");
        printf("  CONTROLE DE ESTOQUE\n");
        separador();
        printf(" \n");
        printf("  1. Listar todo o estoque\n");
        printf("  2. Filtrar por 'disponivel'\n");
        printf("  3. Filtrar por 'reservado'\n");
        printf("  4. Filtrar por 'vendido'\n");
        printf("  5. Alterar status de veiculo\n");
        printf("  0. Voltar\n");
        separador();
        op = ler_inteiro("  Opcao: ");
        switch (op) {
            case 1: listar_veiculos(NULL);         break;
            case 2: listar_veiculos("disponivel"); break;
            case 3: listar_veiculos("reservado");  break;
            case 4: listar_veiculos("vendido");    break;
            case 5: atualizar_status_veiculo();    break;
            case 0: break;
            default: printf("  [!] Opcao invalida.\n");
        }
    } while (op != 0);
}

// ============================================================
// MODULO 6 - Relatorio de Vendas
// ============================================================

// Converte uma data no formato "dd/mm/aaaa..." para um inteiro
// no formato AAAAMMDD, o que permite comparar datas com simples
// operadores numericos (<, >) em vez de comparar strings.
static int data_para_int(const char *data_str) {
    int d, m, a;
    sscanf(data_str, "%d/%d/%d", &d, &m, &a);
    return a * 10000 + m * 100 + d;
}

// Gera um relatorio de vendas dentro de um periodo informado pelo
// usuario (data inicio e data fim). Mostra cada venda do periodo em
// formato de tabela e, ao final, um resumo com quantidade de vendas
// e o valor total vendido.
void gerar_relatorio(void) {
    separador();
    printf(" \n");
    printf("  RELATORIO DE VENDAS\n");
    separador();

    char inicio_str[12], fim_str[12];
    int inicio_int, fim_int;

    // Le e valida a data de inicio do periodo (formato dd/mm/aaaa)
    do {
        printf(" \n");
        ler_string("  Data inicio (dd/mm/aaaa): ", inicio_str, sizeof(inicio_str));
        int d, m, a;
        if (sscanf(inicio_str, "%d/%d/%d", &d, &m, &a) != 3 || m < 1 || m > 12 || d < 1 || d > 31)
            printf("  [!] Data invalida.\n");
        else { inicio_int = a * 10000 + m * 100 + d; break; }
    } while (1);

    // Le e valida a data de fim do periodo, garantindo que seja >= inicio
    do {
        ler_string("  Data fim    (dd/mm/aaaa): ", fim_str, sizeof(fim_str));
        int d, m, a;
        if (sscanf(fim_str, "%d/%d/%d", &d, &m, &a) != 3 || m < 1 || m > 12 || d < 1 || d > 31)
            printf("  [!] Data invalida.\n");
        else {
            fim_int = a * 10000 + m * 100 + d;
            if (fim_int < inicio_int)
                printf("  [!] Data fim deve ser maior ou igual a data inicio.\n");
            else break;
        }
    } while (1);

    int    qtd_vendas   = 0;
    double total_vendas = 0.0;

    // Cabecalho da tabela de detalhamento
    separador();
    printf("  Detalhamento:\n");
    printf("  %-6s  %-20s  %-25s  %-15s  %12s  %-15s\n",
           "ID", "Data", "Cliente", "Veiculo", "Valor(R$)", "Pagamento");
    printf("  %-6s  %-20s  %-25s  %-15s  %12s  %-15s\n",
           "------","--------------------","-------------------------",
           "---------------","------------","---------------");

    // Percorre todas as vendas, filtrando pelo periodo informado
    for (int i = 0; i < total_vendas; i++) {
        int data_venda_int = data_para_int(vendas[i].data_venda);
        if (data_venda_int < inicio_int || data_venda_int > fim_int) continue;

        // Busca nome do cliente e descricao do veiculo para exibir na linha
        int idx_cli = buscar_cliente_por_id(vendas[i].id_cliente);
        int idx_vei = buscar_veiculo_por_id(vendas[i].id_veiculo);

        char nome_cli[26] = "?";
        char nome_vei[16] = "?";
        if (idx_cli >= 0) strncpy(nome_cli, clientes[idx_cli].nome, 25);
        if (idx_vei >= 0) snprintf(nome_vei, sizeof(nome_vei), "%s %s",
                                   veiculos[idx_vei].marca, veiculos[idx_vei].modelo);

        printf("  %-6d  %-20s  %-25s  %-15s  %12.2f  %-15s\n",
               vendas[i].id_venda,
               vendas[i].data_venda,
               nome_cli,
               nome_vei,
               vendas[i].valor_total,
               vendas[i].forma_pagamento);

        // Acumula estatisticas do periodo
        qtd_vendas++;
        total_vendas += vendas[i].valor_total;
    }

    // Exibe o resumo final do periodo filtrado
    separador();
    printf("  Periodo       : %s a %s\n", inicio_str, fim_str);
    printf("  Qtd. de vendas: %d\n",      qtd_vendas);
    printf("  Total vendido : R$ %.2f\n", total_vendas);
    separador();
    pausar();
}

// ============================================================
// Menu Principal
// ============================================================

// Imprime o cabecalho fixo exibido no topo do menu principal.
void exibir_cabecalho(void) {
    separador();
    printf(" \n");
    printf("   BC - Black Concessionaria\n");
    printf("   Sistema de Vendas de Veiculos\n");
    separador();
}

int main(void) {
    int op;

    // Pre-carrega alguns dados de exemplo (1 cliente e 2 veiculos)
    // apenas para facilitar testes manuais do programa.
    {
        Cliente c = {1, "Ana Lima", "ana@email.com", "81999990001",
                     "12345678901", "Rua das Flores, 10", 1};
        clientes[total_clientes++] = c;
        proximo_id_cliente = 2;

        Veiculo v1 = {1, "carro", "Toyota", "Corolla", 2020,
                      85000.00, 42000, "disponivel"};
        veiculos[total_veiculos++] = v1;

        Veiculo v2 = {2, "moto", "Honda", "CB500", 2022,
                      28000.00, 8000, "disponivel"};
        veiculos[total_veiculos++] = v2;

        proximo_id_veiculo = 3;
    }

    // Loop principal: exibe o menu e despacha para o modulo escolhido
    // ate o usuario digitar a opcao 0 (Sair).
    do {
        exibir_cabecalho();
        printf(" \n");
        printf("  1. Clientes\n");
        printf("  2. Veiculos\n");
        printf("  3. Realizar Venda\n");
        printf("  4. Simulador de Financiamento\n");
        printf("  5. Controle de Estoque\n");
        printf("  6. Relatorio de Vendas\n");
        printf("  0. Sair\n");
        separador();
        op = ler_inteiro("  Opcao: ");

        switch (op) {
            case 1: menu_clientes();        break;
            case 2: menu_veiculos();        break;
            case 3: realizar_venda();       break;
            case 4: simular_financiamento();break;
            case 5: menu_estoque();         break;
            case 6: gerar_relatorio();      break;
            case 0:
                printf("\n  Encerrando sistema. Ate logo!\n\n");
                break;
            default:
                printf("  [!] Opcao invalida.\n");
        }
    } while (op != 0);

    return 0;
}