#include "md5.hpp"

#include <iostream>

int main() {
    const std::string message = "I love ZI";
    const std::string hash = md5(message);
    const auto parts = HashToVector(hash);
    std::cout << "Сообщение: " << message << "\nMD5: " << hash << "\n8 блоков по 16 бит: ";
    for (auto part : parts) {
        std::cout << part << ' ';
    }
    std::cout << '\n';

    // Старый интерфейс из курса: вызывающий освобождает выделенный массив.
    const auto* array = HashToArray(hash);
    delete[] array;
}
