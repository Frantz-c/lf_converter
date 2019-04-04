/*
 * compiler avec -DWINDOWS_OS pour windows et prier pour que ca marche...
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define ERROR		1
#define UNIX		0xf0
#define WINDOWS		0x0f
#define BACKUP_DIR	"backup_directory_0xffe8001c__zsj"

#ifdef WINDOWS_OS
	# define DIR_SEP		'\\'
#else
	# define DIR_SEP		'/'
#endif

int		print_help(char *prog)
{
	printf("%s [option] file ...\n\n"
			"--windows, -w\n"
			"--unix, -u\n\n",
			prog);
	return (0);
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

int		replace_all_lf(char	*content, char *new_content, int opt)
{
	char	c = (opt == UNIX) ? '\r' : '\n';
	char	*lf;
	int		offset = 0;
	int		len;

	while (*content && (lf = strchr(content, c)))
	{
		if (opt == WINDOWS)
		{
			if (lf[-1] == '\r')
			{
				strncpy(new_content + offset, content, lf - content + 1);
				offset += (lf - content) + 1;
				//fprintf(stderr, "Error: '\\r' detected, stop.\n");
				//exit(1);
			}
			else
			{
				strncpy(new_content + offset, content, lf - content);
				offset += (lf - content) + 2;
				new_content[offset - 2] = '\r';
				new_content[offset - 1] = '\n';
			}
		}
		else
		{
			if (lf[1] != '\n')
				strncpy(new_content + offset, content, lf - content + 1);
			else
				strncpy(new_content + offset, content, lf - content);
			offset += (lf - content);
		}
		content = lf + 1;
	}

	if (offset == 0)
	{
		fprintf(stderr, "Error: no '\\n' detected, stop.\n");
		exit(1);
	}
	len = strlen(content);
	strncpy(new_content + offset, content, len);
	offset += len;
	return (offset);
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
	lstat(file, &st);
	if (file_size < st.st_size)
	{
		file_size = st.st_size;
		free(content);
		content = malloc(file_size + 1);
	}
	read(fd, content, st.st_size);
	close(fd);
	content[st.st_size] = '\0';
	*readl = st.st_size;
	return (content);
}

int		count_lf(char *s)
{
	char	*tmp;
	int		count = 0;

	if (*s == '\n')
		s++;
	while (*s && (tmp = strchr(s, '\n')))
	{
		if (s[-1] != '\r')
			count++;
		s = tmp + 1;
	}
	return (count);
}

char	*malloc_new_content(char *content, int content_size, int opt)
{
	static char	*new_content = NULL;
	static int	new_size = 0;
	int			lf_count = 0;

	if (opt == WINDOWS)
		lf_count = count_lf(content);
	if (new_size < content_size + lf_count)
	{
		free(new_content);
		new_size = lf_count + content_size;
		return (new_content = malloc(new_size));
	}
	return (new_content);
}

char	*get_backup_name(const char *name)
{
	char		*new;
	const char	*start = name;
	static int	uniq_no = 0;

	name += strlen(name) - 1;
	while (name > start && *name != DIR_SEP)
		name--;
	new = malloc(strlen(name) + 11 + strlen(BACKUP_DIR) + 1);
	sprintf(new, BACKUP_DIR "%c%s_0x%x", DIR_SEP, name, uniq_no);
	uniq_no++;
	return (new);
}

void	replace_lf(int opt, char *file)
{
	char	*content;
	char	*new_content;
	int		size;
	int		new_size;
	char	*backup_name;
	
	if ((content = file_get_contents(file, &size)) == NULL)
		return;
	new_content = malloc_new_content(content, size, opt);
	new_size = replace_all_lf(content, new_content, opt);
	backup_name = get_backup_name(file);
	rename(file, backup_name);
	int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd == -1)
	{
		rename(backup_name, file);
		fprintf(stderr, "Error 0x01\n");
		exit(1);
	}
	write(fd, new_content, new_size);
	close(fd);
}

int		main(int ac, char *av[])
{
	int		opt;

	if (ac < 2)
		return (print_help(av[0]));
	if ((opt = get_option(av[1])) == ERROR)
		return (ERROR);
	mkdir(BACKUP_DIR, 0777);
	for (av += 2; *av; av++)
	{
		replace_lf(opt, *av);
	}
	return (0);
}
