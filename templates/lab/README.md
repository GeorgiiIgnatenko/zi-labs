# Лабораторная №NN

Скопировать эту папку в `labs/labNN/`, заменить номер и описать задание.

Источник: указать PDF и страницу. Общие алгоритмы добавлять в `common/crypto.hpp` и `common/crypto.cpp`.

Из корня репозитория:

```sh
mkdir -p build
g++ -std=c++20 -Wall -Wextra -Wpedantic -O2 labs/labNN/main.cpp common/crypto.cpp -o build/labNN
./build/labNN
```

Добавить короткий пример ввода и ожидаемого вывода, границы поддерживаемых данных.
При необходимости создать `tests.cpp` и собирать той же командой, заменив `main.cpp` на `tests.cpp`, а имя программы на `labNN_tests`.

Подробный отчёт: `local/reports/labNN.md`, по шаблону `templates/report.md`.
