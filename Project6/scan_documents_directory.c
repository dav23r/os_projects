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

/*
 * generates html page of the given directory.
 * every entry of the directory is linken on itself for user.
 */
static char * generate_html(char *path, vector *entries)
{
	int last_allocated_size = 1024;
	char *res = (char *) malloc(last_allocated_size);
	assert(res);
	
	strcat(res, "<html>\n");
	strcat(res, "<head>\n");
	strcat(res, "<title>root</title>\n");
	strcat(res, "</head>\n");
	strcat(res, "<body>\n");
	int i;
	for (i = 0; i < VectorLength(entries); ++i)
	{
		char *entry_name = VectorNth(entries, i);
		char *url = strcat(strcat(path, "/"), entry_name);
		char *open_tag = strcat(strcat("<a href='", url), "'>");
		char *close_tag = strcat(entry_name, "</a>\n");
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
char * scan_and_print_directory(char *directory_path, bool save_html_file)
{	
	DIR *dir = opendir (directory_path);
	if (!dir) return NULL;
	
	struct dirent *read;
	vector root_entry_names;
	VectorNew(&root_entry_names, sizeof(char *), strings_vector_free_fn, 4);

	while ((read = readdir (dir)) != NULL)
		VectorAppend(&root_entry_names, &read->d_name);
		// puts (read->d_name);
	closedir (dir);
	
	char *html = generate_html(directory_path, &root_entry_names);
	if (save_html_file)
	{
		//here, '/document directory pages' is a directory where this kind of genarated htmls go
		FILE *fp = fopen(strcat("/document directory pages/", strcat(directory_path, ".html")), "w");
		fprintf(fp, html);
		fclose(fp);
		free(html);
		return NULL;
	}
	else
		return html;
}



