#include "wrapper.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	const char *password = "test123";
	const char *content = "test1234";

	char *result = c_encrypt_string(password, content);
	printf("%s\n", result);
	free(result);

	return 0;
}
