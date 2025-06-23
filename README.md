# AnicunsMap - Sistema de Navegação Dijkstra

---

Sistema de navegação interativo que utiliza o algoritmo de Dijkstra para calcular o menor caminho entre dois pontos em um mapa real extraído do OpenStreetMap. O cálculo de rotas é realizado por um backend em **C**, e a visualização gráfica e interação do usuário são gerenciadas com a biblioteca PySide6, sendo assim um frontend em Python.

Segue acesso a documentação completa: link

---

## Funcionalidades Principais

* Carregamento de mapas OSM e conversão para formato interno POLY
* Visualização gráfica interativa do grafo
* Seleção de vértices de origem e destino via interface
* Cálculo e exibição do menor caminho (Dijkstra)
* Edição dinâmica do grafo (adição/remoção de vértices e arestas)
* Estatísticas de execução (tempo e nós visitados)
* Exportação de imagem do mapa e personalização da visualização

## Tecnologias Utilizadas 

* Backend em C (estrutura de grafos + algoritmo de Dijkstra)
* Frontend em Python com PySide6 (GUI)
* Integração entre duas camadas via ctypes (Python ↔ C)
* Dados no formato OpenStreetMap (.osm)

## Estrutura de Pastas 
O projeto é organizado em uma estrutura de pastas lógica que separa o backend, o frontend e os dados, facilitando a navegação e a manutenção.
```
TrabalhoFinalAED2/
├── backend_c/
│   ├── build/
│   │   └── main_test
│   ├── lib/
│   │   ├── integracao_lib.dll
│   │   └── integracao_lib.so
│   └── src/
│       ├── integracao.c
│       ├── integracao.h
│       └── main_test.c
├── data/
│   ├── Campus2UFG&Regiao.osm
│   └── anicuns.osm
├── frontend_python/
│   ├── assets/
│   ├── venv/
│   ├── main_gui.py
│   └── requirements.txt
├── .gitattributes
├── README.md
├── build_and_run.sh
├── compila_dll.bat
└── interface.bat
```

## Execução 
Graças aos scripts de automação, colocar o AnicunsMap para funcionar é um processo direto. Os scripts cuidam da criação de ambientes virtuais, instalação de dependências, compilação e execução.

### Pré-requisitos

Garanta que os seguintes softwares estejam instalados em seu sistema.

* **Para todos os usuários (Windows, Linux, macOS):**
    * **Python 3.x:** A versão mais recente é recomendada. Baixe em [python.org](https://www.python.org/) e certifique-se de adicioná-lo ao PATH do sistema durante a instalação.

* **Para usuários de Linux/macOS (ou desenvolvedores Windows que modificarão o C):**
    * **Compilador C (GCC):**
        * **Linux:** Instale o pacote `build-essential` (em sistemas Debian/Ubuntu) ou `Development Tools` (em sistemas Fedora/RHEL).
        * **macOS:** Instale as Xcode Command Line Tools com o comando `xcode-select --install`.
        * **Windows (Opcional):** Para modificar o backend, instale o [MinGW-w64](https://mingw-w64.org/doku.php/download/mingw-builds).

### Executando o Programa

Com os pré-requisitos atendidos, basta executar um único script. Não é necessário criar ou ativar o ambiente virtual manualmente.

* **Para Linux/macOS:**
    Execute o script `build_and_run.sh` a partir da pasta raiz do projeto  (`TrabalhoFinalAED2/`) .
    ```bash
    ./build_and_run.sh
    ```
    Este script irá automaticamente:
    1.  Verificar se um ambiente virtual existe em `frontend_python/venv` e, se não, irá criá-lo.
    2.  Ativar o ambiente virtual.
    3.  Instalar as dependências listadas no `requirements.txt`.
    4.  Compilar o código C do backend para gerar a biblioteca `integracao_lib.so`.
    5.  Iniciar a aplicação gráfica.

    > **Nota:** Na primeira vez que executar, talvez seja necessário dar permissão de execução ao script com o comando: `chmod +x build_and_run.sh`.
    
* **Para Windows:**
    Execute o arquivo `run_app.bat` (ou `build_and_run.bat`, dependendo da versão do projeto) com um duplo-clique ou pelo terminal.
    ```cmd
    run_app.bat
    ```
    Este script irá automaticamente:
    1.  Verificar se um ambiente virtual existe e, se não, irá criá-lo.
    2.  Ativar o ambiente virtual.
    3.  Instalar as dependências listadas no `requirements.txt`.
    4.  Iniciar a aplicação gráfica utilizando a biblioteca `integracao_lib.dll` **pré-compilada**.

    > **Nota para Desenvolvedores Windows:** O script principal **não recompila** o código C. Se você fizer qualquer alteração nos arquivos do backend (`.c` ou `.h`), você deve executar o script `compila_dll.bat` manualmente **antes** de rodar `run_app.bat` para que suas mudanças tenham efeito.

---

## Autores 
* Ana Luísa Gonçalves 
* Gabriel Rodrigues da Silva 
* Júlia de Souza Nascimento
* Luis Vittor Ferreira Nunes 

---


