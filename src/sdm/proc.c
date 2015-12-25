#define _GNU_SOURCE
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int check_proc(pid_t pid)
{
	printf("checking proc %d\n", pid);
	return kill(pid, 0);
}

/**
 * @brief Read the pid from the pidfile. Assume the pidfile only
 * has one pid in it.
 * @param [in] file The path to the pidfile
 * @return The process id; -1 on failure
 */
pid_t read_pidfile(const char *path)
{
	pid_t pid = -1;
	FILE *fp = NULL;
	if (!path)
		goto out;
	fp = fopen(path, "r");
	if (!fp)
		goto out;
	if (fscanf(fp, "%d", &pid) != 1)
		goto out;
out:
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	return pid;
}

pid_t lookup_pid(const char *cmd)
{
	char name[NAME_MAX];
	char *file = NULL;
	DIR *dir = NULL;
	FILE *fp = NULL;
	struct dirent *ent = NULL;
	pid_t pid = -1, rv = -1;
	dir = opendir("/proc");
	while ((ent = readdir(dir)) != NULL) {
		pid = atoi(ent->d_name);
		if (pid < 1)
			continue;
		free(file);
		if (fp)
			fclose(fp);
		asprintf(&file, "/proc/%d/status", pid);
		fp = fopen(file, "r");
		if (!fp)
			goto out;
		if (fscanf(fp, "%*s %s", name) != 1)
			goto out;
		if (strcmp(name, cmd) == 0) {
			rv = pid;
			break;
		}
	}
out:
	free(file);
	if (dir)
		closedir(dir);
	if (fp)
		fclose(fp);
	file = NULL;
	dir = NULL;
	fp = NULL;
	return rv;
}
