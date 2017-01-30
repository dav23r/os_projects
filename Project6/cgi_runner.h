#ifndef CGI_RUNNER
#define CGI_RUNNED

#include "worker.h"
#include "config_service.h"


/* Interface to run cgi-script. 
   -> header_info should contain appropriately assigned values from
      http header. 
   -> socked_fd should contain http body content, script's response 
      will be directed there too. It should be stripped of header
      and point to very first line of body.
   -> conf is a struct from service_config containing auxilary data
      as ip address, port number etc
   Return value signifies successful envioroment setup and script call.
   It is script's responsibility to write valid things to socket.
*/

bool run_cgi_script(struct header_info *http_header, 
                    int socket_fd, struct config *conf);

#endif
