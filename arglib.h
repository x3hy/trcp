#ifndef ARGLIB_H
#define ARGLIB_H

// Basic strlen implementation.
static int _arglib_strlen(char *str){
	if (!str) return 0;
	int str_s = 0;
	while (str[str_s])
		str_s++;
	return str_s+1;
}

/* checks if a given argument matches based on
 * a splitchar. */
static int _is_arg_match(char *pattern, char *arg, char splitchar){
	const int arg_len = _arglib_strlen(arg);
	if (arg[0] == '*')
		return 0;

	// Locate the splitchar within the argument
	int split = 0;
	while (arg[split]){
		if (arg[split] == splitchar)
			break;
		split++;
	}

	// Save a copy of arg to undobuf
	char undobuf[arg_len];
	for (int i = 0; i < arg_len; i++)
		undobuf[i] = arg[i];

	// Separate the key and value from the argument
	char key[arg_len - split];
	for (int i = 0; i < arg_len; i++){
		if (i <= (split != arg_len-1 ? split : arg_len)){
			key[i] = arg[i];
			if (split && i == split) key[i] = '\0';
			else if (i == arg_len) arg[0] = '\0';
		}

		// Copy JUST the value back to arg
		if (split != arg_len -1)
			arg[i] = arg[i + split + 1];
	}

	/* now we have the following:
	 * key: first half of the argument (before the splitchar)
	 * arg: last half of the argument (after the splitchar) */
	for (int i = 0; i < _arglib_strlen(pattern); i++)
		if (pattern[i] != key[i]){
			// reset the argument using undobuf
			for (int i = 0; i < arg_len; i++)
				arg[i] = undobuf[i];
			return 1;
		}
	return 0;
}

static int  __arg_exitcode       =  0;
static char __arg_splitchar      = '=';
static int  __arg_show_help      =  0;
static int  __arg_help_len       =  0;
static int  __arg_help_has_count =  1;

// Loops through arguments given
#define ARGIDX __arg_i
#define ARGLAST (argc-1)
#define ARGFIRST 0
#define IS_ARGLAST (ARGIDX == argc-1)
#define IS_ARGFIRST (ARGIDX == 0)
#define ARGNEXT continue

#define FORARGS \
	for (int ARGIDX = 1; ARGIDX < argc; ARGIDX++)

// gets the value index from argv
#define ARGVAL argv[ARGIDX]

#include <stdio.h> // fputs putchar stdout

// Help menu backend
static int _arg_show_help_menu(char *arg, char *desc){
	const int arg_len = _arglib_strlen(arg);

	if (__arg_show_help){
		fputs(arg, stdout);

		for (int i = 0; i < __arg_help_len - arg_len; i++)
			// Print padding to line up descriptions.
			putchar(' ');
		
		// Separator
		fputs(" | ", stdout);
		fputs(desc, stdout);
		putchar('\n');

	/* if the current argument is larger than the
	 * __arg_help_len then make it equal to the
	 * larger value. */
	} else if (__arg_help_has_count == 0){
	 	if (__arg_help_len < arg_len)
			__arg_help_len = arg_len;
		return 1;
	}

	return __arg_show_help;
}

// Help menu instructions
#define _HELP(func, ac, av) \
	do { \
		if (__arg_help_len == 0){ \
			__arg_help_has_count = 0; \
			func(ac, av); \
			__arg_help_has_count = 1; \
		} \
		__arg_show_help = 1; \
		func(ac, av); \
		__arg_show_help = 0; \
	} while (0)

#define HELP(func) \
	_HELP(func, 2, NULL)

/* Uses a switch case to allow for inline code execution.
 * the `switch` line also assigns __arg_success the return
 * code of _is_arg_match which checks if a given argument
 * matches the given pattern. */
static char *arg_name;
static char *arg_desc;
#define ARG(arg, desc) \
	arg_name = arg; \
	arg_desc = desc; \
	if (!_arg_show_help_menu(arg, desc)) \
		switch(__arg_exitcode = _is_arg_match(arg, \
			ARGVAL, __arg_splitchar)) \
			case 0:

/* same as ARG just any arg works if given BUT it must be at a certain
 * idx. */
#define POSANY(n, arg, desc) \
		if (!_arg_show_help_menu(arg, desc) && ARGIDX == n) \
			switch(0) \
				case 0:

#endif //ARGLIB_H
