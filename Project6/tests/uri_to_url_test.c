#include "../url_tools.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void){
    
    int buf_len = 100;
    char res[buf_len];

    char test1[] = "abcdefgh";
    assert(uri_to_url(test1, res, buf_len));
    assert(strcmp(test1, res) == 0);

    char test2[] = "%2Fbin%2Fls";
    assert(uri_to_url(test2, res, buf_len));
    assert(strcmp("/bin/ls", res) == 0);

    char test3[] = "%23%21...%2G";
    assert(uri_to_url(test3, res, buf_len));
    assert(strcmp("#!...%2G", res) == 0);

    char test4[] = "%5B%%2%Z%5D";
    assert(uri_to_url(test4, res, buf_len));
    assert(strcmp("[%%2%Z]", res) == 0);

    char test5[] = "/%2b%3D=";
    assert(uri_to_url(test5, res, buf_len));
    assert(strcmp("/%2b==", res) == 0);

    char test6[] = "%5A%3C";
    assert(uri_to_url(test6, res, buf_len));
    assert(strcmp(test6, res) == 0);

    assert(uri_to_url("dummy", res, 5));
    assert(!uri_to_url("dummy", res, 4));
        
    return 0;
}
