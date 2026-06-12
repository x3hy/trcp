from cryptography.fernet import Fernet
from hashlib import sha256
from base64 import b64encode as base64
import cython

# Generate a fernet hash from the given key
cdef public bytes gen_safe_key(const char *key):
    return base64(sha256(key.encode("utf-8")).digest())

# Encrypts a given string
cdef public bytes encrypt_string(char *string, char *key):
    try:
        fernet = Fernet(key);
        return fernet.encrypt(string.encode())
    except:
        return None


# Decrypts a given string
cdef public bytes decrypt_string(char *string, char *key):
    try:
        fernet = Fernet(key);
        return fernet.decrypt(string)
    except:
        return None
