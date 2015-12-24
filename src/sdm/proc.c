#include <signal.h>
#include <stdio.h>

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
