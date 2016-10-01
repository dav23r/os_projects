#ifndef __arguments__
#define __arguments__
/********* structures ********/
/* Represents an array of pointers to null-terminated strings */
typedef struct{
	char **arguments;	
        int num_args;
}pos_arguments;

/* Represents a closure for flag with it's value and it's own arguments */
typedef struct{
	char flag;
	pos_arguments *flag_arguments;
}flag;

/* Encapsulates all the data provided in the input line of command,
   including positional arguments, flags, and even positional arguments
   for particular flags.                                       */
typedef struct{
	pos_arguments *command_arguments;
	flag *flags;
	int num_flags;
}args_and_flags;

/********* functions *********/

// pos_arguments* getCommandArguments(args_and_flags *this);

#endif 
