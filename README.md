# Лабораторные по защите информации

C++20, стандартная библиотека, сборка через `g++`. Готовы лабораторные **1–4 и 9–11**; 5–8 пока пропущены. Команды ниже выполняются в Linux/WSL из корня репозитория.

## Структура

| Путь | Что находится |
| --- | --- |
| `labs/lab01/` | Модульная степень, тест Ферма, расширенный алгоритм Евклида |
| `labs/lab02/` | Дискретный логарифм: шаг младенца — шаг великана |
| `labs/lab03/` | Обмен ключами Диффи — Хеллмана |
| `labs/lab04/` | Шифр Шамира для файлов; `files.hpp` — файловые операции |
| `labs/lab09/`, `lab10/`, `lab11/` | Подписи Эль-Гамаля, ГОСТ Р 34.10-94 и DSA соответственно |
| `common/crypto.hpp`, `crypto.cpp` | Общая математика, ключи, алгоритмы подписи и проверки |
| `common/signature_files.hpp`, `signature_files.cpp` | Чтение файлов, MD5, сохранение ключей и подписей |
| `common/signature_app.hpp`, `signature_app.cpp` | Общее меню и команды для лабораторных 9–11 |
| `common/signature_tests.hpp` | Общие проверки трёх алгоритмов подписи |
| `course_files/` | Примеры курса; `md5.hpp` — используемая реализация MD5 |
| `lectures/` | PDF лекций и заданий |
| `build/` | Собранные программы и временные файлы; создаётся локально, игнорируется Git |

В каждой `labs/labNN/` файл `main.cpp` запускает программу, `tests.cpp` — проверки. Для сборки нужны также общие файлы из `common/` и, для подписей, `course_files/md5.hpp`.

## Сборка и запуск 9–11

```sh
mkdir -p build
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/lab09/main.cpp common/crypto.cpp common/signature_files.cpp common/signature_app.cpp -o build/lab09
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/lab10/main.cpp common/crypto.cpp common/signature_files.cpp common/signature_app.cpp -o build/lab10
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/lab11/main.cpp common/crypto.cpp common/signature_files.cpp common/signature_app.cpp -o build/lab11
```

Для интерактивного меню запустите `./build/lab09`, `./build/lab10` или `./build/lab11`. Пункты одинаковые: **1 — создать ключи, 2 — подписать файл, 3 — проверить подпись, 0 — выход**. Ключи генерируются автоматически. Каждый ответ вводится на отдельной строке; пути в меню — без кавычек.

Быстрая ручная проверка числовых примеров:

```sh
./build/lab09 demo
./build/lab10 demo
./build/lab11 demo
```

Ожидаемые пары `(r,s)`: **(20,21)**, **(2,3)**, **(2,6)**. Каждая программа выводит «Подпись верна».

## Ручная проверка файла

В первой строке выберите `09`, `10` или `11`. Каждое выполнение создаёт новую папку для примера.

```sh
lab=09
demo_dir=$(mktemp -d build/check-XXXXXX)
printf 'abc' > "$demo_dir/message.txt"
./build/lab${lab} keygen "$demo_dir/secret.key" "$demo_dir/public.key"
./build/lab${lab} sign "$demo_dir/message.txt" "$demo_dir/secret.key" "$demo_dir/message.sig"
./build/lab${lab} verify "$demo_dir/message.txt" "$demo_dir/public.key" "$demo_dir/message.sig"
```

Результат: **«Подпись верна»**. Вместо `message.txt` можно подписать любой обычный файл: текст, изображение, PDF, архив. Подпись сохраняется отдельно, исходный файл не меняется. Для проверки нужны документ, открытый ключ и подпись; секретный ключ передавать не нужно.

Теперь в том же терминале создайте изменённую копию и проверьте её со старой подписью:

```sh
printf 'abd' > "$demo_dir/changed.txt"
./build/lab${lab} verify "$demo_dir/changed.txt" "$demo_dir/public.key" "$demo_dir/message.sig"
```

Ожидается **«Подпись НЕВЕРНА»**, код выхода 2. Код 0 означает успех, 1 — ошибку формата, параметров или чтения. Существующие ключи и подписи не перезаписываются; для нового результата выбирайте новое имя. В командной строке пути с пробелами заключайте в кавычки.

## Автоматические проверки и предыдущие лабораторные

Тесты подписи: замените `09` на `10` или `11` для другой работы.

```sh
lab=09
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 "labs/lab${lab}/tests.cpp" common/crypto.cpp common/signature_files.cpp -o "build/lab${lab}_tests"
./build/lab${lab}_tests
```

Для лабораторных 1–4 достаточно `common/crypto.cpp`; замените `01` на `02`, `03` или `04`:

```sh
lab=01
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 "labs/lab${lab}/main.cpp" common/crypto.cpp -o "build/lab${lab}"
./build/lab${lab}
```

Для их тестов замените `main.cpp` на `tests.cpp`, а выходной файл — на `build/lab${lab}_tests`.

Реализации учебные: модуль до 2 млрд, для подписей используется MD5 и сокращённый хеш. Это не полноразмерные реализации криптографических стандартов; коллизии возможны, поэтому обнаружение любого изменения файла не гарантируется.
