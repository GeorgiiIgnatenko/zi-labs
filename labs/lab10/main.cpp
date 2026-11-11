#include "../../common/signature_app.hpp"

int main(int argc, char* argv[]) {
    return signature_app::run(crypto::SignatureAlgorithm::gost94, argc, argv);
}
