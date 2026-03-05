# linux_killer (`pkill-smart`)

Утилита для Linux, которая помогает безопасно и быстро завершать зависшие процессы.

Проект включает:

- CLI-команду `pkill-smart` (поиск по имени/PID + завершение процесса)
- Интерактивный режим `pkill-smart-gui` через `fzf`

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

## Сборка

```bash
cd linux_killer
make
```

После сборки появится бинарник `./pkill-smart`.

## Локальный запуск

### 1) Поиск и завершение по имени

```bash
./pkill-smart --name telegram
```

### 2) Завершение по PID

```bash
./pkill-smart --pid 12345
```

### 3) Принудительное завершение (если процесс игнорирует SIGTERM)

```bash
./pkill-smart --name telegram --force
```

### 4) Без подтверждения

```bash
./pkill-smart --name telegram --yes --force
```

### 5) Список процессов

```bash
./pkill-smart --list
```

### 6) Интерактивный режим

```bash
./pkill-smart --interactive
```

## Установка как системной команды

### Вариант A: через Makefile

```bash
sudo make install
```

Установятся:

- `/usr/local/bin/pkill-smart`
- `/usr/local/bin/pkill-smart-gui`

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
pkill-smart --name telegram
pkill-smart --pid 12345 --force
pkill-smart --list
pkill-smart --interactive
```

GUI напрямую:

```bash
pkill-smart-gui
```

## Полный список аргументов

```bash
pkill-smart --help
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
pkill-smart --name telegram
```

Если не закрывается:

```bash
pkill-smart --name telegram --force
```

### Процесс завис по известному PID

```bash
pkill-smart --pid 4321 --force
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
linux_killer/
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
git commit -m "Initial commit: pkill-smart process killer"
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
