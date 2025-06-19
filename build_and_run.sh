#!/bin/bash

# Este comando faz o script parar imediatamente se algum comando falhar.
set -e

echo "--- LIMPANDO QUALQUER BIBLIOTECA ANTIGA..."
# O -f ignora erros se o arquivo não existir.
rm -f backend_c/lib/integracao_lib.so

echo "--- COMPILANDO A NOVA BIBLIOTECA C..."
# Compila a biblioteca. Se houver um erro aqui, o script vai parar.
gcc -shared -o backend_c/lib/integracao_lib.so -fPIC backend_c/src/integracao.c

echo "--- COMPILAÇÃO CONCLUÍDA COM SUCESSO."
echo "--- ATIVANDO O AMBIENTE VIRTUAL E INICIANDO O PROGRAMA PYTHON..."

# Ativa o venv e executa o Python.
source frontend_python/venv/bin/activate
python3 frontend_python/main_gui.py