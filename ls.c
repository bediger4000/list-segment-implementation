#include <stdlib.h>    /* malloc/free */
#include <unistd.h>    /* write */
#include <sys/types.h> /* stat, getpwuid, getgrgid, *dir */
#include <sys/stat.h>  /* stat */
#include <time.h>      /* time, ctime */
#include <pwd.h>       /* getpwuid */
#include <grp.h>       /* getgrgid */
#include <dirent.h>    /* opendir, readdir, closedir */

void wrstr(int fd, char *string);
int strln(char *str);

unsigned int flags = 0;
#define LONG_OUTPUT    0x01
#define ALL_FILENAMES  0x02
#define RECURSIVE_LIST 0x04
#define REVERSE_SORT   0x08

int
main(int ac, char **av)
{
	int i;
	int fnames_idx = 0;

	for (i = 1; i < ac; ++i)
	{
		if ('-' == av[i][0])
		{
			switch (av[i][1])
			{
			case 'l':  /* long listing */
				break;
			case 'r':  /* reverse sort order */
				break;
			case 'R':  /* list subdirectories recursively */
				break;
			case 'a':  /* "all", do not ignore "." prefixed filenames */
				break;
			default:
				wrstr(2, "Bad option\n");
				break;
			}
		} else {
			fnames_idx = i;
			break;
		}
	}

	if (fnames_idx < ac && ac > 1)
	{
		wrstr(1, "Doing ls on these file:\n");
		for (;fnames_idx < ac; ++fnames_idx)
		{
			wrstr(1, av[fnames_idx]);
			wrstr(1, "\n");
		}
	} else {
		wrstr(1, "Doing ls on current dir\n");
	}

	return 0;
}

int
strln(char *str)
{
	int i = 0;
	if (str)
		while (str[i]) ++i;
	return i;
}

void
wrstr(int fd, char *string)
{
	if (string && fd >= 0)
		write(fd, string, strln(string));
}
