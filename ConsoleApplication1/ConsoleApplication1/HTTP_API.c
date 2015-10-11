#include "HTTP_API.h"
#include <stdio.h>
#include <string.h>

/*
*tx_buff = buffer in which request line will be populated
*path    = path to use in the GET request
*/
int HTTP_Build_Get_Req_Line(char *tx_buff, char *path)
{
	sprintf(tx_buff, "GET /%s HTTP/1.1 \r\n", path);
	return 17 + strlen(path);
}

/*
Must be called at least once to add "Host" header
*/
int HTTP_Add_Header(char *tx_buff, char *header_name, char *header_data, int more)
{
	if (more)
	{
		sprintf(tx_buff, "%s: %s\r\n", header_name, header_data);
		return 4 + strlen(header_name) + ((header_data) ? strlen(header_data) : 0);
	}
	else
	{
		sprintf(tx_buff, "%s: %s\r\n\r\n", header_name, header_data);
		return 6 + strlen(header_name) + ((header_data) ? strlen(header_data) : 0);
	}
}
