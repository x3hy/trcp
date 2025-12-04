#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	char *time;
	char *message;
	char *username;
	char *token; // May not need 
} message;


static char *get_time(void);


int 
main()
{
	char * cur_date = get_time();
	
	printf("date is: %s\n", cur_date);
	free(cur_date);
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
