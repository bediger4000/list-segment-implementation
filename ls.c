#include <stdio.h>
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

/* Doubly-linked so it can be sorted by filename */
struct filename_node {
	char *filename;
	struct stat sb;
	struct filename_node *prev;
	struct filename_node *next;
	struct filename_node *contents;
	int on_command_line;
};
struct filename_node *new_node(const char *filename);
void free_list(struct filename_node **list);
struct filename_node *push_filename(struct filename_node *head, const char *filename, const char *directoryname);
void process_filenames(struct filename_node **head);
int stat_filenames(struct filename_node *head);
void print_list(struct filename_node *head);
int wrstr(int fd, char *string);
int strln(const char *str);
int strcm(const char *s1, const char *s2);
char *strdp(const char *string);
struct filename_node *directory_filelist(const char *directory_name, const char *prefix);
void sort_list(struct filename_node **head);
int wrnumber(int fd, int number);
void print_long_output(struct filename_node *n);
void print_mode(mode_t st_mode);
void print_links(nlink_t st_nlink);
void print_owner(uid_t st_uid);
void print_owner(uid_t st_uid);
void print_group(gid_t st_gid);
void print_timestamp(struct timespec st_atim);
void print_size(off_t st_size);

unsigned int flags = 0;
#define LONG_OUTPUT    0x01
#define ALL_FILENAMES  0x02
#define RECURSIVE_LIST 0x04
#define REVERSE_SORT   0x08

int block_count = 0;

int
main(int ac, char **av)
{
	int i;
	int fnames_idx = 0;
	struct filename_node *head = NULL, *tail = NULL;

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
			tail = push_filename(tail, av[fnames_idx], "");
			if (!head) head = tail;
			tail->on_command_line = 1;
		}
	} else {
		head = directory_filelist(".", "");
	}

	process_filenames(&head);

	free_list(&head);

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
wrnumber(int fd, int number)
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
	n->contents = NULL;
	return n;
}

struct filename_node *
push_filename(struct filename_node *tail, const char *filename, const char *directoryname)
{
	struct filename_node *elem;
	char *qname;
	int fnln = strln(filename);
	int dnln = strln(directoryname);

	if (dnln > 0)
	{
		qname = malloc(dnln + 1 + fnln + 1);
		mcpy(qname, directoryname, dnln);
		qname[dnln] = '/';
		mcpy(&qname[dnln+1], filename, fnln);
		qname[dnln+1+fnln] = '\0';
	} else {
		qname = strdp(filename);
	}

	elem = new_node(qname);

	if (tail && elem)
	{
		elem->prev = tail;
		tail->next = elem;
	}
	return elem;
}

void
process_filenames(struct filename_node **head)
{
	if (stat_filenames(*head))
	{
		sort_list(head);
		print_list(*head);
	}
}

int
sort_criteria(const char *f1, const char *f2)
{
	int r;
	if (flags & REVERSE_SORT)
		r = 0 > strcm(f1, f2);
	else
		r = 0 < strcm(f1, f2);

	return r;
}

void
sort_list(struct filename_node **head)
{
	int looping = 1;

	while (looping)
	{
		struct filename_node *tmp = *head;
		looping = 0;
		
		while (tmp && tmp->next)
		{
			if (sort_criteria(tmp->filename, tmp->next->filename))
			{
				struct filename_node *a = tmp;
				struct filename_node *b = tmp->next;
				struct filename_node *b_next = b->next;
				struct filename_node *a_prev = a->prev;

				b->next = a;
				a->prev = b;

				if (a_prev)
					a_prev->next = b;
				b->prev = a_prev;

				if (b_next)
					b_next->prev = a;
				a->next = b_next;

				if (a == *head)
					*head = b;

				looping = 1;
			}
			if (tmp->contents)
				sort_list(&(tmp->contents));
			tmp = tmp->next;
		}
	}
}

int
strcm(const char *s1, const char *s2)
{
	int i, r = 0;

	if (!s1 || !s2)
		return r;

	for (i = 0; s1[i] && s2[i] && r == 0; ++i)
	{
		r = s1[i] - s2[i];
	}

	return r;
}

int
stat_filenames(struct filename_node *head)
{
	int worked = 0;
	struct filename_node *f;
	for (f = head; f; f = f->next)
	{
		if (lstat(f->filename, &f->sb))
		{
			wrstr(2, "Problem with stat(2) of \"");
			wrstr(2, f->filename);
			wrstr(2, "\"\n");
		} else {
			block_count += f->sb.st_blocks;
			++worked;
			if ((f->sb.st_mode & S_IFMT) == S_IFDIR)
			{
				if (f->on_command_line)
				{
					f->contents = directory_filelist(f->filename, f->filename);
					stat_filenames(f->contents);
				}
			}
		}
	}
	return worked;
}

void
print_list(struct filename_node *head)
{
	struct filename_node *f;
	if (flags & LONG_OUTPUT)
	{
		wrstr(1, "total ");
		wrnumber(1, block_count);
		wrstr(1, "\n");
	}
	for (f = head; f; f = f->next)
	{
		if (flags & LONG_OUTPUT)
		{
			print_long_output(f);
		}
		if (f->contents)
		{
			if (f->prev || f->next)
			{
				wrstr(1, f->filename);
				wrstr(1, ":\n");
			}
			print_list(f->contents);
		} else {
			wrstr(1, f->filename);
			wrstr(1, "\n");
		}
	}
}

struct filename_node *
directory_filelist(const char *directory_name, const char *prefix)
{
	struct filename_node *tail = NULL, *head = NULL;

	if (directory_name)
	{
		DIR *dirp = opendir(directory_name);
		struct dirent *entry;

		while (NULL != (entry = readdir(dirp)))
		{
			if (entry->d_name[0] == '.')
			{
				if (flags & ALL_FILENAMES)
					tail = push_filename(tail, entry->d_name, prefix);
			} else
				tail = push_filename(tail, entry->d_name, prefix);
			if (!head) head = tail;
		}

		closedir(dirp);
		dirp = NULL;
		tail = NULL;
	}

	return head;
}

void
print_long_output(struct filename_node *n)
{
	print_mode(n->sb.st_mode);
	print_links(n->sb.st_nlink);
	print_owner(n->sb.st_uid);
	print_group(n->sb.st_gid);
	print_size(n->sb.st_size);
	print_timestamp(n->sb.st_atim);
}

void
print_mode(mode_t st_mode)
{
	char *file_type = "-";
	switch (st_mode & S_IFMT)
	{
	case S_IFBLK:  file_type = "b"; break;
	case S_IFCHR:  file_type = "c"; break;
	case S_IFDIR:  file_type = "d"; break;
	case S_IFIFO:  file_type = "p"; break;
	case S_IFLNK:  file_type = "l"; break;
	case S_IFSOCK: file_type = "s"; break;
	}
	wrstr(1, file_type);
	if (st_mode & S_IRUSR)
		wrstr(1, "r");
	else
		wrstr(1, "-");
	if (st_mode & S_IWUSR)
		wrstr(1, "w");
	else
		wrstr(1, "-");
	if (st_mode & S_IXUSR)
		wrstr(1, "x");
	else
		wrstr(1, "-");
	if (st_mode & S_IRGRP)
		wrstr(1, "r");
	else
		wrstr(1, "-");
	if (st_mode & S_IWGRP)
		wrstr(1, "w");
	else
		wrstr(1, "-");
	if (st_mode & S_IXGRP)
		wrstr(1, "x");
	else
		wrstr(1, "-");
	if (st_mode & S_IROTH)
		wrstr(1, "r");
	else
		wrstr(1, "-");
	if (st_mode & S_IWOTH)
		wrstr(1, "w");
	else
		wrstr(1, "-");
	if (st_mode & S_IXOTH)
		wrstr(1, "x");
	else
		wrstr(1, "-");
	wrstr(1, " ");
}

void
print_links(nlink_t st_nlink)
{
	wrnumber(1, st_nlink);
	wrstr(1, " ");
}

void
print_owner(uid_t st_uid)
{
	struct passwd *pw;

	pw = getpwuid(st_uid);
	if (pw)
		wrstr(1, pw->pw_name);
	else
		wrnumber(1, st_uid);
	wrstr(1, " ");
}

void
print_group(gid_t st_gid)
{
	struct group *gr = getgrgid(st_gid);

	if (gr)
		wrstr(1, gr->gr_name);
	else
		wrnumber(1, st_gid);
	wrstr(1, " ");
}

void
print_size(off_t st_size)
{
	/* 2^33  = 8589934592, 10 places */
	off_t i  = 10000000000;  /* sizeof(off_t) matters here */
	while (st_size < i && i > 10)
	{
		wrstr(1, " ");
		i /= 10;
	}

	wrnumber(1, st_size);
	wrstr(1, " ");
}

void
print_timestamp(struct timespec st_atim)
{
	char *ts = ctime(&st_atim.tv_sec);
	ts[strln(ts)-1] = '\0'; /* stupid. */
	wrstr(1, ts);
	wrstr(1, " ");
}

void
free_list(struct filename_node **node)
{
	while (*node)
	{
		struct filename_node *tmp = (*node)->next;
		if ((*node)->contents) free_list(&(*node)->contents);
		free((*node)->filename);
		(*node)->filename = NULL;
		(*node)->prev = (*node)->next = NULL;
		free(*node);
		*node = tmp;
	}

	*node = NULL;
}
