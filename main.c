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


char *base64_encode (const void *b64_encode_this, size_t encode_this_many_bytes){
    BIO *b64_bio, *mem_bio;
    BUF_MEM *mem_bio_mem_ptr;
    b64_bio = BIO_new(BIO_f_base64());
    mem_bio = BIO_new(BIO_s_mem());
    BIO_push(b64_bio, mem_bio);
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64_bio, b64_encode_this, encode_this_many_bytes);
    BIO_flush(b64_bio);
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);
    BIO_set_close(mem_bio, BIO_NOCLOSE);
    BIO_free_all(b64_bio);
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';
    return (*mem_bio_mem_ptr).data;
}

char *base64_decode (const void *b64_decode_this, size_t decode_this_many_bytes){
    BIO *b64_bio, *mem_bio;
    char *base64_decoded = calloc( (decode_this_many_bytes*3)/4+1, sizeof(char) );
    b64_bio = BIO_new(BIO_f_base64());
    mem_bio = BIO_new(BIO_s_mem());
    BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes);
    BIO_push(b64_bio, mem_bio);
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_byte_index = 0;
    while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ){
        decoded_byte_index++;
    }
    BIO_free_all(b64_bio);
    return base64_decoded;
}

