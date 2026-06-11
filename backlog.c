#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BACKLOG_SIZE 10
static char *backlog[BACKLOG_SIZE];
int backlog_idx = 0;

void backlog_append(char *content);

#define print_backlog \
	for (int i = 0; i < backlog_idx; i++)\
		if (backlog[i] != NULL) \
			puts(backlog[i])

int main(){
	backlog_append("1");
	backlog_append("2");
	backlog_append("3");
	backlog_append("4");
	backlog_append("5");
	backlog_append("6");
	backlog_append("7");
	backlog_append("8");
	backlog_append("9");
	backlog_append("10");
	print_backlog;
	putchar('\n');

	backlog_append("11");
	print_backlog;
	return 0;
}

void backlog_append(char *content){
	if (content == NULL)
		return;

	// Backlog is full so we shuffle
	if (backlog_idx == BACKLOG_SIZE){
		free(backlog[0]);
		for (int i = 0; i < BACKLOG_SIZE-1; ++i){
			backlog[i] = (char *)malloc(strlen(backlog[i+1])*sizeof(char));
			strcpy(backlog[i], backlog[i+1]);
			free(backlog[i+1]);
			backlog[i+1] = NULL;
		}
	}
	
	backlog[backlog_idx] = (char *)malloc(strlen(content)*sizeof(char));
	strcpy(backlog[backlog_idx], content);
	backlog_idx++;
	return;
}
