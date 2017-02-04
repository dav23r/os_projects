#include "scan_documents_directory.h"
#include "vector.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>

void strings_vector_free_fn(void *elem)
{
	free(*(char **)elem);
}

static int count_files_in_dir(char *dirpath)
{
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir(dirpath); 
	while ((entry = readdir(dirp)) != NULL) {
        file_count++;
	}
	closedir(dirp);
	return file_count;
}

char *replace(char *str)
{
	char *res =malloc(strlen(str)+1);
	int i;
	for (i=0;i<strlen(str); i++)
	{
		if (i > 0 && str[i] == '/') res[i] = '+';
		else res[i] = str[i];
	}
	res[strlen(str)] = '\0';
	return res;
}

/*
 * generates html page of the given directory.
 * every entry of the directory is linken on itself for user.
 * if (entry->d_type == DT_REG) check  fot directory, meybe useful next time
 */
static char * generate_html(char *path, vector *entries)
{
	printf("%d\n", VectorLength(entries));
	int last_allocated_size = 1024;
	char *res = (char *) malloc(last_allocated_size);
	res[0] = '\0';
	assert(res);
	strcat(res, "<html>\n");
	strcat(res, "<head>\n");
	strcat(res, "<title>root</title>\n");
	strcat(res, "</head>\n");
	strcat(res, "<body>\n");
	char link[256];
	//printf("%s\n", link);
	int i;
	for (i = 0; i < VectorLength(entries); ++i)
	{
		if (strcmp(*(char **)VectorNth(entries, i), ".") == 0 || strcmp(*(char **)VectorNth(entries, i), "..") == 0) continue;
		link[0] = '\0';
		//strcpy(link, host);
		strcpy(link, path);
		char *entry_name = *(char **)VectorNth(entries, i);
		if (link[strlen(link)-1] != '/') strcat(link, "/");
		char *url = strcat(link, entry_name);
		char open_tag[128];open_tag[0] = '\0';
		strcat(strcat(strcat(open_tag, "<a href=\""), url), "\">");
		char entry_name_copy[strlen(entry_name) + 10]; entry_name_copy[0] = '\0';
		strcpy(entry_name_copy, entry_name);
		char *close_tag = strcat(entry_name_copy, "</a><br>\n");
		char *whole_tag = strcat(open_tag, close_tag);
		if (strlen(res) + strlen(whole_tag) + 20 >= last_allocated_size)	// 20 is about num of chars needed for closing last two tags bellow

		{
			last_allocated_size *= 2;
			res = (char *) realloc((void *) res, last_allocated_size);
		}
		strcat(res, whole_tag);
	}
	strcat(res, "</body>\n");
	strcat(res, "</html>");

	return res;
}

// should be called when server starts
char * scan_and_print_directory(char *directory_path, char *doc_root, bool save_html_file)
{
	printf("88888888888888888888888888888888888888888888888888888 - %s\n", directory_path);
	DIR *dir = opendir (directory_path);
	if (!dir) return NULL;
	struct dirent *read;
	vector root_entry_names;
	VectorNew(&root_entry_names, sizeof(char **), strings_vector_free_fn, 4);
	int count = count_files_in_dir(directory_path), i = 0;
	printf("%d\n", count);
	char *arr[count];

	while ((read = readdir (dir)) != NULL && i++ < count) {
		arr[i] = strdup(read->d_name);
		VectorAppend(&root_entry_names, &arr[i]);
	}
		// puts (read->d_name);
	closedir (dir);

	char *html = generate_html(remove_prefix(directory_path, doc_root), &root_entry_names);
	printf("html - %s\n", html);
	if (save_html_file)
	{
		//here, '/document directory pages' is a directory where this kind of genarated htmls go
		char dest_dir[128]; dest_dir[0] = '\0';
		strcpy(dest_dir, "document_directory_pages");
		char *filename = replace(directory_path);
		strcat(strcat(dest_dir, filename), ".html");
		free(filename);
		FILE *fp = fopen(dest_dir, "w");
		if (!fp) {
			perror("Unable to locate new html file");
			exit(0);
		}
		fprintf(fp, "%s", html);
		fclose(fp);
		free(html);
		VectorDispose(&root_entry_names);
		return NULL;
	}
	else
		return html;
}

static char * remove_prefix(char *str, char *prefix)
{
	printf("%s-%s\n", str, prefix);
	if (strlen(prefix) > strlen(str)) return NULL;
	int i;
	for (i = 0; i < strlen(prefix)-1; i++, str++);
	return str;
}
