extern "C" {

#include <sodium.h>

}

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define CHUNK_SIZE 4096

namespace {

struct SaltedPassword {
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
    std::string password;
};

struct GeneratedKey {
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
    std::array<unsigned char, crypto_secretstream_xchacha20poly1305_KEYBYTES>
        key;
};

GeneratedKey keyDerivation(const SaltedPassword& salted_pwd) {
    GeneratedKey key;
    key.salt = salted_pwd.salt;
    if (crypto_pwhash(key.key.data(), key.key.size(),
                      salted_pwd.password.c_str(), salted_pwd.password.length(),
                      key.salt.data(), crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error{"out of memory"};
    }
    return key;
}

void encrypt(const std::string& file_name, const std::string& plaintext,
             const std::string& password) {
    SaltedPassword salted_pwd;
    salted_pwd.password = password;
    randombytes_buf(salted_pwd.salt.data(), salted_pwd.salt.size());
    const auto key = keyDerivation(salted_pwd);

    unsigned char buf_in[CHUNK_SIZE];
    unsigned char
        buf_out[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state st;

    std::ofstream output_file{file_name, std::ios_base::binary};
    if (!output_file) {
        throw std::runtime_error{"Could not open output file"};
    }

    // Write salt at front of file.
    output_file.write((char*)key.salt.data(), key.salt.size());

    crypto_secretstream_xchacha20poly1305_init_push(&st, header,
                                                    key.key.data());
    output_file.write((char*)header, sizeof(header));
    std::istringstream input_stream{plaintext};

    while (input_stream) {
        input_stream.read((char*)buf_in, sizeof(buf_in));
        unsigned long long out_len = 0;
        unsigned char tag =
            input_stream ? 0 : crypto_secretstream_xchacha20poly1305_TAG_FINAL;
        crypto_secretstream_xchacha20poly1305_push(
            &st, buf_out, &out_len, buf_in, input_stream.gcount(), nullptr, 0,
            tag);
        output_file.write((char*)buf_out, out_len);
    }
}

std::string decrypt(const std::string& file_name, const std::string& password) {
    std::ifstream input_file{file_name, std::ios_base::binary};
    if (!input_file) {
        throw std::runtime_error{"Could not open input file"};
    }
    SaltedPassword salted;
    // Extract the salt at front of the file.
    if (!input_file.read((char*)salted.salt.data(), salted.salt.size())) {
        throw std::runtime_error{"Could not extract salt!"};
    }
    salted.password = password;
    const auto key = keyDerivation(salted);

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
        decrypted_bytes +=
            std::string{(char*)buf_out, out_len};
    }

    return decrypted_bytes;
}

}  // namespace

int main() {
    if (sodium_init() != 0) {
        return 1;
    }

    std::string plain_text = "This is some plaintext. blah blah!";
    encrypt("outfile.txt", plain_text, "password123");

    std::string decrypted = decrypt("outfile.txt", "password123");
    std::cout << decrypted << std::endl;

    return 0;
}
