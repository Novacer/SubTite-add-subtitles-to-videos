#ifndef SUBTITLER_ENCRYPTION_FILE_ENCRYPTION_H
#define SUBTITLER_ENCRYPTION_FILE_ENCRYPTION_H

#include <stdexcept>
#include <string>

namespace subtitler {
namespace encryption {

/**
 * Encrypts data using symmetric key encryption. Password is used to derive
 * a key, and the ciphertext (along with other metadata) is written to the
 * output_path file.
 *
 * Use DecryptFromFile() to retrieve the data.
 *
 * It is assumed (for now) that data can fit entirely in memory, but this
 * can be extended to handle large files in the future.
 *
 * @param output_path the output file to write the encrypted data.
 * @param data the plaintext payload.
 * @param password the password used to derive the secret key.
 */
void EncryptDataToFile(const std::string& output_path, const std::string& data,
                       const std::string& password);

/**
 * Decrypts the data from EncryptDataToFile(). Password is used to derive the
 * same secret key as the encryption. Throws DecryptionWrongPassword
 * if the password was wrong.
 *
 * @param input_path the input file to decrypt.
 * @param password the password used to derive the secret key.
 * @return std::string the plaintext data.
 */
std::string DecryptFromFile(const std::string input_path,
                            const std::string& password);

}  // namespace encryption
}  // namespace subtitler

#endif
