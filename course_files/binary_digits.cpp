// Перевод неотрицательного целого числа в двоичную запись.
#include <algorithm>
#include <iostream>
#include <string>

int main() {
    long long number;
    std::cout << "Введите неотрицательное целое число: ";
    if (!(std::cin >> number) || number < 0) {
        std::cerr << "Некорректное число.\n";
        return 1;
    }
    std::string bits;
    do {
        bits += static_cast<char>('0' + (number & 1));
        number >>= 1;
    } while (number != 0);

    // Младший бит извлекается первым, поэтому разворачиваем строку.
    std::reverse(bits.begin(), bits.end());
    std::cout << "Двоичная запись: " << bits << '\n';
}
