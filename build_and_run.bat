@echo off
echo --- LIMPANDO QUALQUER BIBLIOTECA ANTIGA...
del backend_c\lib\integracao_lib.dll > nul 2>&1

echo --- COMPILANDO A NOVA BIBLIOTECA C...
gcc -shared -o backend_c\lib\integracao_lib.dll -Ibackend_c\src backend_c\src\integracao.c

if %errorlevel% neq 0 (
    echo Erro na compilacao do backend C. Certifique-se de que o GCC esta no seu PATH.
    pause
    exit /b %errorlevel%
)

echo --- COMPILACAO CONCLUIDA COM SUCESSO.
echo --- ATIVANDO O AMBIENTE VIRTUAL E INICIANDO O PROGRAMA PYTHON...

cd frontend_python
call .\venv\Scripts\activate.bat
python main_gui.py
cd ..
pause