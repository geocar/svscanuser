#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

extern int trylock(int fd);

static void 
#ifdef __GNUC__
inline
#endif
doit(const char *username, const char *dir, uid_t u)
{
	struct stat sb;
	char *execline[] = {
		"runas", "xxx",
		"noroot",
		"chdirhome",
		"subsvscan", ".service",
		0,
	};
	int fd, r;

	if (chdir(dir) == -1) return;
	if (lstat(".service", &sb) == -1) return;
	if (S_ISLNK(sb.st_mode)) return;
	if (chdir(".service") == -1) return;
	if (stat(".", &sb) == -1) return;
	if (sb.st_uid != u) return;
	if (sb.st_mode & 022) return;
	if (stat("..", &sb) == -1) return;
	if (sb.st_uid != u) return;
	if (sb.st_mode & 02) return;

	fd = open(".lock", O_RDWR);
	if (fd != -1) {
		r = trylock(fd);
		(void)close(fd);
		if (r < 1) return;
	}

	if (fork() == 0) {
		execline[1] = (void*)username;
		execvp(*execline, execline);
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	struct passwd *pw;

	signal(SIGCHLD, SIG_IGN);
	if (getuid()) exit(1);
	for (;;) {
		setpwent();
		while ((pw = getpwent())) {
			if (!pw->pw_uid) continue;
			if (!pw->pw_dir) continue;
			if (!pw->pw_name) continue;
			if (!*pw->pw_name) continue;
			if (!*pw->pw_dir) continue;
			if (*pw->pw_dir != '/') continue;
			doit(pw->pw_name, pw->pw_dir, pw->pw_uid);
		}
		endpwent();
		sleep(5);
	}
}
