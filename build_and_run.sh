#!/bin/bash

# Este comando faz o script parar imediatamente se algum comando falhar.
set -e

# --- 1. CONFIGURAÇÃO E VERIFICAÇÃO DO AMBIENTE VIRTUAL ---
VENV_DIR="frontend_python/venv"

if [ ! -d "$VENV_DIR" ]; then
    echo "--- AMBIENTE VIRTUAL NÃO ENCONTRADO. CRIANDO UM NOVO..."
    python3 -m venv "$VENV_DIR"
    echo "--- AMBIENTE VIRTUAL CRIADO COM SUCESSO."
fi

# --- 2. ATIVAÇÃO E INSTALAÇÃO DE DEPENDÊNCIAS ---
echo "--- ATIVANDO O AMBIENTE VIRTUAL..."
source "$VENV_DIR/bin/activate"

echo "--- INSTALANDO DEPENDÊNCIAS DO requirements.txt..."
# O pip não reinstala pacotes que já existem, então este comando é eficiente.
pip install -r frontend_python/requirements.txt

echo "--- LIMPANDO QUALQUER BIBLIOTECA ANTIGA..."
# O -f ignora erros se o arquivo não existir.
rm -f backend_c/lib/integracao_lib.so

echo "--- COMPILANDO A NOVA BIBLIOTECA C..."
# Compila a biblioteca. Se houver um erro aqui, o script vai parar.
gcc -shared -o backend_c/lib/integracao_lib.so -fPIC backend_c/src/integracao.c

echo "--- COMPILAÇÃO CONCLUÍDA COM SUCESSO."

# --- 4. EXECUÇÃO DA APLICAÇÃO PYTHON ---
echo "--- INICIANDO O PROGRAMA PYTHON..."
python3 frontend_python/main_gui.py

echo "--- PROGRAMA FINALIZADO."
