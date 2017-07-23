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

void 

int
main(int ac, char **av)
{
	int i;
	int fnames_adx = 0;

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
				write(2, "Bad option\n", 11);
				break;
			}
		} else {
			fnames_idx = ac;
			break;
		}
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
