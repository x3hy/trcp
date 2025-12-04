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
	char *server_ip;
	char *endpoint;
	char *username;
} app_config;


static char *get_time(void);
static message msg_init(char *);
static void msg_print(message);
static void msg_free(message *);

static app_config app;

int 
main()
{
	// Init app 
	app.username = strdup("tes123");

	// Init message
	message test = msg_init("test123");
	msg_print(test);
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
	snprintf(out, out_s, "%04d-%02d-%02dT%02d:%02d:%02d", 
			1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	// Return the iso date 
	return out;
}


// Initializes a message 
message
msg_init(char *msg)
{
	message out;
	out.message = msg;
	out.time = get_time();
	out.username = app.username;
	return out;
}


// Print a messages values 
void 
msg_print(message msg)
{
	printf("message: %s\ntime: %s\nusername: %s\n",
			msg.message, 
			msg.time,
			msg.username);
}

// Free message data 
void
msg_free(message *msg)
{
	free(msg->message);
	free(msg->time);
	free(msg->username);
} 
