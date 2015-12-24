#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *g_pidfile = 0; /* path to the pidfile */
char *g_root = NULL; /* path to the root */

/**
 * @brief Free global path strings
 */
void free_paths(void)
{
	free(g_pidfile);
	free(g_root);
	g_pidfile = g_root = NULL;
}

/**
 * @brief Determine the path to the root. If the command is in
 * <DESTDIR>/usr/bin or <DESTDIR>/bin, then the root is DESTDIR. Fallback
 * directory is "/".
 * @param [in] path The path to the command we are running
 * @return Path to root on success; NULL on error
 */
char *set_root(const char *path)
{
	char *root = NULL;
	const char *end = NULL;
	if (!path)
		goto out;
	if ((end = strstr(path, "/usr/bin")) != NULL) {
		if (path == end) {
			root = strdup("/");
		} else {
			root = strndup(path, end - path);
		}
	} else if ((end = strstr(path, "/bin")) != NULL) {
		if (path == end) {
			root = strdup("/");
		} else {
			root = strndup(path, end - path);
		}
	} else {
		root = strdup("/");
	}
out:
	return root;
}

/**
 * @brief Determine the path to the pidfile. Default pidfile
 * is <root>/var/run/<cmd>.pid.
 * @param [in] root Root path
 * @param [in] cmd Name of the command
 * @return Path to pidfile on success; NULL on error
 */
char *set_pidfile(const char *root, const char *cmd)
{
	char *path = NULL;
	if (!root || !cmd)
		goto out;
	if (strcmp(root, "/") == 0)
		root = "";
	asprintf(&path, "%s/var/run/%s.pid", root, cmd);
out:
	return path;
}
