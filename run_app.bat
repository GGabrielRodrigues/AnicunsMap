@echo off
REM Não precisamos de delayed expansion aqui
::SETLOCAL ENABLEDELAYEDEXPANSION

echo.
echo ==========================================================
echo Iniciando o Sistema de Navegacao - Para a primeira execucao
echo ==========================================================
echo.

:: --- 1. Verificar se Python 3 esta instalado ---
echo [PASSO 1/3] Verificando instalacao do Python 3...
python --version >NUL 2>&1

 IF ERRORLEVEL 1 (
     echo ERRO: Python 3 nao encontrado.
     
     pause >NUL
         exit /B 1
 )

:: --- 2. Criar e ativar ambiente virtual ---
echo.
echo [PASSO 2/3] Criando e ativando o ambiente virtual...
python -m venv venv
IF %ERRORLEVEL% NEQ 0 (
    echo ERRO: Nao foi possivel criar o ambiente virtual.
    pause >NUL
    exit /B 1
)

call venv\Scripts\activate

pip install pyside6 >NUL
IF %ERRORLEVEL% NEQ 0 (
    echo ERRO: Falha ao instalar dependencias Python.
    pause >NUL
    exit /B 1
)

:: --- 3. Executar a Aplicacao Python ---
echo.
echo [PASSO 3/3] Executando o Sistema de Navegacao...
python frontend_python\main_gui.py
IF %ERRORLEVEL% NEQ 0 (
    echo ERRO: O aplicativo Python encontrou um problema e foi encerrado.
    pause >NUL
    exit /B 1
)

echo.
echo ==========================================================
echo Execucao finalizada.
echo ==========================================================
echo Pressione qualquer tecla para fechar esta janela.
call deactivate
pause >NUL