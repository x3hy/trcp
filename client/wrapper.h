#ifndef FERNET_WRAP
#define FERNET_WRAP

#include <Python.h>
#include "fernet.h"
#include <string.h>

static char *c_encrypt_string(const char *key, const char *content){
	char *output = NULL;

	// Generate the bytes key from the given key
	PyObject *safe_key = gen_safe_key(key);
	if (safe_key == NULL)
		goto cleanup;

	// Encrypt the string
	PyObject *result = encrypt_string(content, safe_key);
	if (result && PyBytes_Check(result)){
		// Transfer it to the output string
		const char *data = PyBytes_AsString(result);
		if (data)
			output = strdup(data);

	}

	// Deactivate python
cleanup:
	Py_DECREF(result);
	Py_DECREF(safe_key);
	return output;
}


#endif
