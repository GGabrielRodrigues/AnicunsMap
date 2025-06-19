# AnicunsMap - Sistema de Navegação Dijkstra

## 1. Visão Geral do Projeto

Este projeto implementa um sistema de navegação baseado no algoritmo de Dijkstra para encontrar o menor caminho entre dois pontos em um grafo. O sistema é composto por um backend em C para processamento de dados geográficos (arquivos OSM e POLY) e cálculo de rotas, e um frontend em Python com PySide6 para a interface gráfica do usuário.

## 2. Requisitos (Funcionais e Não Funcionais)

### 2.1 Requisitos Funcionais (RFs)

* **RF01 - Carregamento de Mapa:** O sistema deve permitir o carregamento de arquivos OpenStreetMap (.osm), convertê-los internamente para o formato .poly e construir o grafo correspondente.
* **RF02 - Visualização do Grafo:** O usuário deve ser capaz de visualizar o grafo carregado (vértices e arestas) na interface gráfica.
* **RF03 - Seleção de Origem e Destino:** O usuário deve ser capaz de selecionar vértices de origem e destino diretamente no mapa.
* **RF04 - Cálculo de Menor Caminho:** O sistema deve calcular e exibir o menor caminho entre a origem e o destino selecionados utilizando o algoritmo de Dijkstra.
* **RF05 - Edição do Grafo:** O sistema deve permitir a adição e remoção de vértices e arestas do grafo.
* **RF06 - Informações do Caminho:** O sistema deve exibir informações detalhadas sobre o caminho encontrado (distância total, número de vértices, sequência de vértices).
* **RF07 - Estatísticas de Execução:** O sistema deve exibir estatísticas sobre o tempo de processamento e o número de nós explorados pelo algoritmo de Dijkstra.
* **RF08 - Exportação de Imagem:** O sistema deve permitir copiar a imagem do mapa para a área de transferência.
* **RF09 - Controles de Visualização:** O usuário deve poder ajustar o zoom, a cor do grafo e o tamanho dos pontos, e optar por numerar vértices e rotular arestas.

### 2.2 Requisitos Não Funcionais (RNFs)

* **RNF01 - Usabilidade:** A interface do usuário deve ser intuitiva e fácil de usar.
* **RNF02 - Desempenho:** O algoritmo de Dijkstra deve ser eficiente para mapas de tamanho médio (até X vértices/arestas), com tempo de resposta aceitável (ex: menos de 5 segundos para cálculo de rota em um mapa de 10.000 vértices).
* **RNF03 - Portabilidade:** O sistema deve ser executável em sistemas operacionais Linux e Windows.
* **RNF04 - Confiabilidade:** O sistema deve lidar robustamente com arquivos de entrada inválidos ou corrompidos, exibindo mensagens de erro claras.
* **RNF05 - Escalabilidade:** O backend em C deve ser capaz de processar grafos maiores com aumento proporcional no tempo de processamento.
* **RNF06 - Segurança:** Não aplicável para este projeto, pois não lida com dados sensíveis ou acesso remoto.
* **RNF07 - Manutenibilidade:** O código deve ser modular e bem documentado para facilitar futuras modificações e depurações. (Este é o RNF08 que você mencionou)
* **RNF08 - Modularidade e Documentação:** O código deve ser estruturado em módulos lógicos com documentação interna clara e abrangente (comentários, docstrings).
* **RNF09 - Extensibilidade:** O design deve permitir a adição de novos algoritmos de busca de caminho ou funcionalidades de visualização com impacto mínimo no código existente.

## 3. Arquitetura do Sistema

O sistema adota uma arquitetura cliente-servidor simplificada, onde o frontend (Python) atua como cliente e o backend (C) como "servidor" de lógica de grafo e algoritmos. A comunicação entre eles é feita através de `ctypes`, que permite ao Python chamar funções e acessar estruturas de dados da biblioteca C.

**Componentes Principais:**

* **Backend C (`backend_c/`):**
    * `integracao.h`: Definições de estruturas de dados (Nós, Vias, Vértices, Arestas, Heap, etc.) e protótipos de funções de integração.
    * `integracao.c`: Implementação das funções de parsing de OSM, carregamento de POLY, construção do grafo, algoritmo de Dijkstra, e funções de edição (adição/remoção de vértices/arestas).
    * `main_test.c`: Um programa C simples para testar o backend independentemente da GUI (opcional, mas bom para desenvolvimento).
* **Frontend Python (`frontend_python/`):**
    * `main_gui.py`: Implementa a interface gráfica do usuário usando PySide6. Responsável por interagir com o usuário, visualizar o mapa e chamar as funções do backend C via `ctypes`.
    * `assets/`: Contém recursos como arquivos de exemplo (`exemplo.md`).
* **Scripts de Build/Execução:**
    * `build_and_run.sh`: Script shell para compilar a biblioteca C e iniciar a aplicação Python.

**Diagrama de Componentes (Exemplo - você pode criar um visualmente):**

+-------------------+       +-------------------+
|   Frontend Python |       |     Backend C     |
|   (main_gui.py)   |<----->| (integracao.c/.h) |
|   - PySide6 GUI   |       | - Parsing OSM/POLY|
|   - Interação Usu.|       | - Estruturas Grafo|
|   - Visualização  |       | - Alg. Dijkstra   |
|                   |       | - Edição Grafo    |
+-------------------+       +-------------------+
^                           ^
|                           |
| (ctypes)                  | (Arquivos .osm/.poly)
V                           V
+-------------------+       +-------------------+
|      Usuário      |       |Sistema de Arquivos|
+-------------------+       +-------------------+

## 4. Tecnologias Utilizadas

* **Linguagens de Programação:** C, Python
* **Framework GUI:** PySide6 (Python)
* **Ferramentas de Build:** GCC (para C), `venv` (para Python)
* **Formatos de Dados:** OpenStreetMap (.osm), POLY (formato proprietário para o grafo)

## 5. Estrutura de Pastas

TrabalhoFinalAED2/
├── backend_c/
│   ├── lib/
│   │   └── integracao_lib.so  # Biblioteca C compilada (Linux/macOS) ou integracao_lib.dll (Windows)
│   └── src/
│       ├── integracao.h
│       ├── integracao.c
│       └── main_test.c
├── frontend_python/
│   ├── assets/
│   │   └── exemplo.md
│   ├── venv/                   # Ambiente virtual Python
│   └── main_gui.py
├── data/                       # Diretório para arquivos .osm e .poly
│   └── anicuns.osm
│   └── anicuns.poly
├── build_and_run.sh            # Script para compilar e executar (Linux/macOS)
├── build_and_run.bat           # Script para compilar e executar (Windows)
└── README.md                   # Esta documentação

## 6. Como Compilar e Executar

Para executar o projeto, siga os passos abaixo no seu terminal.

### 6.1 Pré-requisitos

Certifique-se de ter os seguintes softwares instalados em seu sistema:

* **Python 3.x:** Baixe e instale a versão mais recente em [python.org](https://www.python.org/).
* **GCC (GNU Compiler Collection):** Para compilar o código C.
    * **Linux:** Geralmente já vem pré-instalado. Se não, use `sudo apt install build-essential` (Ubuntu/Debian) ou `sudo yum groupinstall "Development Tools"` (Fedora/RHEL).
    * **Windows:** Recomenda-se instalar o [MinGW-w64](https://mingw-w64.org/doku.php/download/mingw-builds) e adicioná-lo ao PATH do sistema.
    * **macOS:** Instale as Xcode Command Line Tools com `xcode-select --install`.

### 6.2 Preparação do Ambiente (Primeira Vez)

Navegue até a pasta raiz do projeto (`TrabalhoFinalAED2/`) no seu terminal.

1.  **Crie e Ative um Ambiente Virtual Python:**
    Um ambiente virtual isola as dependências do projeto.
    ```bash
    cd frontend_python/
    python3 -m venv venv
    ```
    * **Linux/macOS:**
        ```bash
        source venv/bin/activate
        ```
    * **Windows (Command Prompt):**
        ```cmd
        .\venv\Scripts\activate.bat
        ```
    * **Windows (PowerShell):**
        ```powershell
        .\venv\Scripts\Activate.ps1
        ```

2.  **Instale as Dependências Python:**
    Com o ambiente virtual ativado, instale a biblioteca PySide6.
    ```bash
    pip install PySide6
    ```

3.  **Retorne à Pasta Raiz do Projeto:**
    ```bash
    cd ..
    ```

### 6.3 Executando o Programa

Após a preparação inicial, você pode compilar o backend C e executar a interface gráfica.

* **Linux/macOS:**
    Execute o script de shell:
    ```bash
    ./build_and_run.sh
    ```
    * *Nota:* Certifique-se de que o script tenha permissões de execução: `chmod +x build_and_run.sh`.

* **Windows:**
    Execute este arquivo no Command Prompt:
    ```cmd
    build_and_run.bat
    ```

Ao seguir estas instruções, o programa deve ser iniciado e a janela da aplicação de navegação será exibida. O mapa padrão `anicuns.osm` (localizado na pasta `data/`) será carregado automaticamente. Você pode carregar outros arquivos OSM usando o botão "Carregar Outro Arquivo OSM" na interface.

## 7. Autores

* Ana Luisa Gonçalves
* Gabriel Rodrigues
* Julia Nascimento
* Luis Vittor?