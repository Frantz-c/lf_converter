#define ERROR	1
#define UNIX	0xf0
#define WINDOWS	0x0f

int		print_help(char *prog)
{
	printf("%s [option] file ...\n\n"
			"--windows || -w\n"
			"--unix || -u\n\n",
			prog);
}

int		get_option(char *s)
{
	if ((s[0] == '-' && s[1] == 'w' && s[2] == '\0')
			|| strcmp(s, "--windows") == 0)
	{
		return (WINDOWS);
	}
	if ((s[0] == '-' && s[1] == 'u' && s[2] == '\0')
			|| strcmp(s, "--unix") == 0)
	{
		return (UNIX);
	}
	fprintf(stderr, "Wrong argument %s\n", s);
	return (1);
}

void	replace_all_lf(char	*content, int *size, int opt)
{
	char	c = (opt == UNIX) ? '\r' : '\n';
	char	*lf;
	int		rem = *size;

	while ((lf = strchr(content, c)))
	{
		if (opt == WINDOWS)
		{
			memmove(lf + 1, lf, rem);
			*content = '\r';
			lf++;
			*size++;
		}
		else
		{
			memmove(lf, lf + 1, rem);
			*size--;
		}
		content = lf + 1;
	}
}

char	*file_get_contents(char *file, int *readl)
{
	static char	*content = NULL;
	static int	file_size = 0;
	const int	fd = open(file, O_RDONLY);
	struct stat	st;

	if (fd == -1)
	{
		fprintf(stderr, "can't open %s\n", file);
		return (NULL);
	}
	lstat(&st, file);
	if (file_size < st.st_size)
	{
		file_size = st.st_size;
		free(content);
		content = malloc(file_size);
	}
	read(fd, content, st.st_size);
	*readl = st.st_size;
	return (content);
}

char	*malloc_new_content(char *content, int size, int *new_size, int opt)
{
	static char	*new_content = NULL;
	static int	new_size = 0;
	int			lf_count = 0;

	if (opt == WINDOWS)
		lf_count = count_lf(content);
	*new_size = lf_count + size;
	return (malloc(*new_size));
}

void	replace_lf(int opt, char *file)
{
	char	*content;
	int		size;
	char	*new_content = NULL;
	int		new_size = 0;
	int		nlf = 0;
	
	if ((content = file_get_contents(file, &size)) == NULL)
		return;
	new_content = malloc_new_content(content, size, &new_size, opt);
	replace_all_lf(content, size, opt);
}

int		main(int ac, char *av[])
{
	int		opt;

	if (ac < 2)
		return (print_help(av[0]));
	if ((opt = get_option(av[1])) == ERROR)
		return (ERROR);
	for (av += 2; *av; av++)
	{
		replace_lf(opt, *av);
	}
}
