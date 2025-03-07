#include <net/net.h>
#include <net/netctl.h>
#include <sysmodule/sysmodule.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "common.h"
#include "saves.h"

#define HTTP_SUCCESS 	1
#define HTTP_FAILED	 	0

#define HTTP_USER_AGENT "Mozilla/5.0 (PLAYSTATION 3; 1.00)"


int http_init(void)
{
    int ret;

	//init
	ret = sysModuleLoad(SYSMODULE_NET);
	if (ret < 0) {
		LOG("Error : sysModuleLoad(SYSMODULE_NET) HTTP_FAILED (%x)", ret);
		return HTTP_FAILED;
	}

	ret = sysModuleLoad(SYSMODULE_NETCTL);
	if (ret < 0) {
		LOG("Error: sysModuleLoad(SYSMODULE_NETCTL) (%X)", ret);
		return HTTP_FAILED;
	}

	ret = netInitialize();
	if (ret < 0) {
		LOG("Error : netInitialize HTTP_FAILED (%x)", ret);
		return HTTP_FAILED;
	}

	ret = netCtlInit();
	if (ret < 0) {
		LOG("Error: netCtlInit (%X)", ret);
		return HTTP_FAILED;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	return HTTP_SUCCESS;
}

void http_end(void)
{
	curl_global_cleanup();
//	netDeinitialize();
	netCtlTerm();

	sysModuleUnload(SYSMODULE_NETCTL);
	sysModuleUnload(SYSMODULE_NET);

	return;
}

/* follow the CURLOPT_XFERINFOFUNCTION callback definition */
static int update_progress(void *p, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
{
	LOG("DL: %ld / %ld", dlnow, dltotal);
	update_progress_bar(dlnow, dltotal, (const char*) p);

	return 0;
}

static int upload_progress(void *p, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
{
	LOG("UL: %ld / %ld", ulnow, ultotal);
	update_progress_bar(ulnow, ultotal, (const char*) p);

	return 0;
}

static void set_curl_opts(CURL* curl)
{
	union net_ctl_info proxy_info;

	// Set user agent string
	curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USER_AGENT);
	// not sure how to use this when enabled
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	// not sure how to use this when enabled
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	// Set SSL VERSION to TLS 1.2
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	// Set timeout for the connection to build
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
	// Follow redirects
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	// maximum number of redirects allowed
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20L);
	// Fail the request if the HTTP code returned is equal to or larger than 400
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	// request using SSL for the FTP transfer if available
	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

	// check for proxy settings
	memset(&proxy_info, 0, sizeof(proxy_info));
	netCtlGetInfo(NET_CTL_INFO_HTTP_PROXY_CONFIG, &proxy_info);

	if (proxy_info.http_proxy_config == NET_CTL_HTTP_PROXY_ON)
	{
		memset(&proxy_info, 0, sizeof(proxy_info));
		netCtlGetInfo(NET_CTL_INFO_HTTP_PROXY_SERVER, &proxy_info);
		curl_easy_setopt(curl, CURLOPT_PROXY, proxy_info.http_proxy_server);
		curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);

		memset(&proxy_info, 0, sizeof(proxy_info));
		netCtlGetInfo(NET_CTL_INFO_HTTP_PROXY_PORT, &proxy_info);
		curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy_info.http_proxy_port);
	}
}

int http_download(const char* url, const char* filename, const char* local_dst, int show_progress)
{
	char full_url[1024];
	CURL *curl;
	CURLcode res;
	FILE* fd;

	curl = curl_easy_init();
	if(!curl)
	{
		LOG("ERROR: CURL INIT");
		return HTTP_FAILED;
	}

	fd = fopen(local_dst, "wb");
	if (!fd) {
		LOG("Error: fopen(%s) failed", local_dst);
		return HTTP_FAILED;
	}

	if (!filename) filename = "";
	snprintf(full_url, sizeof(full_url), "%s%s", url, filename);
	LOG("Downloading (%s) -> (%s)", full_url, local_dst);

	set_curl_opts(curl);
	curl_easy_setopt(curl, CURLOPT_URL, full_url);
	// The function that will be used to write the data 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
	// The data file descriptor which will be written to
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);

	if (show_progress)
	{
		init_progress_bar("Downloading...", filename);
		/* pass the struct pointer into the xferinfo function */
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &update_progress);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*) filename);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	}

	// Perform the request
	res = curl_easy_perform(curl);

	if (res == CURLE_SSL_CONNECT_ERROR)
	{
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_NONE);
		res = curl_easy_perform(curl);
	}

	// close file descriptor
	fclose(fd);
	// cleanup
	curl_easy_cleanup(curl);

	if (show_progress)
		end_progress_bar();

	if(res != CURLE_OK)
	{
		LOG("curl_easy_perform() failed: %s", curl_easy_strerror(res));
		unlink_secure(local_dst);
		return HTTP_FAILED;
	}

	return HTTP_SUCCESS;
}

/* <DESC>
 * Performs an FTP upload and renames the file just after a successful
 * transfer.
 * </DESC>
 */
 
#define UPLOAD_FILE_AS  "while-uploading.txt"
#define RENAME_FILE_TO  "renamed-and-fine.txt"
  

int ftp_upload(const char* local_file, const char* url, const char* filename, int show_progress)
{
	FILE *fd;
	CURL *curl;
	CURLcode res;
	char remote_url[1024];
	unsigned long fsize;

	struct curl_slist *headerlist = NULL;
	static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
	static const char buf_2 [] = "RNTO " RENAME_FILE_TO;

//	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if(!curl)
	{
		LOG("ERROR: CURL INIT");
		return HTTP_FAILED;
	}

	/* get a FILE * of the same file */
	fd = fopen(local_file, "rb");
	if(!fd)
	{
		LOG("Couldn't open '%s'", local_file);
		return HTTP_FAILED;
	}

	/* get the file size of the local file */
	fseek(fd, 0, SEEK_END);
	fsize = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	snprintf(remote_url, sizeof(remote_url), "%s%s", url, filename);

	LOG("Local file size: %lu bytes.", fsize);
	LOG("Uploading (%s) -> (%s)", local_file, remote_url);
	
	/* build a list of commands to pass to libcurl */
	headerlist = curl_slist_append(headerlist, buf_1);
	headerlist = curl_slist_append(headerlist, buf_2);

	set_curl_opts(curl);
	/* enable uploading */
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

	/* specify target */
	curl_easy_setopt(curl, CURLOPT_URL, remote_url);

	// create missing dirs if needed
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR);

	/* please ignore the IP in the PASV response */
	curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);

	/* pass in that last of FTP commands to run after the transfer */
//	curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

	/* we want to use our own read function */
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);

	/* now specify which file to upload */
	curl_easy_setopt(curl, CURLOPT_READDATA, fd);

	/* Set the size of the file to upload (optional). */
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);

	if (show_progress)
	{
		init_progress_bar("Uploading...", filename);
		/* pass the struct pointer into the xferinfo function */
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &upload_progress);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*) filename);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	}

	/* Now run off and do what you have been told! */
	res = curl_easy_perform(curl);

	if (res == CURLE_SSL_CONNECT_ERROR)
	{
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_NONE);
		res = curl_easy_perform(curl);
	}

	/* close the local file */
	fclose(fd);

	/* clean up the FTP commands list */
	curl_slist_free_all(headerlist);

	/* always cleanup */
	curl_easy_cleanup(curl);

	if (show_progress)
		end_progress_bar();

	/* Check for errors */
	if(res != CURLE_OK)
	{
		LOG("curl_easy_perform() failed: %s", curl_easy_strerror(res));
		return HTTP_FAILED;
	}

	return HTTP_SUCCESS;
}
