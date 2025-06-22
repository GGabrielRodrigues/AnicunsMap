# AnicunsMap - Sistema de Navegação Dijkstra

---

## 1. Visão Geral do Projeto

O **AnicunsMap** é um sistema de navegação robusto e intuitivo, desenvolvido para encontrar o menor caminho entre dois pontos em um grafo utilizando o eficiente algoritmo de Dijkstra. Este projeto adota uma arquitetura cliente-servidor simplificada, onde o processamento de dados geográficos e o cálculo de rotas são realizados por um backend em **C**, enquanto a interação do usuário e a visualização gráfica são gerenciadas por um frontend em **Python** com a biblioteca **PySide6**. A integração entre as duas camadas é feita de forma transparente através de `ctypes`, permitindo que o Python acesse e utilize as funcionalidades da biblioteca C de forma eficiente.

---

## 2. Requisitos (Funcionais e Não Funcionais)

Este sistema foi projetado para atender a um conjunto específico de requisitos, garantindo tanto a sua funcionalidade quanto a sua qualidade.

### 2.1 Requisitos Funcionais (RFs)

Os requisitos funcionais descrevem o que o sistema deve fazer.

* **RF01 - Carregamento de Mapa:** O sistema é capaz de carregar arquivos OpenStreetMap (.osm) e convertê-los internamente para um formato POLY otimizado, construindo assim o grafo correspondente para processamento.
* **RF02 - Visualização do Grafo:** O usuário pode visualizar o grafo carregado na interface gráfica, observando a disposição dos vértices (pontos) e arestas (conexões) que representam as vias.
* **RF03 - Seleção de Origem e Destino:** A interface permite que o usuário selecione visualmente os vértices de origem e destino diretamente no mapa, facilitando a interação.
* **RF04 - Cálculo de Menor Caminho:** O coração do sistema, esta funcionalidade calcula e exibe o menor caminho entre os pontos de origem e destino selecionados, utilizando o algoritmo de Dijkstra. O caminho encontrado é visualmente destacado no mapa.
* **RF05 - Edição do Grafo:** O sistema oferece modos de edição que permitem a manipulação do grafo, incluindo a adição e remoção de vértices e arestas, proporcionando flexibilidade na construção e modificação de mapas.
* **RF06 - Informações do Caminho:** Após o cálculo, o sistema exibe detalhes sobre o caminho encontrado, como a distância total percorrida, o número de vértices que compõem a rota e a sequência exata de vértices que formam o trajeto.
* **RF07 - Estatísticas de Execução:** Para análise de desempenho, o sistema apresenta estatísticas cruciais sobre o tempo de processamento do algoritmo de Dijkstra e o número de nós (vértices) explorados durante a busca pelo menor caminho.
* **RF08 - Exportação de Imagem:** O usuário tem a capacidade de copiar a imagem atual do mapa para a área de transferência, facilitando o compartilhamento ou a documentação visual.
* **RF09 - Controles de Visualização:** A interface oferece controles para personalizar a visualização do grafo, permitindo ajustar o zoom, selecionar a cor do grafo (cinza, cinza escuro, cinza claro), definir o tamanho dos pontos (vértices) e optar por numerar os vértices ou rotular as arestas para maior clareza.
* **RF10 - Interatividade da Tela:** A tela de visualização é altamente interativa, permitindo que o usuário arraste o mapa com o mouse para navegar por diferentes áreas e utilize o scroll para aplicar zoom in/out, proporcionando uma experiência de usuário fluida.
* **RF11 - Seleção de Vértices Interativa:** Os vértices podem ser selecionados diretamente com o mouse, simplificando a definição dos pontos de origem e destino, bem como a interação nos modos de edição.
* **RF12 - Modos de Edição Detalhados:**
  * **Modo de Navegação:** Permite que o usuário selecione apenas os vértices de origem e destino, e o programa automaticamente desenha o menor caminho.
  * **Adicionar Vértices:** Neste modo, o usuário pode inserir novos vértices no grafo em qualquer posição desejada no mapa.
  * **Adicionar Arestas:** Permite criar novas conexões (arestas) entre dois vértices existentes, ligando-os no grafo.
  * **Remover Vértice:** Oferece a funcionalidade de excluir vértices específicos do grafo, juntamente com todas as arestas a eles conectadas.
  * **Remover Arestas:** Permite remover arestas individuais entre dois vértices, desfazendo conexões específicas.
* **RF13 - Exibição de Origem e Destino:** O sistema exibe claramente os valores (IDs ou coordenadas) dos vértices selecionados como origem e destino, oferecendo feedback direto ao usuário.
* **RF14 - Reset de Seleção:** Um botão de "resetar a seleção" está disponível para limpar as arestas e/ou vértices selecionados, permitindo que o usuário reinicie a operação de busca de caminho ou edição.

### 2.2 Requisitos Não Funcionais (RNFs)

Os requisitos não funcionais descrevem como o sistema deve operar, focando na qualidade e nas restrições.

* **RNF01 - Usabilidade:** A interface do usuário é projetada para ser intuitiva e fácil de usar, minimizando a curva de aprendizado para novos usuários.
* **RNF02 - Desempenho:** O algoritmo de Dijkstra no backend C é otimizado para ser eficiente, com um tempo de resposta aceitável (ex: menos de 5 segundos para cálculo de rota em um mapa de 10.000 vértices), mesmo para mapas de tamanho médio.
* **RNF03 - Portabilidade:** O sistema é compatível e executável em sistemas operacionais Linux e Windows, garantindo uma ampla base de usuários.
* **RNF04 - Confiabilidade:** O sistema é robusto e capaz de lidar com arquivos de entrada inválidos ou corrompidos, exibindo mensagens de erro claras e informativas para o usuário.
* **RNF05 - Escalabilidade:** O backend em C é projetado para processar grafos maiores com um aumento proporcional, porém controlável, no tempo de processamento, permitindo a utilização com mapas mais extensos.
* **RNF06 - Segurança:** Para este projeto, o requisito de segurança não é aplicável, pois o sistema não manipula dados sensíveis nem oferece acesso remoto.
* **RNF07 - Manutenibilidade:** O código é estruturado de forma modular e bem documentado, facilitando futuras modificações, depurações e a compreensão por outros desenvolvedores.
* **RNF08 - Modularidade e Documentação:** O código é dividido em módulos lógicos, com documentação interna abrangente (comentários e docstrings), promovendo a clareza e a facilidade de manutenção.
* **RNF09 - Extensibilidade:** O design do sistema permite a adição de novos algoritmos de busca de caminho ou funcionalidades de visualização com impacto mínimo no código existente, tornando-o adaptável a futuras expansões.

---

## 3. Arquitetura do Sistema

O **AnicunsMap** segue uma arquitetura cliente-servidor simplificada, onde cada componente desempenha um papel específico. O frontend Python atua como o cliente, responsável pela interação com o usuário e pela exibição do mapa, enquanto o backend C funciona como o "servidor" de lógica, contendo os algoritmos de grafo e processamento de dados.

**Componentes Principais:**

* **Backend C (`backend_c/`):**
  * `integracao.h`: Contém as definições das estruturas de dados fundamentais (Nós, Vias, Vértices, Arestas, Heap, etc.) e os protótipos das funções que serão expostas ao frontend.
  * `integracao.c`: Implementa as funcionalidades principais, incluindo o parsing de arquivos OSM, o carregamento de dados do formato POLY, a construção eficiente do grafo, a implementação do algoritmo de Dijkstra para o menor caminho e as funções para edição do grafo (adição/remoção de vértices/arestas).
  * `main_test.c`: Um programa C opcional, mas altamente recomendado, para testar as funcionalidades do backend de forma independente da interface gráfica, agilizando o desenvolvimento e a depuração.
* **Frontend Python (`frontend_python/`):
  * `main_gui.py`: É o arquivo principal que implementa a interface gráfica do usuário utilizando a biblioteca PySide6. Ele é responsável por capturar as interações do usuário, renderizar a visualização do mapa e, crucialmente, invocar as funções do backend C através da biblioteca `ctypes`.
  * `assets/`: Diretório que armazena recursos adicionais, como arquivos de exemplo ou outros assets visuais.
* **Scripts de Build/Execução:**
  * `build_and_run.sh`: Um script shell projetado para sistemas Linux/macOS que automatiza o processo de compilação da biblioteca C e, em seguida, inicia a aplicação Python.
  * `build_and_run.bat`: O script equivalente para sistemas Windows, que executa as mesmas etapas de compilação e execução.

**Diagrama de Componentes:**

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

---

## 4. Tecnologias Utilizadas

O desenvolvimento do AnicunsMap emprega uma combinação de tecnologias de código aberto para garantir desempenho, portabilidade e uma interface de usuário responsiva.

* **Linguagens de Programação:**
  * **C:** Utilizada para o backend devido à sua eficiência e controle de baixo nível, ideal para o processamento de grafos e algoritmos de caminho.
  * **Python:** Empregada no frontend para o desenvolvimento rápido da interface gráfica e a facilidade de integração com o backend C via `ctypes`.
* **Framework GUI:**
  * **PySide6:** Um binding Python para a poderosa biblioteca Qt, que permite a criação de interfaces gráficas de usuário ricas e multiplataforma.
* **Ferramentas de Build:**
  * **GCC (GNU Compiler Collection):** O compilador padrão para o código C no backend.
  * `venv` (Python): Utilizado para criar ambientes virtuais Python isolados, garantindo que as dependências do projeto não interfiram com outras instalações Python no sistema.
* **Formatos de Dados:**
  * **OpenStreetMap (.osm):** O formato de entrada padrão para dados geográficos, permitindo o uso de mapas reais.
  * **POLY:** Um formato proprietário (interno) utilizado para representar o grafo de forma otimizada após o parsing do arquivo OSM.

---

## 5. Estrutura de Pastas

A organização do projeto segue uma estrutura lógica para facilitar a navegação, o desenvolvimento e a manutenção.

TrabalhoFinalAED2/
├── backend_c/
│   ├── lib/
│   │   └── integracao_lib.so   # Biblioteca C compilada (Linux/macOS) ou integracao_lib.dll (Windows)
│   └── src/
│       ├── integracao.h        # Definições de estruturas e protótipos do backend C
│       ├── integracao.c        # Implementação das funcionalidades do backend C
│       └── main_test.c         # Programa de teste independente para o backend C (opcional)
├── frontend_python/
│   ├── assets/
│   │   └── exemplo.md          # Recursos adicionais, como arquivos de exemplo
│   ├── venv/                   # Ambiente virtual Python para isolamento de dependências
│   └── main_gui.py             # Script principal da interface gráfica do usuário
├── data/                       # Diretório para armazenar arquivos de mapa
│   └── anicuns.osm             # Exemplo de arquivo OpenStreetMap
│   └── anicuns.poly            # Exemplo de arquivo POLY (grafo convertido)
├── build_and_run.sh            # Script para compilar e executar o sistema (Linux/macOS)
├── build_and_run.bat           # Script para compilar e executar o sistema (Windows)
└── README.md                   # Este arquivo de documentação

---

## 6. Como Compilar e Executar

Para colocar o AnicunsMap em funcionamento, siga os passos detalhados abaixo.

### 6.1 Pré-requisitos

Certifique-se de que os seguintes softwares estão instalados e configurados em seu sistema antes de prosseguir:

* **Python 3.x:** Recomenda-se a versão mais recente. Você pode baixá-lo em [python.org](https://www.python.org/).
* **GCC (GNU Compiler Collection):** Necessário para compilar o código C do backend.
  * **Linux:** Na maioria das distribuições, o GCC já vem pré-instalado ou pode ser instalado facilmente. Por exemplo, em sistemas baseados em Debian/Ubuntu, use `sudo apt install build-essential`. Em sistemas Fedora/RHEL, utilize `sudo yum groupinstall "Development Tools"`.
  * **Windows:** A maneira mais recomendada é instalar o [MinGW-w64](https://mingw-w64.org/doku.php/download/mingw-builds) e certificar-se de que ele esteja adicionado à variável de ambiente `PATH` do sistema.
  * **macOS:** Instale as Xcode Command Line Tools executando `xcode-select --install` no terminal.

### 6.2 Preparação do Ambiente (Primeira Vez)

Navegue até a pasta raiz do projeto (`TrabalhoFinalAED2/`) no seu terminal para iniciar a preparação.

1. **Crie e Ative um Ambiente Virtual Python:**
   É fundamental utilizar um ambiente virtual para isolar as dependências do projeto e evitar conflitos com outras instalações Python no seu sistema.

   ```bash
   cd frontend_python/
   python3 -m venv venv
   ```

   * **Para Linux/macOS:**
     ```bash
     source venv/bin/activate
     ```
   * **Para Windows (Command Prompt):**
     ```cmd
     .\venv\Scripts\activate.bat
     ```
   * **Para Windows (PowerShell):**
     ```powershell
     .\venv\Scripts\Activate.ps1
     ```
2. **Instale as Dependências Python:**
   Com o ambiente virtual ativado, instale a biblioteca `PySide6` necessária para a interface gráfica.

   ```bash
   pip install PySide6
   ```
3. **Retorne à Pasta Raiz do Projeto:**
   Após a instalação das dependências, volte para a pasta principal do projeto.

   ```bash
   cd ..
   ```

### 6.3 Executando o Programa

Uma vez que o ambiente esteja preparado, você pode compilar o backend C e iniciar a interface gráfica do sistema.

* **Para Linux/macOS:**
  Execute o script de shell `build_and_run.sh`.

  ```bash
  ./build_and_run.sh
  ```

  * *Nota:* Se encontrar um erro de permissão, certifique-se de que o script tenha permissões de execução com o comando: `chmod +x build_and_run.sh`.
* **Para Windows:**
  Execute o arquivo `build_and_run.bat` no Command Prompt (Prompt de Comando).

  ```cmd
  build_and_run.bat
  ```

Ao seguir estas instruções, o programa será iniciado e a janela da aplicação de navegação do AnicunsMap será exibida. Por padrão, o mapa `anicuns.osm` (localizado na pasta `data/`) será carregado automaticamente. Você pode carregar outros arquivos OpenStreetMap (.osm) utilizando o botão "Carregar Outro Arquivo OSM" disponível na interface do usuário.

---

## 7. Autores

Este projeto foi desenvolvido com a colaboração dos seguintes autores:

* Ana Luisa Gonçalves
* Gabriel Rodrigues
* Julia Nascimento
* Luis Vittor Ferreira Nunes
