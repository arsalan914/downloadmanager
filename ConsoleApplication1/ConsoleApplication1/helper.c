#include "helper.h"
#include <stdio.h>

int Parse_Uri(char *uri, char *host, char *path, char *filename)
{
	int slashes = 0;
	char *ptr = uri;
	char *temp1 = 0, *temp2;

	if (strncmp(uri, "http://", 7) == 0)
	{
		ptr += 7;
	}
	else
	{
		printf("scheme not there");
	}

	temp2 = strtok(ptr, "/");
	strcpy(host, temp2);

	temp2 = strtok(NULL, "\0");
	strcpy(path, temp2);

	temp1 = path + strlen(path);

	while (*temp1 != '/')
	{
		temp1--;
	}
	temp1++;

	strncpy(filename, temp1, path + strlen(path) - temp1);

	filename[path + strlen(path) - temp1] = '\0';

}

/*
return 1 if hostname
return 0 if ip
*/
int Is_Hostname(char * uri)
{
	while (*uri != '\0' && *uri != ':')
	{
		if (*uri == '.' || isdigit((int)*uri))
		{
			;
		}
		else
		{
			return 1;
		}

		uri++;
	}

	return 0;
}

/*
return 0 indicates default port i.e 80
return 1 indicates non default port
*/
int Extract_Port(char *uri, char *port)
{
	while (*uri != '\0')
	{
		if (*uri == ':')
		{
			uri++;
			memcpy(port, uri, strlen(uri));
			port[strlen(uri)] = '\0';
			return 1;
		}

		uri++;
	}

	memcpy(port, "80", strlen("80"));
	port[strlen("80")] = '\0';
	return 0;
}

int Move_To_Content_Length_Header(char *rx_buff, char** rx_ptr, int rx)
{
	/* Move rx_ptr to start of "content-length" header. */
	while (strncmp(*rx_ptr, "Content-Length", 14) != 0)
	{
		(*rx_ptr)++;

		/* Content Length does not exist.*/
		if (*rx_ptr - rx_buff > rx)
		{
			printf("\nContent length header not found.\n");
			return 0;
		}
	}

	return 1;
}
