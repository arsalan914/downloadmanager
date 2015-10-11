#include <windows.h>

int Parse_Uri(char *uri, char *host, char *path, char *filename);
int Is_Hostname(char * uri);
int Extract_Port(char *uri, char *port);
int Move_To_Content_Length_Header(char *rx_buff, char** rx_ptr, int rx);