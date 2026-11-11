#include "signature_app.hpp"
#include "signature_files.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace signature_app {
namespace {
using crypto::SignatureAlgorithm;

std::string read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Ввод завершён.");
    }
    return line;
}

long long number(const std::string& text) {
    std::istringstream input(text);
    long long result;
    char extra;
    if (!(input >> result) || input >> extra || result < 0 || result > crypto::max_number) {
        throw std::invalid_argument("Введите целое число от 0 до 2000000000.");
    }
    return result;
}

long long read_number(const std::string& prompt) {
    while (true) {
        const auto text = read_line(prompt);
        try {
            return number(text);
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}

void print_public(const crypto::SignaturePublicKey& key) {
    std::cout << "Открытый ключ: p=" << key.p << ", q=" << key.q << ", g(a)=" << key.g
              << ", y=" << key.y << '\n';
}

void show_number(const crypto::SignatureKeyPair& key, long long h, long long k) {
    print_public(key.public_key);
    std::cout << "Числовой пример: x=" << key.x << ", h=" << h << ", k=" << k << '\n';
    const auto signature = crypto::sign_hash(h, key, k);
    if (!signature) {
        std::cout << "Получено r=0 или s=0. Нужно другое k. Подпись не создана.\n";
        return;
    }
    std::cout << "r=" << signature->r << ", s=" << signature->s << '\n';
    std::cout << (crypto::verify_hash(h, key.public_key, *signature) ? "Подпись верна.\n" : "Подпись НЕВЕРНА.\n");
}

void demo(SignatureAlgorithm algorithm) {
    if (algorithm == SignatureAlgorithm::elgamal) {
        std::cout << "Лекция 8, стр. 44–46: заданный h=3, не MD5 файла baaaab.\n";
        show_number(crypto::make_signature_key(algorithm, 23, 0, 5, 7), 3, 5);
        std::cout << "u=17, k^(-1) mod 22=9; y^r*r^s mod p=10, g^h mod p=10.\n";
    } else {
        std::cout << (algorithm == SignatureAlgorithm::gost94
            ? "Лекция 8, стр. 55–56: заданный h=3.\n"
            : "Параметры стр. 55 и формулы DSA стр. 57 лекции 8; заданный h=3.\n");
        show_number(crypto::make_signature_key(algorithm, 67, 11, 25, 6), 3, 8);
        std::cout << (algorithm == SignatureAlgorithm::gost94
            ? "h^(-1)=4, u1=1, u2=3, v=2=r.\n"
            : "k^(-1)=7, s^(-1)=2, u1=6, u2=4, v=2=r.\n");
    }
}

void help(const std::string& executable) {
    std::cout << "Без аргументов — меню. Команды из корня проекта:\n"
              << executable << " keygen secret.key public.key\n"
              << executable << " sign файл secret.key подпись.sig\n"
              << executable << " verify файл public.key подпись.sig\n"
              << executable << " demo\n"
              << executable << " number p q g x h k  (Эль-Гамаль: q=0)\n"
              << executable << " hash файл\n"
              << "Для путей с пробелами в командной строке нужны кавычки.\n"
              << "Коды выхода CLI: 0 — успех, 1 — ошибка, 2 — неверная подпись.\n";
}

int command(SignatureAlgorithm algorithm, const std::vector<std::string>& args) {
    if (args.size() == 1 && args[0] == "demo") {
        demo(algorithm);
    } else if (args.size() == 3 && args[0] == "keygen") {
        const auto key = crypto::generate_signature_key(algorithm);
        signature_files::save_key_pair(args[1], args[2], key);
        print_public(key.public_key);
        std::cout << "Ключи сохранены. Секретный файл нужен только для подписания.\n";
    } else if (args.size() == 4 && args[0] == "sign") {
        const auto key = signature_files::load_private_key(args[2], algorithm);
        const auto result = signature_files::sign_file(args[1], args[3], key);
        std::cout << "MD5=" << result.digest << "\nh=" << result.h << "\nr=" << result.signature.r
                  << ", s=" << result.signature.s << "\nПодпись сохранена: " << args[3] << '\n';
    } else if (args.size() == 4 && args[0] == "verify") {
        const auto key = signature_files::load_public_key(args[2], algorithm);
        const bool valid = signature_files::verify_file(args[1], args[3], key);
        std::cout << (valid ? "Подпись верна.\n" : "Подпись НЕВЕРНА.\n");
        return valid ? 0 : 2;
    } else if (args.size() == 7 && args[0] == "number") {
        const auto key = crypto::make_signature_key(algorithm, number(args[1]), number(args[2]),
                                                     number(args[3]), number(args[4]));
        show_number(key, number(args[5]), number(args[6]));
    } else if (args.size() == 2 && args[0] == "hash") {
        std::cout << signature_files::md5_file(args[1]) << '\n';
    } else {
        throw std::invalid_argument("Неверная команда. Используйте --help или запустите без аргументов.");
    }
    return 0;
}

void menu(SignatureAlgorithm algorithm) {
    const auto name = algorithm == SignatureAlgorithm::elgamal ? "Эль-Гамаля"
        : (algorithm == SignatureAlgorithm::gost94 ? "ГОСТ Р 34.10-94" : "DSA");
    std::cout << "Подпись " << name << '\n';
    while (true) {
        try {
            std::cout << "\n1. Создать ключи\n2. Подписать файл\n3. Проверить подпись\n0. Выход\n";
            const auto choice = read_number("Пункт: ");
            if (choice == 0) {
                return;
            }
            if (choice == 1) {
                const auto key = crypto::generate_signature_key(algorithm);
                const auto private_path = read_line("Новый файл секретного ключа: ");
                const auto public_path = read_line("Новый файл открытого ключа: ");
                signature_files::save_key_pair(private_path, public_path, key);
                print_public(key.public_key);
                std::cout << "Ключи сохранены.\n";
            } else if (choice == 2 || choice == 3) {
                const auto source = read_line("Исходный файл: ");
                const auto key = read_line(choice == 2 ? "Файл секретного ключа: " : "Файл открытого ключа: ");
                const auto signature = read_line(choice == 2 ? "Новый файл подписи: " : "Файл подписи: ");
                command(algorithm, {choice == 2 ? "sign" : "verify", source, key, signature});
            } else {
                std::cout << "Нет такого пункта.\n";
            }
        } catch (const std::exception& error) {
            if (std::cin.eof()) {
                std::cout << "\nВвод завершён.\n";
                return;
            }
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
} // namespace

int run(SignatureAlgorithm algorithm, int argc, char* argv[]) {
    try {
        if (argc == 1) {
            menu(algorithm);
            return 0;
        }
        if (argc == 2 && std::string(argv[1]) == "--help") {
            help(argv[0]);
            return 0;
        }
        return command(algorithm, std::vector<std::string>(argv + 1, argv + argc));
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
} // namespace signature_app
