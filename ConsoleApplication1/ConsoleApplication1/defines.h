//http://fs1.d-h.st/download/00123/NRi/ViperSC2_5.2.1.zip
//http://downloads.sourceforge.net/project/tortoisesvn/1.8.4/Application/TortoiseSVN-1.8.4.24972-x64-svn-1.8.5.msi
//http://cznic.dl.sourceforge.net/project/tortoisesvn/1.8.4/Application/TortoiseSVN-1.8.4.24972-x64-svn-1.8.5.msi
//http://read.pudn.com/downloads151/ebook/655187/Windriver.pdf
//http://www.centuryfurniture.com/Catalogs/catalog%20brochures/Signature2012.pdf

#define URL "http://hirez.http.internapcdn.net/hirez/InstallHiRezSmitePT19.exe"
//#define URL "http://anaa.asn.au/includes/pdfs/notice-of-motion.pdf"
//#define URL "http://a1408.g.akamai.net/5/1408/1388/2005110403/1a1a1ad948be278cff2d96046ad90768d848b41947aa1986/sample_iPod.m4v.zip"

//#define URL "http://cznic.dl.sourceforge.net/project/tortoisesvn/1.8.4/Application/TortoiseSVN-1.8.4.24972-x64-svn-1.8.5.msi"
//#define URL "http://www.biharanjuman.org/Quran/Holy_Quran-Urdu-King_Fahad_Printing_Press_Saudi.pdf"
//#define SERVER_IP "cznic.dl.sourceforge.net"
//#define SERVER_PORT "80"
//#define PATH "project/tortoisesvn/1.8.4/Application/TortoiseSVN-1.8.4.24972-x64-svn-1.8.5.msi"
#define DB_NAME "MDM.db"

#define DOWNLOAD_CHUNK_SIZE 1024 * 1024 * 4


/* Structure passes to the thread. */
typedef struct
{
	int start;
	int end;
} PARAM;

/* e.g Entry in Database

id   filename     thread_number start end remaining
0     file.mp3

*/
typedef struct
{
	int		id;
	char	filename[250];
	int		thread_number;
	int		start;
	int		end;
	int		remaining;
} db_schema;
