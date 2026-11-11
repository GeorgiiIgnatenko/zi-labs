# Лабораторная №11 — Электронная подпись DSA (FIPS 186)

Задание: страница 5 [PDF лабораторных](../../lectures/Лабораторные%20работы%20ЗИ.pdf). Теория: [лекция 8](../../lectures/Лекция%208%20ЗИ%202026.pdf), страницы 50–57, особенно 57.

Учебная реализация формул: C++20, p≤2 млрд, MD5 из курса. Это не полноразмерная реализация стандарта для практической защиты документов. Лабы 5–8 для запуска не нужны.

## Сборка и пример

Из корня репозитория:

```sh
mkdir -p build
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/lab11/main.cpp common/crypto.cpp common/signature_files.cpp common/signature_app.cpp -o build/lab11
./build/lab11 demo
./build/lab11
```

Пример даёт `(r,s)=(2, 6)` и «Подпись верна». Без аргументов открывается меню: 1 — генерация ключей, 2 — ручные ключи, 3 — подпись файла, 4 — проверка, 5 — пример лекции, 6 — ручные h,k, 0 — выход. Каждое число вводится на отдельной строке; пути в меню вводите без кавычек.

## Подписать любой файл

```sh
mkdir -p local/demo11
printf 'abc' > local/demo11/message.txt
./build/lab11 keygen local/demo11/secret.key local/demo11/public.key
./build/lab11 sign local/demo11/message.txt local/demo11/secret.key local/demo11/message.sig
./build/lab11 verify local/demo11/message.txt local/demo11/public.key local/demo11/message.sig
```

Ожидается «Подпись верна». В `sign` можно передать PNG, PDF, ZIP или любой другой обычный файл. Подпись хранится отдельно, исходник не меняется. Открытый ключ передаётся проверяющему отдельно; секретный файл ему не нужен. Новое k выбирается автоматически для каждого подписания.

Проверка изменения:

```sh
printf 'abd' > local/demo11/changed.txt
./build/lab11 verify local/demo11/changed.txt local/demo11/public.key local/demo11/message.sig
```

Для этого теста ожидается «Подпись НЕВЕРНА», код выхода 2. Код 1 означает ошибку чтения, параметров или формата; 0 — успех. Малые учебные параметры и сокращение хеша допускают коллизии, поэтому обнаружение абсолютно любого изменения не гарантируется.

Существующие ключи/подписи не перезаписываются: для повторения выберите новые выходные имена. Для путей с пробелами в CLI используйте кавычки. Секретный ключ содержит x открытым текстом; не передавайте его проверяющему.

## Проверки

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/lab11/tests.cpp common/crypto.cpp common/signature_files.cpp -o build/lab11_tests
./build/lab11_tests
./build/lab11 number 67 11 25 6 3 8
./build/lab11 hash local/demo11/message.txt
```

MD5 для `abc` без перевода строки: `900150983cd24fb0d6963f7d28e17f72`. Тесты: 1 100 расчётов подписи, включая 200 случаев повторного выбора k, и 4 761 проверка пар (r,s), ошибки параметров, генерация и границы. Независимые Python-проверки файлов и чисел сохранены в `local/checks/verify_labs09_11.py` (вне Git).

## Общая реализация

- `main.cpp` выбирает алгоритм этой лабораторной.
- [crypto.cpp](../../common/crypto.cpp) — ключи, математические формулы, проверка и генерация.
- [signature_files.cpp](../../common/signature_files.cpp) — потоковый MD5, чтение/запись ключей и подписей.
- [signature_app.cpp](../../common/signature_app.cpp) — одинаковые меню и CLI.
- [Подробный отчёт](../../local/reports/lab11.md).
- [Общий шаблон, сравнение и инструкция к защите](../../local/reports/labs09-11.md).

Отчёты находятся только в рабочей папке, `local/` игнорируется Git. Для передачи исходников нужны `labs/lab11/`, `common/` и `course_files/md5.hpp`; одна папка лабораторной отдельно не соберётся.
