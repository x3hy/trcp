#include "remote/cjson/cJSON.h"
#include "remote/plib/lib/input.h"
#include "remote/plib/lib/ansi.h"
#include "remote/plib/plib.h"
#include <curl/curl.h>
#include <openssl/pem.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

// largest message size
// 127 characters
#define MSG_SIZE 128 

typedef struct {
  char *time;
  char *message;
  char *username;
} message;

struct mem {
  char *mem;
  size_t size;
};

typedef struct {
  struct {
    char *ip;
    int port;
    struct {
			struct {
				char *msg_count;
				char *msg;
				char *ping;
			} GET;
			struct {
				char *msg;
				char *user_check;
			} POST;
    } endpoint;
  } server;
  char *username;
} app_config;

typedef struct {
  char *message;
  int code;
} msg_return;

static char *get_time(void);
static message msg_init(char *);
static void msg_print(message);
static void msg_free(message *);
static char *msg_url(message, app_config);
static char *server_url(app_config);
static void free_app_config(app_config);
static char *base64_encode(const unsigned char *, size_t);
static char *base64_decode(const char *, size_t *);
static size_t write_callback(void *, size_t, size_t, void *);
static cJSON *get_url_json(const char *);
static msg_return post_msg(message, app_config);
static char * server_url_at_point(app_config, char*);
static void message_listen_loop(app_config);
static void ui_update(void);


static app_config base;
input global_in;


#define b64e(c) base64_encode((unsigned char *)c, strlen(c))
#define b64d(c) base64_decode((unsigned char *)c, strlen(c))

// quit handler
void 
quit(int a)
{
	_in_no_raw();
	disable_mouse_reporting_ansi();
	deactivate_terminal_buffer();
	show_cursor();
	
	free_app_config(base);
	fflush(stdout);

	exit(a);
}

void 
quit_callback(void *a)
{
	quit(0);
}


int
main(int argc, char * argv[])
{
	signal(SIGINT, quit);
	pl_arg *p_help = PL_A("--help", "Show this dialog", .short_flag = "-h");
	pl_arg *p_user = PL_A("--username","Set username", .takes_value = 1, .short_flag = "-u",  .required = 1);
	pl_arg *p_ip = PL_A("--host", "Set host", .takes_value = 1, .short_flag = "-i", .required = 1);
	pl_arg *p_port = PL_A("--port", "Set port", .takes_value = 1, .short_flag = "-p", .required = 1);
	pl_r plib_r;

	if((plib_r = PL_PROC()) != PL_SUCCESS)
	{
		PL_E_INFO(plib_r);
		pl_help();
		return 1;
	}

	if(atoi(PL_G(p_port)) == 0 && strcmp(PL_G(p_port), "0") != 0)
	{
		fprintf(stderr, "Port '%s' is invalid..\n", PL_G(p_port));
		return 1;
	}
	
  // Init app (point of no return)
	base.username = strdup(PL_G(p_user));
	base.server.ip = strdup(PL_G(p_ip));
	base.server.port = atoi(PL_G(p_port));
  base.server.endpoint.POST.msg = strdup("post_msg");
	base.server.endpoint.POST.user_check = strdup("user_check");
  base.server.endpoint.GET.msg = strdup("get_msg");
	base.server.endpoint.GET.msg_count = strdup("msg_count");
	base.server.endpoint.GET.ping = strdup("ping");
	
	// Enable misc ANSI
	_in_raw();
	enable_mouse_reporting_ansi();
	activate_terminal_buffer();
	hide_cursor();
	fflush(stdout);

	in_key('Q', quit_callback);
	
	// app is initialized!
	in_loop()
	{
		in_catch(&global_in);
		in_update(&global_in);
		ui_update();
		
		// handle text input  
		if(global_in.type == KEY)
		{
		}
	}
	
	// Clean up 
	return 0;
}

static void 
ui_update(void)
{
	struct winsize s;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &s);
	
	clear();
	gotoxy(1, 1);
	
	border b = (border){
		.top = "━",
		.bottom = "━",
		.left = "┃",
		.right = "┃",
		.top_right = "┓",
		.top_left = "┏",
		.bottom_right = "┛",
		.bottom_left = "┗"
	};

	box view = UI_BOX(VEC(s.ws_col, s.ws_row-1), VEC(0, 0), .ansi = "", .border = b, .fill = ' ');
	
	printf("%s\033[0m\n",view._r);
	printf(":prompt goes here");

	fflush(stdout);
}

// Returns UTC ISO date
static char
*get_time(void)
{
  time_t t = time(NULL);
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  gmtime_r(&t, &tm);

  // Get output size
  size_t out_s =
      snprintf(NULL, 0, "%04d-%02d-%02dT%02d:%02d:%02d",
					1900 + tm.tm_year,
        	tm.tm_mon + 1,
					tm.tm_mday,
					tm.tm_hour,
					tm.tm_min,
					tm.tm_sec);

  char *out = (char *)malloc(out_s + 1);
  snprintf(out, out_s + 1, "%04d-%02d-%02dT%02d:%02d:%02d",
			1900 + tm.tm_year,
      tm.tm_mon + 1,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec);

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
  out.username = strdup(base.username);
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
  size_t out_s = snprintf(NULL, 0, "%s:%d", 
			app.server.ip, 
			app.server.port);

  out = malloc(out_s + 1);

  snprintf(out, out_s + 1, "%s:%d", 
			app.server.ip, 
			app.server.port);

  return out;
}

static char *
server_url_at_point(app_config app, char* endpoint)
{
	char * serv = server_url(app);

	char *out;
	size_t out_s = snprintf(NULL, 0, "%s/%s",
			serv,
			endpoint);

	out = malloc(out_s + 1);

	snprintf(out, out_s + 1, "%s/%s",
			serv,
			endpoint);

	// Clean up
	free(serv);
	return out;
}

// Generate a URL for posting message to server
static char *
msg_url(message msg, app_config app)
{
  char *out;

  char *msg_username = b64e(msg.username);
  char *msg_time = b64e(msg.time);
  char *msg_message = b64e(msg.message);

  char *serv = server_url(base);
  size_t out_s = snprintf(NULL, 0, "%s/%s/%s/%s/%s", 
			serv, 
			app.server.endpoint.POST.msg, 
			msg_username, 
			msg_time,
      msg_message);

  out = malloc(out_s + 1);

  snprintf(out, out_s + 1, "%s/%s/%s/%s/%s", 
			serv, 
			app.server.endpoint.POST.msg, 
			msg_username, 
			msg_time,
      msg_message);

  free(msg_username);
  free(msg_time);
  free(msg_message);
  return out;
}


// await a new message
static void 
message_listen_loop(app_config app)
{
	char *get_n_url = server_url_at_point(app, app.server.endpoint.GET.msg_count);
	while(1)
	{
		cJSON *resp = get_url_json(get_n_url);
		cJSON *msg = cJSON_GetObjectItemCaseSensitive(resp, "message");
		const int count = msg->valueint;
	}

	free(get_n_url);
}


// Free app_config memory
static void
free_app_config(app_config app)
{
	free(app.username);
	free(app.server.ip);
	free(app.server.endpoint.POST.msg);
	free(app.server.endpoint.POST.user_check);
	free(app.server.endpoint.GET.msg);
	free(app.server.endpoint.GET.msg_count);
	free(app.server.endpoint.GET.ping);
}

static char *
base64_encode(const unsigned char *data, size_t input_length)
{
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
    else if (url_safe[i] == '=') {
      url_safe[i] = '\0';
      break;
    }
  }

  // Clean up
  BIO_free_all(bio);
  return url_safe;
}

static char *
base64_decode(const char *input, size_t *out_len)
{
  size_t len = strlen(input);
  char *clean_input = malloc(len + 4);

  size_t i = 0, j = 0;
  while (i < len) {
    char c = input[i++];
    if (c == '-')
      c = '+';
    else if (c == '_')
      c = '/';
    clean_input[j++] = c;
  }

  // Add missing padding
  while (j % 4 != 0)
    clean_input[j++] = '=';
  clean_input[j] = '\0';

  // Now decode normally using your existing function or similar
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *mem = BIO_new_mem_buf(clean_input, -1);
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  mem = BIO_push(b64, mem);

  char *output = malloc((len * 3) / 4 + 1);
  int decoded_len = BIO_read(mem, output, len * 3 / 4 + 1);
  output[decoded_len] = '\0';

  if (out_len)
    *out_len = decoded_len;

  BIO_free_all(mem);
  free(clean_input);

  return output;
}

static size_t 
write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct mem *mems = (struct mem *)userp;

  char *ptr = realloc(mems->mem, mems->size + realsize + 1);
  if (!ptr) {
    fprintf(stderr, "realloc() failed\n");
    return 0;
  }

  mems->mem = ptr;
  memcpy(&(mems->mem[mems->size]), contents, realsize);
  mems->size += realsize;
  mems->mem[mems->size] = 0;

  return realsize;
}

static cJSON *
get_url_json(const char *url)
{
  CURL *curl = NULL;
  CURLcode res;
  struct mem chunk = {.mem = NULL, .size = 0};
  cJSON *root = NULL;

  chunk.mem = malloc(1);
  if (!chunk.mem)
    return NULL;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl) {
    free(chunk.mem);
    return NULL;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl-cjson-example/1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  } else {
    root = cJSON_Parse(chunk.mem);
    if (!root) {
      const char *error_ptr = cJSON_GetErrorPtr();
      if (error_ptr)
        fprintf(stderr, "JSON Error before: %s\n", error_ptr);
      else
        fprintf(stderr, "Failed to parse JSON (NULL returned)\n");
    }
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
  free(chunk.mem);

  return root;
}

static msg_return
post_msg(message msg, app_config app)
{
  char *url = msg_url(msg, app);
  cJSON *resp = get_url_json(url);

  cJSON_AddArrayToObject(resp, "message");
  cJSON *message = cJSON_GetObjectItemCaseSensitive(resp, "message");
  cJSON *code = cJSON_GetObjectItemCaseSensitive(resp, "code");

  if (!cJSON_IsString(message) || !cJSON_IsNumber(code)) {
    fprintf(stderr, "server returned invalid data!\n");
    return (msg_return){0};
  }

  msg_return out;
  out.message = strdup(message->valuestring);
  out.code = code->valueint;

  cJSON_Delete(resp);
  free(url);

  return out;
}


int achar(){
	static struct termios oldt, newt;
	tcgetattr(STDIN_FILENO,&oldt);
	newt=oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO,TCSANOW,&newt);
	int ch=getchar();
	tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
	return ch;
}
