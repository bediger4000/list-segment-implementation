#include <stdlib.h>    /* malloc/free */
#include <unistd.h>    /* write */
#include <sys/types.h> /* stat, getpwuid, getgrgid, *dir */
#include <sys/stat.h>  /* stat */
#include <time.h>      /* time, ctime */
#include <pwd.h>       /* getpwuid */
#include <grp.h>       /* getgrgid */
#include <dirent.h>    /* opendir, readdir, closedir */

#ifdef NULL
#undef NULL
#endif
#define NULL ((void*)0)

struct filename_node {
	char *filename;
	struct stat sb;
	struct filename_node *prev;
	struct filename_node *next;
};

struct filename_node *new_node(const char *filename);
struct filename_node *push_filename(struct filename_node *head, const char *filename);
void process_filenames(struct filename_node *head);
int stat_filenames(struct filename_node *head);
void print_list(struct filename_node *head);
int wrstr(int fd, char *string);
int strln(const char *str);
char *strdp(const char *string);
struct filename_node *directory_filelist(const char *directory_name);
int wrnumber(int fd, int number, int base);

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
	struct filename_node *head = NULL, *list = NULL;

	for (i = 1; i < ac; ++i)
	{
		if ('-' == av[i][0])
		{
			switch (av[i][1])
			{
			case 'l':  /* long listing */
				flags |= LONG_OUTPUT;
				break;
			case 'r':  /* reverse sort order */
				flags |= REVERSE_SORT;
				break;
			case 'R':  /* list subdirectories recursively */
				flags |= RECURSIVE_LIST;
				break;
			case 'a':  /* "all", do not ignore "." prefixed filenames */
				flags |= ALL_FILENAMES;
				break;
			default:
				wrstr(2, "Bad option: ");
				wrstr(2, av[i]);
				wrstr(2, "\n");
				break;
			}
		} else {
			fnames_idx = i;
			break;
		}
	}

	if (fnames_idx && fnames_idx < ac && ac > 1)
	{
		for (; fnames_idx < ac; ++fnames_idx)
		{
			if (av[fnames_idx][0] == '.')
			{
				if (flags & ALL_FILENAMES)
					list = push_filename(list, av[fnames_idx]);
			} else
				list = push_filename(list, av[fnames_idx]);
			if (!head) head = list;
		}
	} else {
		list = directory_filelist(".");
		head = list;
	}

	process_filenames(head);

	while (list)
	{
		struct filename_node *tmp = list->next;
		free(list->filename);
		list->filename = NULL;
		list->prev = list->next = NULL;
		free(list);
		list = tmp;
	}

	return 0;
}

int
strln(const char *str)
{
	int i = 0;
	if (str)
		while (str[i]) ++i;
	return i;
}

void
mcpy(char *dst, const char *src, int count)
{
	if (dst && src)
	{
		int i;
		for (i = 0; i < count; ++i)
			dst[i] = src[i];
	}
}

int
wrstr(int fd, char *string)
{
	int cc = 0;
	if (string && fd >= 0)
		cc = write(fd, string, strln(string));
	return cc;
}

int
wrnumber(int fd, int number, int base)
{
	char buf[11];
	int place = 9;

	buf[10] = '\0';

	if (number < 0)
	{
		wrstr(fd, "-");
		number = -number; /* -2147483648 == -(-2147483648) */
	}

	if (number == 0)
	{
		return wrstr(fd, "0");
	}

	while (number > 0)
	{
		int r = number%10;
		number /= 10;
		buf[place--] = r + '0';
	}
	return wrstr(fd, &buf[place+1]);
}

char *
strdp(const char *string)
{
	char *dupe = NULL;
	if (string)
	{
		int l = strln(string) + 1;
		dupe = malloc(l);
		mcpy(dupe, string, l);
		
	}
	return dupe;
}

struct filename_node *
new_node(const char *filename)
{
	struct filename_node *n = malloc(sizeof(*n));
	n->prev = n->next = NULL;
	n->filename = strdp(filename);
	return n;
}

struct filename_node *
push_filename(struct filename_node *list, const char *filename)
{
	struct filename_node *elem = new_node(filename);
	if (list && elem)
	{
		elem->next = list;
		list->prev = elem;
	}
	return elem;
}

void
process_filenames(struct filename_node *head)
{
	if (stat_filenames(head))
		print_list(head);
}

int
stat_filenames(struct filename_node *head)
{
	int worked = 0;
	struct filename_node *f;
	for (f = head; f; f = f->prev)
	{
		if (stat(f->filename, &f->sb))
		{
			wrstr(2, "Problem with stat(2) of \"");
			wrstr(2, f->filename);
			wrstr(2, "\"\n");
		} else {
			++worked;
		}
	}
	return worked;
}

void
print_list(struct filename_node *head)
{
	struct filename_node *f;
	for (f = head; f; f = f->prev)
	{
		wrstr(1, f->filename);
		wrstr(1, "\t");
		wrnumber(1, f->sb.st_size, 10);
		wrstr(1, "\n");
	}
}

struct filename_node *
directory_filelist(const char *directory_name)
{
	struct filename_node *list = NULL, *head = NULL;

	if (directory_name)
	{
		DIR *dirp = opendir(directory_name);
		struct dirent *entry;

		while (NULL != (entry = readdir(dirp)))
		{
			if (entry->d_name[0] == '.')
			{
				if (flags & ALL_FILENAMES)
					list = push_filename(list, entry->d_name);
			} else
				list = push_filename(list, entry->d_name);
			if (!head) head = list;
		}

		closedir(dirp);
		dirp = NULL;
	}

	return head;
}
