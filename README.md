# BC — Black Concessionária

Sistema de Vendas de Veículos

> **Sobre este README:** ele combina dois materiais diferentes do projeto: (1) o **Documento de Visão do Sistema** — com estórias de usuário, modelo de dados planejado, protótipo de telas e requisitos não funcionais — e (2) a **implementação atual em C**, que é um protótipo CLI com dados em memória. Sempre que houver diferença entre o que foi planejado na visão e o que já está implementado, isso é indicado explicitamente, para não dar a entender que o protótipo já faz tudo o que está na visão.

---

## 1. Capa

**Instituição:** _[FICR]_

**Disciplina:** _[LÓGICA]_

**Projeto:** BC – Black Concessionária – Sistema de Vendas de Veículos

**Integrantes:** _[Italo VInicius, Silas Vinicius, Darlan Melo, Breno Matheus, Yasmim Lais, Murilo Pussa]_

**Professor:** _[Jonatas da Silva Junior]_

---

## 2. Documento de Visão do Sistema

**Nome do sistema:** BC – Black Concessionária

**Objetivo:** Vendas de carros e motos usadas e venda de carros importados.

**Público-alvo:** Pessoas de baixa, média e alta classe.

**Problema que o sistema resolve:** resolve diversas necessidades relacionadas à mobilidade, segurança e burocracia na vida de uma pessoa.

---

## 3. Personas / Atores do Sistema

O Documento de Visão não traz personas detalhadas (idade, foto, biografia), mas define claramente **papéis de uso** a partir das estórias de usuário e do público-alvo. Esses papéis funcionam como as personas do sistema:

| Persona / Ator | Quem é | O que faz no sistema | Estórias relacionadas |
|---|---|---|---|
| **Atendente** | Funcionário da concessionária que opera o sistema no dia a dia | Cadastra clientes e veículos, realiza vendas, atualiza e consulta o estoque | HU01, HU02, HU03, HU05 |
| **Cliente comprador** | Pessoa de baixa, média ou alta classe interessada em comprar um carro ou moto (usado ou importado) | Simula financiamento para decidir a forma de pagamento antes de fechar a compra | HU04 |
| **Administrador / Gerente** | Responsável pela gestão da concessionária | Acompanha relatórios de vendas para avaliar o desempenho do negócio | HU06 |

---

## 4. Funcionalidades principais (Visão)

- **Cadastro de clientes**: permitir o registro, edição e exclusão de clientes com validação de dados.
- **Cadastro de produtos (veículos)**: permitir o cadastro, atualização e remoção de veículos, incluindo controle de preço e estoque.
- **Criação de pedidos (vendas)**: possibilitar a criação de vendas vinculadas a um cliente.
- **Controle de itens do pedido**: adicionar, remover e atualizar veículos dentro de uma venda, com cálculo automático do valor total.
- **Acompanhamento de status**: visualizar e atualizar o status dos veículos/pedidos (ex.: disponível, reservado, vendido).

> No domínio da concessionária, "produto" = **veículo** e "pedido" = **venda**. Os nomes genéricos vieram do levantamento inicial e foram especializados nas estórias de usuário (seção 5).

---

## 5. Estórias de usuário (com critérios de aceitação)

### HU01 — Cadastro de Cliente
**Como** atendente, **quero** cadastrar clientes **para** associá-los às vendas.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `id_cliente` | int, PK | auto incremento |
| `nome` | varchar(100) | obrigatório |
| `email` | varchar(150) | único, obrigatório |
| `telefone` | varchar(15) | obrigatório |
| `cpf` | char(11) | único, obrigatório |
| `endereco` | varchar(200) | opcional |

**Regras**
- E-mail deve ser único.
- CPF deve ser único.
- Nome e telefone são obrigatórios.

**Validações**
- E-mail deve conter "@" e formato válido.
- CPF deve conter exatamente 11 dígitos numéricos.
- Telefone deve conter apenas números.

**Comportamento**
- Salvar no banco de dados.
- Exibir mensagem de sucesso.
- Disponibilizar cliente para vínculo em vendas.

---

### HU02 — Cadastro de Veículo
**Como** atendente, **quero** cadastrar veículos **para** disponibilizá-los para venda.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `id_veiculo` | int, PK | auto incremento |
| `tipo` | varchar(10) | obrigatório |
| `marca` | varchar(50) | obrigatório |
| `modelo` | varchar(50) | obrigatório |
| `ano` | int | obrigatório |
| `preco` | decimal(10,2) | obrigatório |
| `quilometragem` | int | obrigatório |
| `status` | varchar(20) | obrigatório |

**Regras**
- Preço deve ser maior que zero.
- Veículo deve iniciar com status "disponível".

**Validações**
- Tipo deve ser "carro" ou "moto".
- Ano deve ser maior que 1900 e menor ou igual ao ano atual.
- Quilometragem não pode ser negativa.

**Comportamento**
- Salvar no banco de dados.
- Disponibilizar veículo para venda.
- Exibir mensagem de confirmação.

---

### HU03 — Realizar Venda
**Como** atendente, **quero** registrar vendas **para** associar clientes aos veículos.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `id_venda` | int, PK | auto incremento |
| `id_cliente` | int, FK | obrigatório |
| `id_veiculo` | int, FK | obrigatório |
| `data_venda` | datetime | obrigatório |
| `valor_total` | decimal(10,2) | obrigatório |
| `forma_pagamento` | varchar(20) | obrigatório |

**Regras**
- Cliente deve existir.
- Veículo deve estar disponível.
- Após a venda, o veículo deve ser marcado como "vendido".

**Validações**
- Forma de pagamento deve ser: à vista, financiamento ou consórcio.
- Valor total deve ser maior que zero.

**Comportamento**
- Registrar venda no banco de dados.
- Atualizar status do veículo.
- Exibir mensagem de sucesso.
- Disponibilizar venda para relatórios.

---

### HU04 — Simular Financiamento
**Como** cliente, **quero** simular um financiamento **para** visualizar o valor das parcelas.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `valor_veiculo` | decimal(10,2) | obrigatório |
| `entrada` | decimal(10,2) | obrigatório |
| `parcelas` | int | obrigatório |
| `taxa_juros` | decimal(5,2) | obrigatório |
| `valor_parcela` | decimal(10,2) | calculado |

**Regras**
- Entrada deve ser menor ou igual ao valor do veículo.
- Número de parcelas deve ser maior que zero.
- Taxa de juros deve ser maior ou igual a zero.

**Validações**
- Valores não podem ser negativos.

**Comportamento**
- Calcular valor da parcela automaticamente.
- Exibir resultado da simulação.
- Permitir nova simulação sem salvar.

---

### HU05 — Controle de Estoque
**Como** atendente, **quero** controlar o status dos veículos **para** gerenciar a disponibilidade.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `id_veiculo` | int, FK | — |
| `status` | varchar(20) | obrigatório |

**Regras**
- Veículo vendido não pode voltar automaticamente para disponível.
- Veículo reservado pode voltar para disponível.

**Validações**
- Status permitido: disponível, vendido, reservado.

**Comportamento**
- Atualizar status automaticamente após venda.
- Listar veículos disponíveis.
- Permitir consulta de todo o estoque.

---

### HU06 — Relatório de Vendas
**Como** administrador, **quero** visualizar relatórios **para** acompanhar o desempenho das vendas.

**Campos**
| Campo | Tipo | Observação |
|---|---|---|
| `data_inicio` | date | obrigatório |
| `data_fim` | date | obrigatório |
| `total_vendas` | decimal(10,2) | calculado |
| `quantidade_vendas` | int | calculado |

**Regras**
- O relatório deve considerar apenas vendas dentro do período informado.

**Validações**
- `data_inicio` deve ser menor ou igual a `data_fim`.

**Comportamento**
- Calcular total de vendas.
- Calcular quantidade de vendas.
- Exibir resultados na tela.

---

## 6. Modelo de dados planejado (banco relacional)

A visão do produto prevê um banco de dados relacional com as seguintes tabelas:

| Tabela | Colunas |
|---|---|
| `CLIENTE` | `id_cliente` (PK), `nome`, `email` (único), `telefone`, `cpf` (único), `endereco` |
| `VEICULO` | `id_veiculo` (PK), `tipo` (carro/moto), `marca`, `modelo`, `ano`, `preco`, `quilometragem`, `status` (disponível/vendido/reservado) |
| `VENDA` | `id_venda` (PK), `id_cliente` (FK), `data_venda`, `valor_total`, `forma_pagamento` |
| `ITEM_VENDA` | `id_item` (PK), `id_venda` (FK), `id_veiculo` (FK) |

> Este é o **modelo planejado** para uma versão com persistência em banco de dados relacional. Na seção 9 está descrito o modelo **efetivamente implementado** no protótipo em C (em memória), que é equivalente em estrutura mas não usa banco de dados.

---

## 7. Protótipo de telas (descrição)

A visão do produto prevê uma interface gráfica (provavelmente web), com as telas abaixo. **O protótipo atual em C é uma CLI (terminal) e não implementa estas telas graficamente** — os mesmos fluxos existem como menus de texto (ver seção 9.3).

- **Tela: Cadastro de Cliente**
  - Campos: Nome, Email, Telefone, CPF, Endereço.
  - Ações: botão "Salvar".
- **Tela: Cadastro de Veículo**
  - Campos: Tipo (carro/moto), Marca, Modelo, Ano, Preço, Quilometragem.
  - Ações: botão "Cadastrar".
- **Tela: Venda**
  - Seleção de cliente.
  - Lista de veículos disponíveis.
  - Campo forma de pagamento.
  - Botão "Adicionar veículo".
  - Lista de veículos adicionados.
  - Total da venda.
  - Botão "Finalizar venda".
- **Tela: Simulação de Financiamento**
  - Campos: Valor do veículo, Entrada, Parcelas, Taxa de juros.
  - Ações: botão "Calcular".
  - Resultado: valor da parcela exibido na tela.
  - Opções: botão "Nova simulação".
- **Tela: Controle de Estoque**
  - Lista de veículos.
  - Filtro por status: disponível, vendido, reservado.
- **Tela: Relatório de Vendas**
  - Campos: Data início, Data fim.
  - Ações: botão "Gerar relatório".
  - Resultado: quantidade de vendas e valor total vendido.

---

## 8. Requisitos não funcionais (Visão)

| Requisito | Descrição | Status no protótipo atual |
|---|---|---|
| Desempenho | Resposta em até 2 segundos | ✅ Atendido (operações em memória são instantâneas) |
| Segurança | Autenticação de usuários, criptografia de senhas e validação de entrada de dados | ⚠️ Parcial — há validação de entrada; **não há** autenticação nem criptografia |
| Usabilidade | Interface simples, intuitiva e responsiva | ⚠️ Parcial — interface é simples e intuitiva, mas em modo texto (CLI), não responsiva |
| Disponibilidade | 99% de uptime durante horário comercial | ➖ Não se aplica a um programa local sem servidor |
| Escalabilidade | Suportar múltiplas vendas e acessos simultâneos | ❌ Não implementado — uso é local, single-user, sem concorrência |
| Integridade | Controle consistente de veículos e status de venda | ✅ Atendido — status do veículo é atualizado automaticamente na venda |
| Confiabilidade | Garantir que vendas sejam concluídas sem perda de dados (uso de transações) | ❌ Não se aplica — não há banco de dados nem transações; dados ficam só em memória |
| Portabilidade | Funcionamento em navegadores modernos (Chrome, Edge, Firefox) | ❌ Não implementado — é uma aplicação de terminal, não web |
| Manutenibilidade | Código organizado e de fácil manutenção | ✅ Atendido — código comentado e dividido em módulos lógicos |
| Persistência | Armazenamento seguro dos dados em banco relacional com possibilidade de backup | ❌ Não implementado — dados existem apenas em memória durante a execução |

---

## 9. Estado atual da implementação (protótipo em C)

Esta seção descreve **o que de fato está implementado** no arquivo `BC.c`, em contraste com a visão descrita acima.

### 9.1 Tecnologia

- Linguagem C (CLI, terminal).
- Bibliotecas padrão: `stdio.h`, `stdlib.h`, `string.h`, `ctype.h`, `time.h`, `math.h`.
- **Sem banco de dados**: as "tabelas" são vetores estáticos de `struct`, com capacidade fixa:

| Macro | Valor | Significado |
|---|---|---|
| `MAX_CLIENTES` | 100 | Capacidade máxima de clientes |
| `MAX_VEICULOS` | 200 | Capacidade máxima de veículos |
| `MAX_VENDAS` | 500 | Capacidade máxima de vendas |
| `MAX_ITENS` | 500 | Capacidade máxima de itens de venda |

### 9.2 Estruturas de dados implementadas

#### `Cliente`
| Campo | Tipo no código | Equivalente na visão |
|---|---|---|
| `id_cliente` | `int` | `id_cliente` (PK) |
| `nome` | `char[100]` | `nome` varchar(100) |
| `email` | `char[150]` | `email` varchar(150), único |
| `telefone` | `char[16]` | `telefone` varchar(15) |
| `cpf` | `char[12]` | `cpf` char(11), único |
| `endereco` | `char[200]` | `endereco` varchar(200) |
| `ativo` | `int` | *(não previsto na visão)* — usado para exclusão lógica (soft delete) |

#### `Veiculo`
| Campo no código | Equivalente na visão |
|---|---|
| `id_veiculo` (`int`) | `id_veiculo` (PK) |
| `tipo` (`char[11]`) | `tipo` varchar(10) |
| `marca` (`char[50]`) | `marca` varchar(50) |
| `modelo` (`char[50]`) | `modelo` varchar(50) |
| `ano` (`int`) | `ano` int |
| `preco` (`double`) | `preco` decimal(10,2) |
| `quilometragem` (`int`) | `quilometragem` int |
| `status` (`char[20]`) | `status` varchar(20) |

#### `Venda`
| Campo no código | Equivalente na visão |
|---|---|
| `id_venda` (`int`) | `id_venda` (PK) |
| `id_cliente` (`int`) | `id_cliente` (FK) |
| `id_veiculo` (`int`) | *(na visão, o veículo fica em `ITEM_VENDA`; no código, a venda já guarda o veículo diretamente, além de também gerar um `ItemVenda`)* |
| `data_venda` (`char[20]`) | `data_venda` datetime |
| `valor_total` (`double`) | `valor_total` decimal(10,2) |
| `forma_pagamento` (`char[20]`) | `forma_pagamento` varchar(20) |

#### `ItemVenda`
| Campo no código | Equivalente na visão |
|---|---|
| `id_item` (`int`) | `id_item` (PK) |
| `id_venda` (`int`) | `id_venda` (FK) |
| `id_veiculo` (`int`) | `id_veiculo` (FK) |

> No código atual, cada venda só tem um veículo (denormalizado direto em `Venda` **e** replicado em `ItemVenda`). A estrutura já comporta evoluir para vendas com múltiplos veículos por venda, como sugerido na visão ("Controle de itens do pedido").

### 9.3 Fluxo de menus implementado (equivalente às telas da visão)

```text
BC - Black Concessionaria
==========================
1. Clientes
   1. Cadastrar cliente
   2. Listar clientes
   3. Editar cliente
   4. Excluir cliente
   0. Voltar
2. Veiculos
   1. Cadastrar veiculo
   2. Listar todos os veiculos
   3. Listar disponiveis
   4. Listar reservados
   5. Listar vendidos
   6. Atualizar status manualmente
   0. Voltar
3. Realizar Venda
4. Simulador de Financiamento
5. Controle de Estoque
   1. Listar todo o estoque
   2. Filtrar por 'disponivel'
   3. Filtrar por 'reservado'
   4. Filtrar por 'vendido'
   5. Alterar status de veiculo
   0. Voltar
6. Relatorio de Vendas
0. Sair
```

### 9.4 Regras de negócio implementadas

**Clientes**
- Nome obrigatório; e-mail validado (`@` + domínio) e único; telefone só com dígitos; CPF com 11 dígitos numéricos e único; endereço livre.
- Exclusão é **lógica** (`ativo = 0`) — preserva o histórico de vendas vinculado ao cliente, em vez de apagar o registro.
- Edição mantém o valor atual se o campo for deixado em branco; e-mail/telefone passam pelas mesmas validações do cadastro.

**Veículos**
- Tipo restrito a `"carro"` ou `"moto"`; marca/modelo obrigatórios; ano entre 1901 e o ano atual; preço > 0; km ≥ 0.
- Todo veículo novo nasce com status `"disponivel"`.
- Atualização manual de status só permite alternar entre `"disponivel"` e `"reservado"`; um veículo `"vendido"` não pode ter o status alterado manualmente (só pela venda).

**Vendas**
1. Seleciona-se um cliente ativo existente.
2. Só veículos `"disponivel"` aparecem para escolha.
3. Forma de pagamento restrita a `"a vista"`, `"financiamento"` ou `"consorcio"`.
4. `valor_total` = preço atual do veículo.
5. Ao confirmar: grava `Venda`, grava `ItemVenda`, marca o veículo como `"vendido"`.
6. Limite de `MAX_VENDAS` (500) vendas por execução.

**Simulador de financiamento**
- Valor do veículo > 0; entrada ≥ 0 e ≤ valor do veículo; parcelas > 0; taxa de juros ≥ 0 (0% permitido).
- Sem juros: `valor_parcela = valor_financiado / parcelas`.
- Com juros, Tabela Price:
  ```text
  i = taxa_juros / 100
  PMT = PV * i / (1 - (1 + i)^-n)
  ```
  onde `PV = valor_veiculo - entrada` e `n` = número de parcelas.
- `total_pago = entrada + (valor_parcela * parcelas)`.
- Não grava nada — é só cálculo, podendo repetir a simulação.

**Controle de estoque**
- Reaproveita a listagem de veículos com filtro por status (todos, disponível, reservado, vendido) e permite, no mesmo submenu, acionar a atualização manual de status.

**Relatório de vendas**
- Solicita data início/fim (`dd/mm/aaaa`), valida formato e garante `fim >= início`.
- Converte as datas para inteiro `AAAAMMDD` para comparação numérica.
- Lista as vendas no período (ID, data, cliente, veículo, valor, forma de pagamento) e exibe o total de vendas e o valor total vendido no período.

### 9.5 Dados de exemplo pré-carregados

| Entidade | ID | Dados |
|---|---|---|
| Cliente | 1 | Ana Lima, ana@email.com, 81999990001, CPF 12345678901, Rua das Flores, 10 |
| Veículo | 1 | Carro, Toyota Corolla, 2020, R$ 85.000,00, 42.000 km, disponível |
| Veículo | 2 | Moto, Honda CB500, 2022, R$ 28.000,00, 8.000 km, disponível |

---

## 10. Rastreabilidade: Visão x Implementação

| Estória / Requisito da visão | Implementado no código atual? | Observações |
|---|---|---|
| HU01 — Cadastro de Cliente | ✅ Sim | Inclui também edição e exclusão lógica, não previstas explicitamente na estória mas presentes no código |
| HU02 — Cadastro de Veículo | ✅ Sim | Validações de tipo, ano, preço e km implementadas conforme especificado |
| HU03 — Realizar Venda | ✅ Sim | Fluxo completo: seleção de cliente, veículo disponível, forma de pagamento, baixa de estoque |
| HU04 — Simular Financiamento | ✅ Sim | Inclui cálculo com Tabela Price (juros compostos), não detalhado na fórmula da estória original |
| HU05 — Controle de Estoque | ✅ Sim | Regra "vendido não volta para disponível automaticamente" é respeitada (não há rota de edição manual para veículo vendido) |
| HU06 — Relatório de Vendas | ✅ Sim | Validação `data_inicio <= data_fim` implementada |
| Persistência em banco de dados | ❌ Não | Dados em memória (vetores estáticos); perdidos ao encerrar o programa |
| Telas gráficas / web | ❌ Não | Interface é CLI (texto no terminal) |
| Autenticação de usuários | ❌ Não | Não há login nem controle de acesso |
| Múltiplos veículos por venda | ⚠️ Parcial | Estrutura (`ItemVenda`) já permite, mas o fluxo atual só registra 1 veículo por venda |
| Edição/remoção de veículos | ⚠️ Parcial | Há atualização de status, mas não há edição completa nem exclusão lógica de veículo (diferente do cliente) |

---

## 11. Metodologia de levantamento de requisitos

O levantamento de requisitos foi conduzido por meio de uma série de perguntas direcionadas, simulando a perspectiva do cliente/stakeholder do sistema.

### Prompts utilizados

1. "Estamos pensando em criar um sistema para uma concessionária de carros e motos. Você pode descrever como se fosse o cliente qual seria o nome do sistema, o objetivo, quem usaria e qual problema ele resolveria?"
2. "Considerando esse sistema de concessionária, o que ele precisaria ter no geral para funcionar bem no dia a dia?"
3. "Pensando no cadastro de clientes, quais informações você acha que seriam necessárias guardar? E que tipo de regras o sistema deveria ter pra evitar erros?"
4. "Agora sobre os veículos: quais dados deveriam ser cadastrados? E o que o sistema precisa validar antes de salvar essas informações?"
5. "Como você imagina que deveria funcionar o processo de venda dentro do sistema? Que regras seriam importantes garantir nesse momento?"
6. "No caso de financiamento, como deveria funcionar a simulação? Quais informações o sistema precisaria e o que ele deveria calcular?"
7. "Sobre o controle de estoque, como o sistema deveria organizar os veículos disponíveis? O que deveria acontecer quando um veículo fosse vendido?"
8. "Pensando na parte administrativa, como você acha que os relatórios de vendas deveriam funcionar? O que seria importante mostrar?"
9. "Se você tivesse que imaginar os dados sendo organizados, quais informações deveriam existir no banco de dados? E como elas deveriam se relacionar?"
10. "Por fim, que telas você acha que o sistema deveria ter? E o que cada uma precisaria mostrar ou permitir fazer?"

### Dificuldades encontradas

- Respostas muito amplas no início.
- Falta de padronização nas respostas.
- Pouco detalhamento técnico em alguns pontos.

### Ajustes realizados

- Refinamento das perguntas para maior clareza.
- Direcionamento por contexto (concessionária).
- Inclusão de foco em regras de negócio e validações.
- Estruturação das perguntas por etapas do sistema.

---

## 12. Compilação e execução (implementação atual)

O simulador de financiamento usa `pow()` (de `math.h`), por isso é necessário vincular a biblioteca matemática.

**Linux/macOS**
```bash
gcc BC.c -o bc -lm
./bc
```

**Windows (MinGW/gcc)**
```bash
gcc BC.c -o bc.exe -lm
bc.exe
```

---

## 13. Limitações atuais

- Dados existem apenas em memória — perdidos ao encerrar o programa.
- Capacidade fixa por macro (`MAX_CLIENTES`, `MAX_VEICULOS`, `MAX_VENDAS`, `MAX_ITENS`).
- Sem autenticação, sem criptografia, sem controle de acesso.
- Interface em modo texto (CLI), sem as telas gráficas previstas na visão.
- Sem banco de dados, sem transações, sem suporte a múltiplos usuários simultâneos.
- Apenas 1 veículo por venda no fluxo atual (apesar da estrutura já suportar múltiplos).
- Veículo não possui exclusão lógica nem edição completa (diferente do cliente).

---

## 14. Roadmap sugerido (para aproximar a implementação da visão)

1. Migrar o armazenamento de vetores em memória para um banco de dados relacional (`CLIENTE`, `VEICULO`, `VENDA`, `ITEM_VENDA`, conforme seção 6).
2. Construir as telas descritas na seção 7 (web ou desktop), substituindo a CLI.
3. Implementar autenticação de usuários e criptografia de senha (atendentes/administradores).
4. Suportar múltiplos veículos por venda usando `ITEM_VENDA` como tabela de associação real (N:N entre venda e veículo).
5. Adicionar edição e exclusão lógica de veículos (hoje só clientes têm exclusão lógica).
6. Implementar transações no banco para garantir confiabilidade nas vendas (atender ao RNF de "Confiabilidade" da visão).
7. Adicionar testes automatizados para as validações (e-mail, CPF, telefone) e para o cálculo da Tabela Price.

---

## 15. Conclusão

O **Documento de Visão** definiu, a partir de um levantamento estruturado de requisitos (com personas implícitas nos papéis de atendente, cliente e administrador), o escopo completo do **BC – Black Concessionária**: cadastro de clientes e veículos, realização de vendas, simulação de financiamento, controle de estoque e relatórios — sustentados por um modelo de dados relacional e por telas dedicadas para cada fluxo.

A **implementação atual em C** cobre, em forma de protótipo CLI com dados em memória, todas as seis estórias de usuário centrais (HU01 a HU06), incluindo suas regras de negócio e validações principais. As lacunas em relação à visão completa — persistência em banco de dados, interface gráfica, autenticação e suporte multiusuário — estão documentadas na seção 10 (Rastreabilidade) e organizadas como roadmap na seção 14, servindo de guia para a evolução do projeto além do protótipo atual.