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

REM Salva o diretório atual da raiz do projeto
set PROJECT_ROOT=%~dp0

REM Navega para o diretório frontend_python
cd "%PROJECT_ROOT%frontend_python"

REM Ativa o ambiente virtual
call ".\venv\Scripts\activate.bat"

REM Executa o script Python
python main_gui.py

REM Desativa o ambiente virtual e retorna ao diretório original (opcional, mas boa prática)
deactivate
cd "%PROJECT_ROOT%"

pause
