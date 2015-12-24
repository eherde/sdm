#ifndef OPTIONS_H
#define OPTIONS_H
/**
 * @brief Container for command line options
 */
typedef struct options_t
{
	char *cmd;     /* The command by name */
	char *path;    /* The command by path */
	int help;      /* --help was passed */
	int start;     /* --start was passed */
	int stop;      /* --stop was passed */
} options;
#endif
