#include "subtitler/encryption/file_encryption.h"

#ifdef _MSC_VER
#define SODIUM_STATIC 1
#endif

extern "C" {
#include <sodium.h>
}

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace subtitler {
namespace encryption {
namespace {

const int CHUNK_SIZE = 4096;

void throwIfEmpty(const std::string& param_name, const std::string& value) {
    if (value.empty()) {
        throw std::invalid_argument{param_name + " cannot be empty!"};
    }
}

class SodiumInitializer {
  public:
    SodiumInitializer() {
        if (sodium_init() < 0) {
            throw std::runtime_error{"Could not initialize sodium"};
        }
    }
};

struct SaltedPassword {
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
    std::string password;
};

struct GeneratedKey {
    std::array<unsigned char, crypto_secretstream_xchacha20poly1305_KEYBYTES>
        key;
};

GeneratedKey keyDerivation(const SaltedPassword& salted) {
    GeneratedKey generated_key;
    auto& key = generated_key.key;
    auto& password = salted.password;
    if (crypto_pwhash(
            /* out= */ key.data(),
            /* outlen= */ key.size(),
            /* passwd= */ password.c_str(),
            /* passwdlen= */ password.length(),
            /* salt= */ salted.salt.data(),
            /* opslimit= */ crypto_pwhash_OPSLIMIT_INTERACTIVE,
            /* memlimit= */ crypto_pwhash_MEMLIMIT_INTERACTIVE,
            /* alg= */ crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error{"out of memory"};
    }

    return generated_key;
}

}  // namespace

void EncryptDataToFile(const std::string& output_path, const std::string& data,
                       const std::string& password) {
    throwIfEmpty("output_path", output_path);
    throwIfEmpty("data", data);
    throwIfEmpty("password", password);

    static SodiumInitializer init;

    std::ofstream output_file{output_path, std::ios_base::binary};
    if (!output_file) {
        throw std::runtime_error{"Could not open output file"};
    }

    // Generate random salt, and use password hash (expensive) to derive key.
    SaltedPassword salted_pwd;
    salted_pwd.password = password;
    randombytes_buf(salted_pwd.salt.data(), salted_pwd.salt.size());
    const auto key = keyDerivation(salted_pwd);

    // Write salt to the front of file.
    output_file.write((char*)salted_pwd.salt.data(), salted_pwd.salt.size());

    // Allocate buffers for io, headers.
    unsigned char buf_in[CHUNK_SIZE];
    unsigned char
        buf_out[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state st;

    // Write header to file.
    crypto_secretstream_xchacha20poly1305_init_push(&st, header,
                                                    key.key.data());
    output_file.write((char*)header, sizeof(header));

    // Encrypt the plaintext as a stream
    std::istringstream input_stream{data};
    while (input_stream) {
        input_stream.read((char*)buf_in, sizeof(buf_in));
        unsigned long long out_len = 0;
        // At the end of each block, if there is a subsequent block then
        // it is tagged 0. If this is the final block, then use FINAL tag.
        unsigned char tag =
            input_stream ? 0 : crypto_secretstream_xchacha20poly1305_TAG_FINAL;
        // Write encrypted output to file.
        crypto_secretstream_xchacha20poly1305_push(
            &st, buf_out, &out_len, buf_in, input_stream.gcount(), nullptr, 0,
            tag);
        output_file.write((char*)buf_out, out_len);
    }
}

std::string DecryptFromFile(const std::string input_path,
                            const std::string& password) {
    throwIfEmpty("input_path", input_path);
    throwIfEmpty("password", password);

    static SodiumInitializer init;

    std::ifstream input_file{input_path, std::ios_base::binary};
    if (!input_file) {
        throw std::runtime_error{"Could not open input file"};
    }

    SaltedPassword salted_pwd;
    // Extract the salt at front of the file.
    if (!input_file.read((char*)salted_pwd.salt.data(),
                         salted_pwd.salt.size())) {
        throw std::runtime_error{"Could not extract salt!"};
    }
    salted_pwd.password = password;
    // Re-derive the key from the password.
    const auto key = keyDerivation(salted_pwd);

    // Allocate buffers.
    unsigned char
        buf_in[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    unsigned char buf_out[CHUNK_SIZE];
    crypto_secretstream_xchacha20poly1305_state st;

    // Extract header
    input_file.read((char*)header, sizeof(header));
    if (crypto_secretstream_xchacha20poly1305_init_pull(&st, header,
                                                        key.key.data()) != 0) {
        throw std::runtime_error{"incomplete header"};
    }

    std::string decrypted_bytes;
    // Decrypt the bytes from the input file.
    while (input_file) {
        input_file.read((char*)buf_in, sizeof(buf_in));
        unsigned long long out_len = 0;
        unsigned char tag = 0;
        if (crypto_secretstream_xchacha20poly1305_pull(
                &st, buf_out, &out_len, &tag, buf_in, input_file.gcount(),
                nullptr, 0) != 0) {
            throw std::runtime_error{"corrupted chunk"};
        }
        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL &&
            input_file) {
            throw std::runtime_error{"premature end of file"};
        }
        decrypted_bytes += std::string{(char*)buf_out, out_len};
    }

    return decrypted_bytes;
}

}  // namespace encryption
}  // namespace subtitler
