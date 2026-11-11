#include "../../common/signature_tests.hpp"

int main() {
    return signature_tests::run(crypto::SignatureAlgorithm::elgamal);
}
