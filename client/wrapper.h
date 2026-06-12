#ifndef FERNET_WRAP
#define FERNET_WRAP

#include <Python.h>
#include <stdlib.h>
#include <string.h>

#include "fernet.h"

#ifdef __cplusplus
extern "C" {
#endif

static char* c_encrypt_string(const char* key, const char* content)
{
    if (key == NULL || content == NULL) return NULL;

    PyObject* safe_key = NULL;
    PyObject* result = NULL;
    char* output = NULL;

    safe_key = gen_safe_key(key);
    if (safe_key == NULL) goto cleanup;

    result = encrypt_string(content, safe_key);
    if (result && PyBytes_Check(result)) {
        const char* data = PyBytes_AsString(result);
        if (data) output = strdup(data);
    }

cleanup:
    Py_XDECREF(safe_key);
    Py_XDECREF(result);
    return output;
}

static char* c_decrypt_string(const char* key, const char* content)
{
    if (key == NULL || content == NULL) return NULL;

    PyObject* safe_key = NULL;
    PyObject* result = NULL;
    char* output = NULL;

    safe_key = gen_safe_key(key);
    if (safe_key == NULL) goto cleanup;

    result = decrypt_string(content, safe_key);
    if (result && PyBytes_Check(result)) {
        const char* data = PyBytes_AsString(result);
        if (data) output = strdup(data);
    }

cleanup:
    Py_XDECREF(safe_key);
    Py_XDECREF(result);
    return output;
}

#ifdef __cplusplus
}
#endif

#endif
