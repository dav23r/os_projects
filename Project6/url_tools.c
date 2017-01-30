#include "url_tools.h"
#include <ctype.h>
#include <string.h>

static char reserved_chars[] = "!#$&'()*+,/:;=?@[]";

static int escape_seq(char *str, int first);
static int strcontains(char *str, char seek);

#define escape_size 3
#define base 16
#define min(a, b) ( (a < b) ? (a) : (b) )
bool uri_to_url(char *uri, char *url, int max_size){
   
    int uri_len = strlen(uri);

    if (uri_len > max_size)
        return false;

    int i = 0;
    int j = 0;
    for (; i < uri_len; i++){
        if (i > uri_len - escape_size) {
            url[j] = uri[i];
        } else {
            char decod_ch = escape_seq(uri, i);
            if (decod_ch == 0) {
                url[j] = uri[i];
            } else {
                url[j] = decod_ch;
                i += escape_size - 1;
            }
        }
        j += 1;
    }
    url[j] = '\0';

    return true; 
}


static int escape_seq(char *str, int i){
    if (!str[i] == '%')
        return 0;
    int offset = 1;
    int res = 0;
    int power = base;
    while (offset < escape_size){
        int cur_ch = str[i + offset];
        int cur_digit;
        if (isdigit(cur_ch))
            cur_digit = cur_ch - '0';
        else if (isalpha(cur_ch))
            cur_digit = cur_ch - 'A' + 10;
        else
            return 0;
        if (cur_digit >= base)
            return 0;
        res += cur_digit * power;
        power /= base;
        offset += 1;
    }
    return strcontains(reserved_chars, res);
}

static int strcontains(char *str, char seek){
    char ch;
    int i = 0;
    while (ch = str[i++])
        if (ch == seek)
            return ch;
    return 0;
}
