#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "remote/plib/lib/ansi.h"
#include "remote/plib/lib/input.c"

// Variables
static char *buffer;
static int buffer_cap = 256;
static int _continue  =   1;
static struct winsize term;
static pthread_t ui;

// Functions
static void *ui_hook(void *);
static void ui_draw(void);
static void quit(int);


int
main()
{
	signal(SIGINT, quit);
	activate_terminal_buffer();
	pthread_create(&ui, NULL, ui_hook, NULL);

	
	// Initialize buffer
    buffer = malloc(buffer_cap);
	ui_hook((void *)1);

	printf("\033[%d;1H: ", term.ws_row);
	fgets(buffer, sizeof(buffer), stdin);

	// Clean up and exit 
    printf("You entered: %s", buffer);
	quit(0);
}

static void
quit(int a)
{
	_continue = 0;
	pthread_join(ui, NULL);
	deactivate_terminal_buffer();
    exit(a);
}

static void *
ui_hook( void * a)
{
	if(a == (void *)1){
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &term);
		return NULL;
	}

	for(int c = 0;!_continue; c++)
	{
		// update term values
		ui_hook((void *)1);

		// Draw ui and delay 
		ui_draw();
		fflush(stdout);
		sleep(1);
	}
}


static void
ui_draw(void)
{
	// draw messages
	printf("\033[1;1Htest");
	fflush(stdout);

	// draw msg box
}
