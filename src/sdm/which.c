#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/**
 * @brief Given a command name, lookup the path to the command using PATH
 * @param [in] cmd The command name
 * @return Path to the executable when found; NULL when not found
 */
char *which(const char *cmd)
{
	char *file = NULL;
	char *token = NULL;
	char *tmp = NULL;
	struct stat buf;
	const char *path = NULL;
	path = getenv("PATH");
	if (!path)
		goto out;
	tmp = strdup(path);
	if (!tmp)
		goto out;
	token = strtok(tmp, ":");
	while(token != NULL) {
		asprintf(&file, "%s/%s", token, cmd);
		if (stat(file, &buf) == 0)
			break;
		token = strtok(NULL, ":");
		free(file);
		file = NULL;
	}
out:
	free(tmp);
	tmp = NULL;
	return file;
}
