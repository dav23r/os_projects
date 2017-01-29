#ifndef URI_TO_URL_H
#define URI_TO_URL_H

#include "bool.h"

/* Provides means of translating uri-encoded string to 
   url-encoded one.
   In uri-encoding characters as / ; . are considered 
   special, consequently are represented 'escaped'
   with percent sign followed by hex code of original
   asci character.

   Sample input:  %2Fbla%28%29
   Sample output: /bla()

   More here: https://en.wikipedia.org/wiki/Percent-encoding
*/
bool uri_to_url(char *uri, char *url, int max_size);

#endif
