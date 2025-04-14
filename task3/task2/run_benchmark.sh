
VENV_NAME="lab3venv"
REQUIREMENTS_FILE="requirements.txt"
PYTHON_SCRIPT="benchmark.py"

if ! command -v virtualenv &> /dev/null; then
    echo "Установка virtualenv..."
    pip install virtualenv --user || {
        echo "Ошибка установки virtualenv"
        exit 1
    }
    export PATH="$PATH:$HOME/.local/bin"
fi


if [ ! -d "$VENV_NAME" ]; then
    echo "Создание виртуального окружения '$VENV_NAME'..."
    virtualenv "$VENV_NAME" || {
        echo "Ошибка создания виртуального окружения"
        exit 1
    }
    pip install -r "$REQUIREMENTS_FILE" || {
    echo "Ошибка установки зависимостей"
    deactivate
    exit 1
}
fi

echo "Активация окружения"
source "$VENV_NAME/bin/activate"|| {
        echo "Ошибка активации виртуального окружения"
        exit 1
}

echo "Запуск скрипта $PYTHON_SCRIPT..."
python "$PYTHON_SCRIPT" || {
    echo "Ошибка выполнения скрипта"
    deactivate
    exit 1
}

deactivate
echo "Все операции завершены успешно!"