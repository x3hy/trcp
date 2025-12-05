#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


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
static char* base64_encode(const unsigned char *, size_t);

		
static app_config base;
static const unsigned char base64_table[65] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

	free(url);
	
	char* base64 = base64_encode((const unsigned char *)"test123", 8);
	printf("%s\n",base64);
	free(base64);

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


	// Return the iso date 
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

// Generate a url for posting message to server 
static char *
msg_url(message msg)
{
	char *out;

	char *serv = server_url(base);
	size_t out_s = snprintf(NULL, 0, "%s/%s/%s/%s",
			serv,
			msg.username,
			msg.time,
			msg.message);

	out = malloc(out_s + 1);

	snprintf(out, out_s + 1, "%s/%s/%s/%s",
			serv,
			msg.username,
			msg.time,
			msg.message);

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

// Source: Jouni Malinen (Converted from C++)
static char *
base64_encode(const unsigned char *src, size_t len)
{
	unsigned char *out, *pos;
	const unsigned char *end, *in;

	size_t olen;

	olen = 4*((len +2) / 3);

	if(olen < len)
		return NULL;

	char * outStr;
	outStr = malloc(olen + 1);
	out = (unsigned char *)&outStr[0];

	end = src + len;
	in = src;
	pos = out;
	while(end - in >= 3)
	{
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
	}

	if(end - in)
	{
		*pos++ = base64_table[in[0] >> 2];
		
		if(end - in == 1)
		{
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos = '=';
		} else { 
			*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}

		*pos++ = '=';
	}

	return outStr;
}
