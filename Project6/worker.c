#include "worker.h"
#include <assert.h>
#include "response_builder.h"
#include "config_service.h"
#include "cgi_runner.h"
#include "scan_documents_directory.h"
#include "log_service.h"
#include <sys/stat.h>
#include "webserver.h"
#include <time.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>


void * work(void *config)
{
	// A single event we use as scratch space
	struct epoll_event *event;
	//ertze meti arasodes gvinda rom amovighot, ert threads erti descriptor
	event = (struct epoll_event *)calloc(1, sizeof(struct epoll_event));
	assert(event);

	while (true) {
		//daucdis sanam ar mova rame
		printf("epoll waiting...\n");
		epoll_wait(epoll_fd, event, 1, -1);
		Data *dat = (Data *)event[0].data.ptr;

		if(event[0].events & EPOLLIN) {
			proccess_request(dat->fd, (hashset *)config);
		}
		close(dat->fd);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, dat->fd, event);
	}
	return NULL;
}

static void proccess_request(int in_fd, hashset *config)
{
	if (in_fd < 0) return;
	while (true)
	{
		printf("processor got fd=%d\n", in_fd);
		int *a = (int *) malloc(sizeof(int));
		*a = 3;
		printf("%d\n", *a);
		char request[BUFFER_SIZE], response[BUFFER_SIZE];
		memset(request,0,BUFFER_SIZE);
		memset(response,0,BUFFER_SIZE);
		int bytes_recieved, status_code_sent, sent_content_len;
		if ((bytes_recieved = recv(in_fd, request, BUFFER_SIZE, 0)) <= 0) break;
		printf("%d\n", bytes_recieved);
		printf("new request: %slen=%d\n", request, strlen(request));
		struct connect_time_and_ip time_and_ip_log;
		printf("`````````````````\n");
		time_and_ip_log.connect_time = get_formatted_datetime();
		printf("`````````````````\n");
		char header[BUFFER_SIZE];
		memset(header,0,BUFFER_SIZE);
		get_header(request, header);

		struct header_info parsed_header;
		parsed_header.host = get_header_value(header, "Host");
		printf("after hosttttttttttttttttttttttttt = %d\n", strlen(header));
		time_and_ip_log.Ip_address = get_config_value(parsed_header.host, "ip", config);

		char *document_root = get_config_value(parsed_header.host, "documentroot", config);
		char *cgi_bin = get_config_value(parsed_header.host, "cgi-bin", config), *error_msg = NULL;
		bool error_occured, file_send = false;
		enum log_type log_level = ACCESSLOG;
		if ((strcmp(document_root, NO_KEY_VALUE) != 0 && strlen(document_root) > 0) ||
			((strcmp(cgi_bin, NO_KEY_VALUE) != 0 && strlen(cgi_bin) > 0)))
		{
			/* here also we get requested filename and extension if type is 'FILE', or program name and content-info if CGI */
			printf("beforeeeeeeeeeeeeeeeeeeeeeee = %s-%d\n", header, strlen(header));
			parsed_header.method = get_request_method_and_type(header, &parsed_header);
			printf("afterrrrrrrrrrrrrrrrrrrrr = %s-%d\n", header, strlen(header));

			parsed_header.etag = get_header_value(header, "Etag");
			parsed_header.keep_alive = keep_alive(header);
			parsed_header.range = get_header_range(header);

			if (parsed_header.cgi_or_file == STATIC_FILE || parsed_header.cgi_or_file == DIR)
			{
				if (strcmp(document_root, NO_KEY_VALUE) == 0 || strlen(document_root) == 0)
				{
					add_initial_header(response, "HTTP/1.1 404 Not Found", strlen(response));
					status_code_sent = 404;
				}
				else
				{
					char *file_path = parsed_header.cgi_or_file == DIR ? get_dir_page_path(document_root, parsed_header.requested_objname) : strcat(document_root, parsed_header.requested_objname + 1);

					char *file_new_hash = compute_file_hash(file_path);
					if (parsed_header.etag && strcmp(parsed_header.etag, file_new_hash) == 0)
					{
						assert(0);
						add_initial_header(response, "HTTP/1.1 304 Not Modified", strlen(response));
						status_code_sent = 304;
					}
					else
					{
						char count_str[12], content_type[36];
						memset(count_str,0,12); memset(content_type,0,36);
						count_str[0] = '\0', content_type[0] = '\0';
						printf("%s\n", file_path);
						FILE *fp = fopen(file_path, "r");
						if (fp)
						{
							off_t offset = parsed_header.range->start;
							int file_size = get_file_size(fp);
							sent_content_len = parsed_header.range->end - parsed_header.range->start < 0 ? file_size : parsed_header.range->end - parsed_header.range->start + 1;
							status_code_sent = parsed_header.range->end - parsed_header.range->start < 0 ? 200 : 206;
							sprintf(count_str, "%d", sent_content_len);
							detect_content_type(content_type, parsed_header.ext);
							add_header_key_value(response, "Content-Type", content_type);
							add_header_key_value(response, "Content-Length", count_str);

							if (status_code_sent == 206) {
								add_header_key_value(response, "Accept-Ranges", "bytes");
								char content_range_str[49];
								sprintf(content_range_str, "bytes %d-%d/%d", parsed_header.range->start, parsed_header.range->end, file_size);
								add_header_key_value(response, "Content-Range", content_range_str);
							}
							add_header_key_value(response, "Cache-Control", "max-age=5");
							add_header_key_value(response, "etag", file_new_hash);
							if (status_code_sent == 200) add_initial_header(response, "HTTP/1.1 200 OK", strlen(response));
							else add_initial_header(response, "HTTP/1.1 206 Partial Content", strlen(response));
							//int out_fd = fileno(fp);
							int out_fd = open(file_path, O_RDONLY);
					        if (out_fd == -1)
					        {
					            exit(EXIT_FAILURE);
					        }
					        strcat(response, "\n");
							send(in_fd, response, strlen(response), 0);
							//ssize_t bytes = sendfile(in_fd, out_fd, &offset, sent_content_len);
					        int remain_data = sent_content_len, sent_bytes;
					        /* Sending file data */
					        while (((sent_bytes = sendfile(in_fd, out_fd, &offset, BUFFER_SIZE)) > 0) && (remain_data > 0))
					        {
					                fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
					                remain_data -= sent_bytes;
					                fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
					        }
							file_send = true;
							fclose(fp);
						} else {
							add_initial_header(response, "HTTP/1.1 404 Not Found", strlen(response));
							status_code_sent = 404;
							add_body(response, "<html><head></head><body><p>404 - NOT FOUND</body></html>");
						}
					}
					//free(file_path);
					//free(file_new_hash);
				}
			}
			else // CGI
			{
				if (strcmp(cgi_bin, NO_KEY_VALUE) == 0 || strlen(cgi_bin) == 0)
				{
					add_initial_header(response, "HTTP/1.1 404 Not Found", strlen(response));
					status_code_sent = 404;
				}
				else
				{
					parsed_header.content_type = get_header_value(header, "Content-Type");
					parsed_header.content_length = get_header_value(header, "Content-Length");
					parsed_header.path_info = strdup(parsed_header.requested_objname);
					parsed_header.requested_objname = strcat(cgi_bin, parsed_header.requested_objname + 1); // +1 not to have two slashes (//)
					run_cgi_script(&parsed_header, in_fd, get_config_block(parsed_header.host, config));
				}
			}
		}
		else	// i.e. host is not valid or neither document and cgi-bin directories aren't provided in config file
		{
			add_initial_header(response, "HTTP/1.1 404 Not Found", strlen(response));
			status_code_sent = 404;
		}

		if (strlen(response) != 0 && !file_send){ // i.e. response is sent from cgi
			send(in_fd, response, strlen(response), 0);
		}
		printf("request - %s\n", request);
		printf("response - %s\n", response);
		printf("%d\n", parsed_header.keep_alive);
		struct accesslog_params *log;
		if (!error_occured) log = build_log_data(time_and_ip_log, parsed_header.host, parsed_header.requested_objname, status_code_sent, sent_content_len, get_header_value(header, "User-Agent"));
		else log = build_error_log(time_and_ip_log, error_msg);
		log_request(log_level, log, get_config_value(parsed_header.host, "log", config));
		log_struct_dispose(log_level, log);
		bool keep_alive = parsed_header.keep_alive;
		printf("%d\n", keep_alive);
		//header_info_dispose(&parsed_header);
		printf("processor cycle doneeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n");
		if (keep_alive){ printf("keeped alive\n"); set_keep_alive(in_fd);}
		else break;
	}
}

static void get_header(char *request, char *header)
{
	int i;
	for (i = 0; i < BUFFER_SIZE - 1; i++)
	{
		if (request[i] == '\n' && request[i+1] == '\n')
		{
			request[i] = '\0';
			break;
		}
		header[i] = request[i];
	}
}

// returns request method, also sets request type in passed header struct
static enum http_method get_request_method_and_type(char *header, struct header_info *header_struct)
{
	header = strdup(header);
	enum http_method ret = UNDEFINED;
	bool method_set = false;
	char *initial_line, *token;
	initial_line = strtok (header, "\n");
	token = strtok (initial_line, " ");
	while (token != NULL)
	{
		if (method_set)
		{
			char *token_copy = strdup(token);
			header_struct->query_string = check_for_query_string(token_copy);
			char *ext = get_filename_extension(token_copy);

			if (strcmp(ext, "html") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "mp4") == 0)
				header_struct->cgi_or_file = STATIC_FILE;
			else if (strcmp(ext, "/") == 0)
				header_struct->cgi_or_file = DIR;
			else header_struct->cgi_or_file = CGI;

			header_struct->requested_objname = token_copy;
			header_struct->ext = ext;
			return ret;
		}
		else
		{
			if (!strcmp(token, "GET"))
			{
				ret = GET;
				method_set = true;
			}
			if (!strcmp(token, "POST"))
			{
				ret = POST;
				method_set = true;
			}
		}
		token = strtok (NULL, " ");
	}

	return ret;
}

static char *get_header_value(char *header, char *key)
{

	char *token, tmp[strlen(key)+2], header_copy[strlen(header)+1];
	tmp[0] = '\0'; header_copy[0] = '\0';
	strcpy(tmp, key);
	strcat(tmp, ":");
	strcpy(header_copy, header);
	printf("key = %s\n", tmp);
	token = strtok(header_copy, " \n");
	while (token != NULL)
	{
		printf("tokennnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn key = %s\n", token);
		if (!strcmp(token, tmp)){
			char *res = strtok(NULL, "\n");
			res[strlen(res)-1] = '\0';
			return strdup(res);
		}
		token = strtok(NULL, " \n");
	}
	return NULL;
}

static bool keep_alive(char *header)
{
	char *value = get_header_value(header, "Connection");
	printf("1111111111111111 - %s\n", value);
	if (!value || strcmp(value, "keep-alive") != 0)
		return false;

	return true;
}

static struct range_info * get_header_range(char *header)
{
	struct range_info *res = (struct range_info *) malloc(sizeof(struct range_info));
	char *value = get_header_value(header, "range");

	// full content requested
	if (!value || strlen(value) == 0)
	{
		res->start = 0;
		res->end = -1;
	}
	else
	{
		char *token;
		token = strtok(value, "- ");
		if (token && strlen(token) > 0)
		{
			res->start = atoi(token);

			if ((token = strtok(NULL, "- ")) != NULL && strlen(token) > 0)
				res->end = atoi(token);
			else
				res->end = -1;	// i.e. by the end
		}
	}
	return res;
}

static char * compute_file_hash(char *full_path)
{
	char res[128], tmp[24];
	res[0] = '\0', tmp[0] = '\0';
	struct stat attr;
	stat(full_path, &attr);

	sprintf(tmp, "%u_%c", (unsigned)attr.st_size, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld_%c", (long)attr.st_mtime, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld%c", (long)attr.st_atime, '\0');
	strcat(res, tmp);

	return strdup(res);
}

static char * check_for_query_string(char *token_copy)
{
	char *question_mark = strrchr(token_copy, '?');
	if (question_mark)
	{
		*question_mark = '\0';
		return strdup(question_mark + 1);
	}
	return NULL;
}

static char *get_filename_extension(char *file_path)
{
	const char *ext;
	if (file_path[strlen(file_path)-1] == '/')
		ext = file_path + strlen(file_path)-1;
	else
	{
		const char *last_dot = strrchr(file_path, '.');
		if(!last_dot || last_dot == file_path)
			return NULL;
		else
			ext = last_dot + 1;
	}
    return (char *)ext;
}

static void detect_content_type(char *content_type, const char *ext)
{
	if (strcmp(ext, "html") == 0 || strcmp(ext, "/") == 0)
		strcpy(content_type, "text/html");
	else if (strcmp(ext, "mp4") == 0)
		strcpy(content_type, "video/mp4");
	else if (strcmp(ext, "jpg") == 0)
		strcpy(content_type, "image/jpeg");
}

static long int get_file_size(FILE *stream)
{
	if(!stream)  {
		perror ("Error opening file");
		return(-1);
	}
	fseek(stream, 0, SEEK_END);

	long int len = ftell(stream);
}

static char * get_dir_page_path(char *document_root, char *dir_name)
{
	char doc_root_copy_2[strlen(document_root)+ strlen(dir_name) + 16];
	doc_root_copy_2[0] = '\0';
	strcpy(doc_root_copy_2, document_root);
	strcat(doc_root_copy_2, dir_name+1);
	strcat(doc_root_copy_2, "index.html");
	if (file_exists(doc_root_copy_2))
		return strdup(doc_root_copy_2);

	char doc_root_copy[strlen(document_root) + 64];
	doc_root_copy[0] = '\0';
	strcat(doc_root_copy, "document directory pages");
	strcat(doc_root_copy, replace(document_root));
	strcat(doc_root_copy, ".html");

	if (!file_exists(doc_root_copy))
		scan_and_print_directory(doc_root_copy, true);
	return strdup(doc_root_copy);
}

static bool file_exists(char *file_path)
{
	return access( file_path, F_OK ) != -1;
}

static void set_keep_alive(int socket_fd)
{
	struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
}

static char * get_formatted_datetime()
{
	time_t rawtime;
	struct tm *timeinfo;

	time (&rawtime);
	timeinfo = localtime(&rawtime);
	return strdup(asctime(timeinfo));
}

static void header_info_dispose(struct header_info *header)
{
	if (!header) return;

	if (header->host) free(header->host);
	if (header->etag) free(header->etag);
	if (header->requested_objname) free(header->requested_objname);
	if (header->content_type) free(header->content_type);
	if (header->range) free(header->range);
	if (header->content_length) free(header->content_length);
	if (header->path_info) free(header->path_info);
	if (header->query_string) free(header->query_string);
}
