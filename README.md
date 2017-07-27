# Task
Develop `ls`, with the -l -r -R and -a options, and
with only malloc, free, write, stat, ctime, time, getpwuid, getgrgid,
readdir, opendir and closedir.

From [Speedrunning code, the 3 things I re-learnt while recoding a basic ls in C](https://medium.com/@poilon/speedrunning-code-the-3-things-i-re-learnt-while-recoding-a-basic-ls-in-c-d559b0f1a92b)

## Results

I ended up with a mostly functional `ls` in 526 lines of uncommented
ANSI C89. I did not implement the "-R" recursive listing. My `ls`
differs a little from GNU `ls` with respect to output file names
when invoked like `ls somedirectory`.

I worked on this from July 22, 12:30pm to about July 26, 7:15pm,
which is just about 4.25 days. I did ordinary things along with
that coding, like job, sleeping, household chores, cooking.

After subtracting sleeping, job, etc, I guess I spent between
12 and 16 hours on this. I believe the original author said
he spent 5 hours on it. He ended up with 520 lines of C.
I did not look at his code until after I finished mine, but
there's a remarkable similarity to the designs. We both used
linked lists to hold file names and then process the lists.

The nodes of the list are remarkably similar, too.

Assus's:

    struct ChainedList
    {
      struct ChainedList *next;
      char *file_name;
      struct stat *stats;
    };

Mine:

    struct filename_node {
        char *filename;
        struct stat sb;
        struct filename_node *prev;
        struct filename_node *next;
        struct filename_node *contents;
        int on_command_line;
    };

I used a doubly-linked list so I could sort (and reverse sort for "-r"
flag) the list easier. We even had some functions named the same:
`sort_list()`, `free_list()`, `print_list()`.


