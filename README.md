# QuickSIG (`quicksig`)

Утилита для Linux, которая помогает безопасно и быстро завершать зависшие процессы.

Проект включает:

- CLI-команду `quicksig` (поиск по имени/PID + завершение процесса)
- Интерактивный режим `quicksig-gui` через `fzf`

## Возможности

- Поиск процесса по имени или части командной строки
- Завершение процесса по PID
- Мягкое завершение через `SIGTERM`
- Принудительное завершение через `SIGKILL` (`--force`)
- Подтверждение перед убийством процессов
- Режим без подтверждения (`--yes`)
- Просмотр всех процессов (`--list`)
- Защита от self-kill: утилита не завершает сама себя

## Требования

- Linux (используется `/proc`)
- C++ компилятор с поддержкой C++17 (`g++` или `clang++`)
- `make`
- Для GUI-режима дополнительно: `fzf`

### Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential make fzf
```

## Быстрый старт: установка для использования

Если хотите сразу пользоваться командой глобально из любой папки:

```bash
cd /home/feelyamon/Cpp_projects/quicksig
make
sudo make install
```

Что делает каждая команда:

- `cd /home/feelyamon/Cpp_projects/quicksig`: переход в папку проекта
- `make`: сборка бинарника `quicksig`
- `sudo make install`: установка `quicksig` и `quicksig-gui` в `/usr/local/bin`

Проверка после установки:

```bash
which quicksig
which quicksig-gui
quicksig --help
```

## Сборка

```bash
cd quicksig
make
```

После сборки появится бинарник `./quicksig`.

## Локальный запуск

### 1) Поиск и завершение по имени

```bash
./quicksig --name telegram
```

### 2) Завершение по PID

```bash
./quicksig --pid 12345
```

### 3) Принудительное завершение (если процесс игнорирует SIGTERM)

```bash
./quicksig --name telegram --force
```

### 4) Без подтверждения

```bash
./quicksig --name telegram --yes --force
```

### 5) Список процессов

```bash
./quicksig --list
```

### 6) Интерактивный режим

```bash
./quicksig --interactive
```

## Установка как системной команды

### Вариант A: через Makefile

```bash
sudo make install
```

Установятся:

- `/usr/local/bin/quicksig`
- `/usr/local/bin/quicksig-gui`

Удаление:

```bash
sudo make uninstall
```

### Вариант B: через скрипт

```bash
sudo ./scripts/install.sh
```

## Использование после установки

```bash
quicksig --name telegram
quicksig --pid 12345 --force
quicksig --list
quicksig --interactive
```

GUI напрямую:

```bash
quicksig-gui
```

## Полный список аргументов

```bash
quicksig --help
```

Ключи:

- `-n, --name <query>`: поиск по имени/командной строке
- `-p, --pid <pid>`: PID процесса (можно повторять)
- `-f, --force`: после таймаута SIGTERM послать SIGKILL
- `-y, --yes`: не спрашивать подтверждение
- `-l, --list`: показать все процессы
- `-i, --interactive`: интерактивный выбор через `fzf`
- `-h, --help`: помощь

## Безопасность и права

- Обычно можно завершать только процессы текущего пользователя.
- Для чужих процессов может понадобиться `sudo`.
- Используйте `--force` только когда обычное завершение не помогает.
- Не завершайте критические системные процессы (`systemd`, `sshd`, и т.д.).

## Типичные сценарии

### Telegram завис

```bash
quicksig --name telegram
```

Если не закрывается:

```bash
quicksig --name telegram --force
```

### Процесс завис по известному PID

```bash
quicksig --pid 4321 --force
```

## Разработка

Сборка:

```bash
make
```

Очистка:

```bash
make clean
```

## Структура проекта

```text
quicksig/
├── src/
│   ├── main.cpp
│   ├── process_manager.h
│   └── process_manager.cpp
├── scripts/
│   ├── fzf_gui.sh
│   └── install.sh
├── Makefile
└── README.md
```

## Публикация в GitHub

```bash
git init
git add .
git commit -m "Initial commit: quicksig CLI tool"
git branch -M main
git remote add origin <YOUR_GITHUB_REPO_URL>
git push -u origin main
```

Если репозиторий уже создан локально:

```bash
git add .
git commit -m "Add README and gitignore"
git push
```

## Лицензия

Добавьте файл `LICENSE` перед публикацией (например MIT), если планируете open-source.
