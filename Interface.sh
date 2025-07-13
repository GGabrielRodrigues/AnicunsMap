#!/bin/bash

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

pip install -r frontend_python/requirements.txt

echo "--- LIMPANDO QUALQUER BIBLIOTECA ANTIGA..."

rm -f backend_c/lib/integracao_lib.so

echo "--- COMPILANDO A NOVA BIBLIOTECA C..."

gcc -shared -o backend_c/lib/integracao_lib.so -fPIC backend_c/src/integracao.c

echo "--- COMPILAÇÃO CONCLUÍDA COM SUCESSO."

# --- 3. EXECUÇÃO DA APLICAÇÃO PYTHON ---
echo "--- INICIANDO O PROGRAMA PYTHON..."
python3 frontend_python/main_gui.py

echo "--- PROGRAMA FINALIZADO."
