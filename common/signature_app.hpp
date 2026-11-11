#pragma once
#include "crypto.hpp"

namespace signature_app {
int run(crypto::SignatureAlgorithm algorithm, int argc, char* argv[]);
}
