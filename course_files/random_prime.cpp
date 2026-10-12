// Генерация простого числа меньше 100. Проверка перебором делителей.
#include <iostream>
#include <random>

bool is_prime(int number) {
    if (number < 2) {
        return false;
    }
    for (int divisor = 2; divisor <= number / divisor; ++divisor) {
        if (number % divisor == 0) {
            return false;
        }
    }
    return true;
}

int main() {
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(2, 99);
    int number;
    do {
        number = distribution(generator);
    } while (!is_prime(number));
    std::cout << number << '\n';
}
