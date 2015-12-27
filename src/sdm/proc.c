#define _GNU_SOURCE
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int check_proc(pid_t pid)
{
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

pid_t lookup_pid(const char *cmd, const char *path)
{
	char name[NAME_MAX];
	char exe[PATH_MAX];
	char *file = NULL;
	DIR *dir = NULL;
	FILE *fp = NULL;
	struct dirent *ent = NULL;
	struct stat st_path, st_exe;
	pid_t pid = -1, rv = -1;
	dir = opendir("/proc");
	while ((ent = readdir(dir)) != NULL) {
		pid = atoi(ent->d_name);
		if (pid < 1)
			continue;
		free(file);
		if (fp)
			fclose(fp);
		/* make sure we have the correct process name */
		asprintf(&file, "/proc/%d/status", pid);
		fp = fopen(file, "r");
		if (!fp)
			goto out;
		if (fscanf(fp, "%*s %s", name) != 1)
			goto out;
		if (strcmp(name, cmd) != 0) {
			continue;
		}
		free(file);
		/* make sure this is the correct executable */
		asprintf(&file, "/proc/%d/exe", pid);
		bzero(exe, sizeof(exe));
		if (readlink(file, exe, PATH_MAX-1) == -1)
			goto out;
		if (stat(path, &st_path) != 0)
			goto out;
		if (stat(exe, &st_exe) != 0)
			goto out;
		if (st_path.st_dev == st_exe.st_dev && st_path.st_ino == st_exe.st_ino) {
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
