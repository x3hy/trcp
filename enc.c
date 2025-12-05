#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <string.h>

unsigned char *encrypt_message(const unsigned char *key_32bytes, const unsigned char *plaintext, size_t plaintext_len, size_t *out_len) {
  unsigned char nonce[12];
  RAND_bytes(nonce, 12);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
  EVP_EncryptInit_ex(ctx, NULL, NULL, key_32bytes, nonce);

  int len;
  unsigned char *ciphertext = malloc(plaintext_len);
  EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
  int ciphertext_len = len;

  EVP_EncryptFinal_ex(ctx, ciphertext + len, &len); // usually 0
  ciphertext_len += len;

  unsigned char tag[16];
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);

  // Prepend nonce, append tag
  unsigned char *output = malloc(12 + ciphertext_len + 16);
  memcpy(output, nonce, 12);
  memcpy(output + 12, ciphertext, ciphertext_len);
  memcpy(output + 12 + ciphertext_len, tag, 16);

  *out_len = 12 + ciphertext_len + 16;
  free(ciphertext);
  EVP_CIPHER_CTX_free(ctx);
  return output;
}

unsigned char *decrypt_message(const unsigned char *key_32bytes,const unsigned char *encrypted,size_t encrypted_len,size_t *out_plaintext_len) {
  if (encrypted_len < 28)
    return NULL; // too short

  const unsigned char *nonce = encrypted;
  const unsigned char *ciphertext = encrypted + 12;
  size_t ciphertext_len = encrypted_len - 28;
  const unsigned char *tag = encrypted + encrypted_len - 16;

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
  EVP_DecryptInit_ex(ctx, NULL, NULL, key_32bytes, nonce);

  unsigned char *plaintext = malloc(ciphertext_len);
  int len;
  EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
  int plaintext_len = len;

  // Set expected tag before finalize
  EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)tag);

  int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
  EVP_CIPHER_CTX_free(ctx);

  if (ret > 0) {
    *out_plaintext_len = plaintext_len + len;
    return plaintext;
  } else {
    // Authentication failed or error
    free(plaintext);
    return NULL;
  }
}
