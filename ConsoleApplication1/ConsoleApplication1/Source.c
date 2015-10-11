/* 
Phase1:
Basic downloading is being done perfectly.Threads are made and file is downloaded in parts and then merged into one
file when all parts are downloaded.
*/
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//#include "defines.h" // contains the macros...gets included from db.h
#include "sqlite3.h"  
#include "HTTP_API.h" // contains HTTP related functions
#include "sock.h"    // contains the socket related functions
#include "helper.h"  // contains the helper functions
#include "db.h"      // contains the db related functions

HANDLE ghSemaphore;

/*
Used to count number of threads created which tells the number of files 
created which is needed to create the final file.
*/
int THREAD_COUNT_ = -1; 


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

/* Thread in which the chunks will be downloaded. */
DWORD WINAPI DownloadChunkThread(LPVOID ptr);

/* Total number of bytes writtent to the files by all threads is maintained in this variable.*/
int GLOBAL = 0;

char HOST[300], PATH[300], PORT[200], FILENAME[200];
sqlite3 *db = NULL;

int main()
{
	/* Socket related variables. */
	WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    int iResult;

	/* Application logic variables.*/
	int status;                              //status
	int rx;                                  // number of bytes recevived
	char rx_buff[1000];                      // buffer in which to receive data
	char *rx_ptr = rx_buff;                  // pointer to 'rx_buff'
	char tx_buff[1000];                      // buffer in which data to be trasnmit will be stored
	char *tx_ptr = tx_buff;                  // pointer to 'tx_buff'
	int tx;                                  // number of bytes to transmit
	HANDLE aThread[30];                      // thread handles
	DWORD ThreadID;                          // thread ids
	int  thread_i = 0;                       // index for the thread handle array
	int size = 0;                            // contains the size of the file to be downloaded in bytes
	PARAM p1;                                // parameter passed to the thread
	int TOTAL_SIZE = 0;                      // the total number of bytes remaining to be downloaded
    int TOTAL_RECVD = 0;                     // unused for now...intended to show download progress 
	char  url[] = URL;                       // download url
	int is_host;                             // check if hostname or ip is given in url.

	/* Database related variables. */
	char *sql;

	//test to write data in exisiting file
#if 0
	FILE *output = NULL;

	output = fopen("0.txt", "rb+");

	if (output!= NULL && fseek(output, 1, SEEK_SET) == 0)
	{

		{
			printf("success %d", fwrite(":", 1, 1, output));
		}
	}
	return 0;
	//test
#endif


	open_db(DB_NAME, &db);

	/* Create SQL statement */
	sql = "CREATE TABLE mdm_info( \
		id             INTEGER PRIMARY KEY AUTOINCREMENT, \
		filename       char(300)           NOT NULL,      \
		thread         INTEGER             NOT NULL,      \
		start          INTEGER             NOT NULL,      \
		end            INTEGER             NOT NULL,      \
		remain         INTEGER             NOT NULL       \
		); ";

	create_table(sql, db);

	read_all_table(db);

//	return 0;

#if 0
	// Create a semaphore.
	ghSemaphore = CreateSemaphore(
		NULL,           // default security attributes
		1,              // initial count
		1,              // maximum count
		NULL);          // unnamed semaphore

	if (ghSemaphore == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		return 1;
	}
#endif

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    
	if (iResult != 0)
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

	/* Parse the uri to extract host,path and filename.*/
	Parse_Uri(url, HOST, PATH, FILENAME);

	/* Check if hostname or ip is given.*/
	is_host = Is_Hostname(HOST);

	/* Extract the port. */
	Extract_Port(HOST, PORT);

	/* TODO :Checks need to be added to handle case where hostname is IP.*/
	if (SOCK_CONNECT(HOST, PORT, &ConnectSocket) != 0)
	{
		printf("Faied to connect to server");
		return 0;
	}
		
    if (ConnectSocket == INVALID_SOCKET)
	{
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

	/* Populate the request line.*/
	tx_ptr += HTTP_Build_Get_Req_Line(tx_ptr, PATH);

	/* Add the 'Host' header.*/
	tx_ptr += HTTP_Add_Header(tx_ptr, "Host", HOST, 1);

	/* Add the 'Range' header.*/
	tx_ptr += HTTP_Add_Header(tx_ptr, "Range", 0, 0);

	/* Print the data that is to be transmitted.*/
	printf("\r\n%s\r\n", tx_buff);

	/* Send the data. */
	tx = send(ConnectSocket, tx_buff, strlen(tx_buff), 0);

	/* Receive the data.*/
	rx = recv(ConnectSocket, rx_buff, 1000, 0);

	/* Print the received data. */
	printf("\r\n rx data= %s\r\n", rx_buff);

	/* Move to the content length part. */
	Move_To_Content_Length_Header(rx_buff, &rx_ptr, rx);

	/* Move 'rx_ptr' ahead of 'content-length'. */
	while (*rx_ptr++ != ' ');

	/* Store the total size of the file.*/
	TOTAL_SIZE = size = atoi(rx_ptr);

	/* Print the size of file.*/
	printf("\r\n size = %d bytes, \r\n size = %f KB , \r\n size = %f MB\r\n", 
			size,
			(float)size / 1024,
			(float)size/(1024*1024));

	/* Clost the socket.*/
	closesocket(ConnectSocket);

	/* Set this to -1 because 'Range' header takes range inclusive of end values e.g 0 - 2 means 3 bytes.*/
	p1.end = -1;

	/* Loop until 'TOTAL_SIZE' is poistive.*/
	while (TOTAL_SIZE  > 0)
	{
		/* Set start value of the 'Range' header. */
		p1.start = p1.end + 1;

		/* Set end value of the 'Range' header. */
		p1.end += DOWNLOAD_CHUNK_SIZE;

		/* TODO:cHECK  moved in thread probably.Check in the table for the entry of the same filename and thread number and load end,start and remaining.*/

		/* If end value is greater than the actual size of the file .*/
		if (p1.end > size)
		{
			/* Set end value according to the bytes remaining.*/
			p1.end -= DOWNLOAD_CHUNK_SIZE;
			p1.end += TOTAL_SIZE;

			/* Update 'TOTAL_SIZE' variable.*/
			TOTAL_SIZE -= (p1.end - p1.start + 1);
				
			/* Indicate the size of the final chunk.*/
			printf("\nfinal chunk =%d\n", p1.end + 1 - p1.start);
		}

		else
		{
			/* Update 'TOTAL_SIZE' variable.*/
			TOTAL_SIZE -= DOWNLOAD_CHUNK_SIZE;
		}
//		while (WaitForSingleObject(ghSemaphore, 0L) != WAIT_OBJECT_0)
//		{
//			Sleep(1);
//		}
//		open_db(DB_NAME, &db);
//		insert_in_table(db, FILENAME, thread_i, p1.start, p1.end, p1.end - p1.start + 1);
//		close_db(db);
//		ReleaseSemaphore(ghSemaphore, 1, NULL);

		THREAD_COUNT_++;

		/* Create a thread. */
		aThread[thread_i] = CreateThread(
							NULL,       // default security attributes
							0,          // default stack size
							(LPTHREAD_START_ROUTINE)DownloadChunkThread,
							(void *)&p1,       // thread function arguments
							0,          // default creation flags
							&ThreadID); // receive thread identifier
		/* Increment the 'thread_i' variable.*/
		thread_i++;

		/* Sleep just to wait for the thread to store the argument locally. Might not be required.*/
		Sleep(100);
	}

	/* Print total number of threads created.*/
	printf("\nThread counter = %d\n", thread_i);

//	Sleep(5000);

	/* Wait for all threads to terminate. */
	WaitForMultipleObjects(thread_i, aThread, TRUE, INFINITE);

	/* Close thread handles */
	for (int i = 0; i < thread_i; i++)
		CloseHandle(aThread[i]);

	close_db(db);


//	while (WaitForSingleObject(ghSemaphore, 0L) != WAIT_OBJECT_0)
//	{
//	}
//	open_db(DB_NAME, &db);
//	read_all_table(db);
//	close_db(db);
//	ReleaseSemaphore(ghSemaphore, 1, NULL);

	/* Print the 'GLOBAL' variable.*/
	printf("\r\n %d \r\n", GLOBAL);

	/* Copy all the files created by the threads into one file.*/
	FILE *finaloutput,*temp;
	finaloutput = fopen(FILENAME, "wb+");
	char filename[100];
	char file_buffer[1024] = { 0 };
	char *bin_data = file_buffer;
	int filesize = 0,tmp;

	/* Loop as many times as many threads were created.*/
	for (int i = 0; i <= THREAD_COUNT_; i++)
	{
		/* Populate the filename.*/
		sprintf(filename, "%d",i);
		strcat(filename, ".txt");

		/* Open the file. */
		temp = fopen(filename, "rb");

		/* clear the buffer. */
		memset(bin_data, 0, 1024);

		/* Calculate the total size of the file. */
		fseek(temp, 0, SEEK_END);
		filesize = ftell(temp);
		rewind(temp);

		/* Calculate the number of chunks. */
		tmp = filesize / 1024;

		/* Print the break down details. */
//		printf("\n1024 bytes chunk =%d\n", tmp);
//		printf("\nremaining chunk =%d\n", filesize % 1024);

		/* copy the 1024 sized chunks. */
		while (tmp != 0)
		{
			fread(bin_data, 1, 1024, temp);

			fwrite(bin_data, 1, 1024, finaloutput);

			memset(bin_data, 0, 1024);

			tmp--;
		}

		/* copy the remaining data. */
		if (filesize % 1024 != 0)
		{
			fread(bin_data, 1, filesize % 1024, temp);

			fwrite(bin_data, 1, filesize % 1024, finaloutput);
		}

		/* Close the file.*/
		fclose(temp);

		/* Delete the file.*/
		remove(filename);

	}

	/* Close the file.*/
	fclose(finaloutput);

    WSACleanup();

    return 0;
}

/* Thread function which will be used to download the data.
   It will be passed start and end value. More info might be passed to it in future e.g url.*/
DWORD WINAPI DownloadChunkThread(LPVOID arg)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	int iResult;

	int rx; // number of bytes recevived
	char rx_buff[1000]; // buffer in which data to receive
	char *rx_ptr = rx_buff;
	char tx_buff[1000];
	char *tx_ptr = tx_buff;
	char header_value[200];
	char *header_value_ptr = header_value;
	int tx; // number of bytes to transmit
	PARAM p1 = *((PARAM *)arg);
	char start_s[50], end_s[50];
	int size = p1.end + 1 - p1.start;//e.g start=0 end=10 then size will be 11 bytes
	int thread_i = THREAD_COUNT_;
	int counter = 0;

	char filename[500];
	char dbname[100];

	db_schema data;

	/* Database related variables. */
	// not needed as we do not want separate database for each thread
#if 0
	sqlite3 *db = NULL;
	char *sql;
#endif

	int thread_id = THREAD_COUNT_;

	FILE *output;

	sprintf(filename, "%d", THREAD_COUNT_);
	sprintf(dbname, "%d", THREAD_COUNT_);
	strcat(dbname, ".db");
	strcat(filename, ".txt");

// not needed as we do not want separate database for each thread
#if 0
	open_db(dbname, &db);

	sqlite3_exec(db, "PRAGMA synchronous=OFF;", NULL, 0, 0);
	sqlite3_exec(db, "PRAGMA journal_mode = OFF;", NULL, 0, 0);
	sqlite3_exec(db, "PRAGMA cache_size = 10000;", NULL, 0, 0);
	sqlite3_exec(db, "PRAGMA page_size = 4096;", NULL, 0, 0);
	sqlite3_exec(db, "PRAGMA temp_store = memory;", NULL, 0, 0);

	/* Create SQL statement */
	sql = "CREATE TABLE mdm_info( \
		id             INTEGER PRIMARY KEY AUTOINCREMENT, \
		filename       char(300)           NOT NULL,      \
		thread         INTEGER             NOT NULL,      \
		start          INTEGER             NOT NULL,      \
		end            INTEGER             NOT NULL,      \
		remain         INTEGER             NOT NULL       \
		); ";

	create_table(sql, db);
#endif

	strcpy(data.filename, FILENAME);
	data.thread_number = thread_i;

	//TODO: Add logic here which will read the table to handle the case where this download is being resumed.

	get_thread_details(db, &data);
	
	/* if no entry is found then data variable memory would have been set to -1 so checking any member for -1 will tell if no entry was found. */
	if (data.start < 0)
	{
		insert_in_table(db, FILENAME, thread_i, p1.start, p1.end, p1.end - p1.start + 1);

		output = fopen(filename, "wb+");
	}
	/* if entry was found. */
	else
	{
		output = fopen(filename, "rb+");

		/* if for some reason file has been deleted.*/
		if (output == NULL)
		{
			output = fopen(filename, "wb+");
		}
		else
		{
			/* move file pointer.*/
			if (fseek(output, (data.end - data.start + 1) - (data.remaining), SEEK_SET) != 0)
			{
				printf("\r\nUnable to seek");
				exit(1);
			}

			/* Update the p1.start. This will mean that the 'Range' header gets a different starting value i.e it will not start downloading from start.*/
			p1.start = (data.end-data.start+1)//size of chunk to be downloaded by this thread 
				       - (data.remaining) 
					   +  data.start; 

			size = p1.end + 1 - p1.start;
		}
	}

//	if (size > 0)
	{

		//	GLOBAL += size;
		//	printf("\nsize =%d\nglobal=%d\n", size, GLOBAL);
		//	Sleep(10);

		//    while(size  > 0)
		{
			// Initialize Winsock
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0) {
				printf("WSAStartup failed with error: %d\n", iResult);
				return 1;
			}

			if (SOCK_CONNECT(HOST, PORT, &ConnectSocket) != 0)
			{
				printf("Faied to connect to server");
				return 0;
			}

			if (ConnectSocket == INVALID_SOCKET) {
				printf("Unable to connect to server!\n");
				WSACleanup();
				return 1;
			}

			/* Set 'rx_ptr' pointer to the start of buffer in which data will be recvd.*/
			rx_ptr = rx_buff;

			/* Convert start and end values into string. */
			sprintf(start_s, "%d", p1.start);
			sprintf(end_s, "%d", p1.end);

			/* populate range header's value part.*/
			sprintf(header_value_ptr, "bytes=%s-%s", start_s, end_s);

			//        printf("\r\nstart =%s\r\n",start_s);
			//        printf("\r\nend =%s\r\n",end_s);

			/* Set 'tx_ptr' pointer to the start of buffer in which data sent will be stored.*/
			tx_ptr = tx_buff;

			tx_ptr += HTTP_Build_Get_Req_Line(tx_ptr, PATH);

			tx_ptr += HTTP_Add_Header(tx_ptr, "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36", 1);
			tx_ptr += HTTP_Add_Header(tx_ptr, "Host", HOST, 1);
			tx_ptr += HTTP_Add_Header(tx_ptr, "Accept", "*/*", 1);
			tx_ptr += HTTP_Add_Header(tx_ptr, "Range", header_value, 0);

			printf("\r\n%s\r\n", tx_buff);

			tx = send(ConnectSocket, tx_buff, strlen(tx_buff), 0);

			rx = recv(ConnectSocket, rx_buff, 1000, 0);

			//        printf("\r\n rx data= %s\r\n", rx_buff);

			if (rx <= 0)
			{
				printf("\nERROR:No data received from server\n");
				return 0;
			}

			/* Move to the content body. */
			while (memcmp(rx_ptr, "\r\n\r\n", 4) != 0)
			{
				rx_ptr++;
			}

			/* Move ahead of CRLF CRLF. */
			if (memcmp(rx_ptr, "\r\n\r\n", 4) == 0)
			{
				rx_ptr += 4;
			}

			else
			{
				printf("error\n");
				Sleep(7000);
				return 0;
			}

			/* Loop until all data is recvd. */
			while (size > 0)
			{
				//            printf("\r\ntotal = %d\r\nfile data size = %d\r\n",rx, (int)(rx - (rx_ptr - rx_buff)));

				/* Copy actual data i.e http body part only excluding the http headers. */
				if ((int)(rx - (rx_ptr - rx_buff)) > 0)
				{
					/* write only the file data i.e (total received minus the headers and CRLF and CRLF)*/
					GLOBAL += fwrite(rx_ptr, 1, (int)(rx - (rx_ptr - rx_buff)), output);
				}

				//      printf("\r\n file data= %s\r\n", rx_ptr);

				/* Subtract the number of bytes of file data received i.e (total received minus headers and CRLF and CRLF). */
				size -= (int)(rx - (rx_ptr - rx_buff));

				//			while (WaitForSingleObject(ghSemaphore, 0L) != WAIT_OBJECT_0)
				//			{
				//				Sleep(1);
				//			}
				//			open_db(DB_NAME, &db);

				counter++;
				if (counter % 400 == 0)
					update_entry(db, FILENAME, thread_id, size);


				//			close_db(db);
				//			ReleaseSemaphore(ghSemaphore, 1, NULL);

				//            TOTAL_RECVD += (int)(rx - (rx_ptr - rx_buff));
				//            printf("\nRemainging=%d\n",size );

				//            printf("\nPercentage %f\n", (float)TOTAL_RECVD/(float)TOTAL_SIZE*100);

				/* Recv data only if there is some otherwise this will block forever. */
				if (size > 0)
				{
					memset(rx_buff, 0, sizeof(rx_buff));

					rx = recv(ConnectSocket, rx_buff, 200, 0);

					//				printf("\r\n%s\r\n",rx_buff);

				}
				else
				{
					break;
				}

				//            printf("\nrx=%d\n",rx);
				rx_ptr = rx_buff;
			}

			closesocket(ConnectSocket);
		}
	}

	update_entry(db, FILENAME, thread_id, size);

	printf("\n%d\n", counter);

	fclose(output);

	printf("\nTHread Ended\n");
}
