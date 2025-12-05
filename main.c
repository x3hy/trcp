#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/pem.h>


typedef struct {
	char *time;
	char *message;
	char *username;
} message;

typedef struct {
	struct {
		char *ip;
		int port;
		char *endpoint;
	} server;
	char *username;
} app_config;


static char *get_time(void);
static message msg_init(char *);
static void msg_print(message);
static void msg_free(message *);
static char *msg_url(message); 
static char *server_url(app_config);
static void free_app_config(app_config);
static char *base64_encode (const void *, size_t);
static char *base64_decode (const void *, size_t);


static app_config base;


#define b64e(c) base64_encode(c, strlen(c))
#define b64d(c) base64_decode(c, strlen(c)) 


int 
main()
{
	// Init app 
	base.username = strdup("tes123");
	base.server.ip = strdup("http://127.0.0.1");
	base.server.port = 5511;
	base.server.endpoint = strdup("msg");

	// Init message
	message test = msg_init("test123");
	msg_print(test);
	
	// Generate URL
	char * url = msg_url(test);
	printf("%s\n", url);

	// Clean up
	free(url);
	msg_free(&test);
	return 0;
}


// Returns UTC ISO date
static char *
get_time(void)
{
	time_t t = time(NULL);
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	gmtime_r(&t, &tm);

	// Get output size
	size_t out_s = snprintf(NULL, 0, "%04d-%02d-%02dT%02d:%02d:%02d",
			1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	char *out = (char *)malloc(out_s + 1);
	snprintf(out, out_s + 1, "%04d-%02d-%02dT%02d:%02d:%02d", 
			1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


	// Return the ISO date 
	return out;
}


// Initializes a message 
static message
msg_init(char *msg)
{
	message out;
	out.message = strdup(msg);
	out.time = get_time();
	out.username = base.username;
	return out;
}


// Print a messages values 
static void 
msg_print(message msg)
{
	printf("message: %s\ntime: %s\nusername: %s\n",
			msg.message, 
			msg.time,
			msg.username);
}


// Free message data 
static void
msg_free(message *msg)
{
	free(msg->message);
	free(msg->time);
	free(msg->username);
}

// Get the full server URL string
static char *
server_url(app_config app)
{
	char *out;
	size_t out_s = snprintf(NULL, 0, "%s:%d/%s",
			app.server.ip,
			app.server.port,
			app.server.endpoint);

	out = malloc(out_s + 1);

	snprintf(out, out_s + 1, "%s:%d/%s", 
			app.server.ip,
			app.server.port,
			app.server.endpoint);

	return out;
}



// Generate a URL for posting message to server 
static char *
msg_url(message msg)
{
	char *out;

	char *msg_username = b64e(msg.username);
	char *msg_time = b64e(msg.time);
	char *msg_message = b64e(msg.message);


	char *serv = server_url(base);
	size_t out_s = snprintf(NULL, 0, "%s/%s/%s/%s",
			serv,
			msg_username,
			msg_time,
			msg_message);

	out = malloc(out_s + 1);

	snprintf(out, out_s + 1, "%s/%s/%s/%s",
			serv,
			msg_username,
			msg_time,
			msg_message);
	return out;
}

// Free app_config memory 
static void 
free_app_config(app_config app)
{
	free(app.username);
	free(app.server.ip);
	free(app.server.endpoint);
}


char *base64url_encode(const unsigned char *data, size_t input_length) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    BUF_MEM *buffer_ptr;

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    BIO_write(bio, data, input_length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    // Duplicate and modify to make URL-safe
    char *url_safe = malloc(buffer_ptr->length + 1);
    memcpy(url_safe, buffer_ptr->data, buffer_ptr->length);
    url_safe[buffer_ptr->length] = '\0';

    for (size_t i = 0; i < buffer_ptr->length; ++i) {
        if (url_safe[i] == '+') url_safe[i] = '-';
        else if (url_safe[i] == '/') url_safe[i] = '_';
        else if (url_safe[i] == '=') { url_safe[i] = '\0'; break; } // strip padding
    }

    // Clean up
    BIO_free_all(bio);

    return url_safe; // Remember to free() when done!
}

char *base64url_decode(const char *input, size_t *out_len) {
    size_t len = strlen(input);
    char *clean_input = malloc(len + 4);

    size_t i = 0, j = 0;
    while (i < len) {
        char c = input[i++];
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
        clean_input[j++] = c;
    }

    // Add missing padding
    while (j % 4 != 0) clean_input[j++] = '=';
    clean_input[j] = '\0';

    // Now decode normally using your existing function or similar
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(clean_input, -1);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_push(b64, mem);

    char *output = malloc((len * 3) / 4 + 1);
    int decoded_len = BIO_read(mem, output, len * 3 / 4 + 1);
    output[decoded_len] = '\0';

    if (out_len) *out_len = decoded_len;

    BIO_free_all(mem);
    free(clean_input);

    return output;
}
