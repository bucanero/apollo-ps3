#include <http/https.h>
#include <ssl/ssl.h>
#include <net/net.h>
#include <sysmodule/sysmodule.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "saves.h"

#define HTTP_YES		1
#define HTTP_NO 		0
#define HTTP_SUCCESS 	1
#define HTTP_FAILED	 	0

#define HTTP_USER_AGENT "Mozilla/5.0 (PLAYSTATION 3; 1.00)"

typedef struct
{
    void* http_pool;
    void* ssl_pool;
    httpsData* caList;
    void* cert_buffer;
} t_http_pools;

static t_http_pools http_pools;
uint64_t prog_bar1_value=0;
u8 cancel = HTTP_NO;

static char getBuffer[64*1024];


int http_init(void)
{
    int ret;
	u32 cert_size=0;
	u8 module_https_loaded=HTTP_NO;
	u8 module_http_loaded=HTTP_NO;
	u8 module_net_loaded=HTTP_NO;
	u8 module_ssl_loaded=HTTP_NO;
	
	u8 https_init=HTTP_NO;
	u8 http_init=HTTP_NO;
	u8 net_init=HTTP_NO;
	u8 ssl_init=HTTP_NO;

	//init
	ret = sysModuleLoad(SYSMODULE_NET);
	if (ret < 0) {
		LOG("Error : sysModuleLoad(SYSMODULE_NET) HTTP_FAILED (%x)", ret);
//		ret=HTTP_FAILED;
//		goto end;
	} else module_net_loaded=HTTP_YES;

	ret = netInitialize();
	if (ret < 0) {
		LOG("Error : netInitialize HTTP_FAILED (%x)", ret);
//		ret=HTTP_FAILED;
//		goto end;
	} else net_init=HTTP_YES;

	ret = sysModuleLoad(SYSMODULE_HTTP);
	if (ret < 0) {
		LOG("Error : sysModuleLoad(SYSMODULE_HTTP) HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else module_http_loaded=HTTP_YES;

	http_pools.http_pool = malloc(0x10000);
	if (http_pools.http_pool == NULL) {
		LOG("Error : out of memory (http_pool)");
		ret=HTTP_FAILED;
		goto end;
	}

	ret = httpInit(http_pools.http_pool, 0x10000);
	if (ret < 0) {
		LOG("Error : httpInit HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else http_init=HTTP_YES;

	ret = sysModuleLoad(SYSMODULE_HTTPS);
	if (ret < 0) {
		LOG("Error : sysModuleLoad(SYSMODULE_HTTP) HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else module_https_loaded=HTTP_YES;

	ret = sysModuleLoad(SYSMODULE_SSL);
	if (ret < 0) {
		LOG("Error : sysModuleLoad(SYSMODULE_HTTP) HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else module_ssl_loaded=HTTP_YES;

	http_pools.ssl_pool = malloc(0x40000);
	if (http_pools.ssl_pool == NULL) {
		LOG("Error : out of memory (ssl_pool)");
		ret=HTTP_FAILED;
		goto end;
	}

	ret = sslInit(http_pools.ssl_pool, 0x40000);
	if (ret < 0) {
		LOG("Error : sslInit HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else ssl_init=HTTP_YES;

	http_pools.caList = (httpsData *)malloc(sizeof(httpsData));
	ret = sslCertificateLoader(SSL_LOAD_CERT_ALL, NULL, 0, &cert_size);
	if (ret < 0) {
		LOG("Error : sslCertificateLoader HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}

	http_pools.cert_buffer = malloc(cert_size);
	if (http_pools.cert_buffer==NULL) {
		LOG("Error : out of memory (cert_buffer)");
		ret=HTTP_FAILED;
		goto end;
	}

	ret = sslCertificateLoader(SSL_LOAD_CERT_ALL, http_pools.cert_buffer, cert_size, NULL);
	if (ret < 0) {
		LOG("Error : sslCertificateLoader HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}

	(&http_pools.caList[0])->ptr = http_pools.cert_buffer;
	(&http_pools.caList[0])->size = cert_size;

	ret = httpsInit(1, (httpsData *) http_pools.caList);
	if (ret < 0) {
		LOG("Error : httpsInit HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	} else https_init=HTTP_YES;

	return HTTP_SUCCESS;

end:
	if(http_pools.caList) free(http_pools.caList);
	if(https_init) httpsEnd();
	if(ssl_init) sslEnd();
	if(http_init) httpEnd();
//	if(net_init) netDeinitialize();
	
	if(module_http_loaded) sysModuleUnload(SYSMODULE_HTTP);
	if(module_https_loaded) sysModuleUnload(SYSMODULE_HTTPS);
	if(module_ssl_loaded) sysModuleUnload(SYSMODULE_SSL);
//	if(module_net_loaded) sysModuleUnload(SYSMODULE_NET);
	
	if(http_pools.http_pool) free(http_pools.http_pool);
	if(http_pools.ssl_pool) free(http_pools.ssl_pool);
	if(http_pools.cert_buffer) free(http_pools.cert_buffer);
	
	return ret;
}

void http_end(void)
{
	if(http_pools.caList) free(http_pools.caList);
	httpsEnd();
	sslEnd();
	httpEnd();
	netDeinitialize();
	
	sysModuleUnload(SYSMODULE_HTTP);
	sysModuleUnload(SYSMODULE_HTTPS);
	sysModuleUnload(SYSMODULE_SSL);
	sysModuleUnload(SYSMODULE_NET);

	if(http_pools.http_pool) free(http_pools.http_pool);
	if(http_pools.ssl_pool) free(http_pools.ssl_pool);
	if(http_pools.cert_buffer) free(http_pools.cert_buffer);
	
	return;
}

char* escape_filename(const char* filename)
{
	int len = strlen(filename);
    char* ret = (char *)calloc(1, len*3);

	httpUtilEscapeUri(ret, len*3, (uint8_t*) filename, len, 0);

	return ret;
}

int http_download(const char* url, const char* filename, const char* local_dst, int show_progress)
{
	int ret = 0, httpCode = 0;
	httpUri uri;
	httpClientId httpClient = 0;
	httpTransId httpTrans = 0;
	FILE* fp=NULL;
	u32 nRecv = 0;
	u32 size = 0;
	uint64_t length = 0;
	void *uri_pool = NULL;
	char* escaped_name = NULL;
	char* escaped_url = NULL;

	ret = httpCreateClient(&httpClient);
	if (ret < 0) {
		LOG("Error : httpCreateClient HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}
    httpClientSetConnTimeout(httpClient, 10 * 1000 * 1000);
    httpClientSetUserAgent(httpClient, HTTP_USER_AGENT);
	httpClientSetAutoRedirect(httpClient, 1);

	// Escape URL file name characters
	escaped_name = escape_filename(filename);
	asprintf(&escaped_url, "%s%s", url, escaped_name);
	
	LOG("Downloading (%s) -> (%s)", escaped_url, local_dst);

	//URI
	ret = httpUtilParseUri(&uri, escaped_url, NULL, 0, &size);
	if (ret < 0) {
		LOG("Error : httpUtilParseUri() HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}

	uri_pool = malloc(size);
	if (uri_pool == NULL) {
		LOG("Error : out of memory (uri_pool)");
		ret=HTTP_FAILED;
		goto end;
	}

	ret = httpUtilParseUri(&uri, escaped_url, uri_pool, size, 0);
	if (ret < 0) {
		LOG("Error : httpUtilParseUri() HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}
	//END of URI	

	//SEND REQUEST
	ret = httpCreateTransaction(&httpTrans, httpClient, HTTP_METHOD_GET, &uri);
	if (ret < 0) {
		LOG("Error : httpCreateTransaction() HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}

	ret = httpSendRequest(httpTrans, NULL, 0, NULL);
	if (ret < 0) {
		LOG("Error : httpSendRequest() HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}
	
	//GET SIZE
	httpResponseGetContentLength(httpTrans, &length);

	ret = httpResponseGetStatusCode(httpTrans, &httpCode);
	if (ret < 0) {
		LOG("Error : cellHttpResponseGetStatusCode() HTTP_FAILED (%x)", ret);
		ret=HTTP_FAILED;
		goto end;
	}

	if(httpCode != HTTP_STATUS_CODE_OK && httpCode >= 400 ) {
		LOG("Error : Status code (%d)", httpCode);
		ret=HTTP_FAILED;
		goto end;
	}
	
	//TRANSFER
	fp = fopen(local_dst, "wb");
	if(fp == NULL) {
		LOG("Error : fopen() HTTP_FAILED : %s", local_dst);
		ret=HTTP_FAILED;
		goto end;
	}
	
	prog_bar1_value=0;
	if (show_progress)
		init_progress_bar("Downloading...", filename);
	
	do {
		if(httpRecvResponse(httpTrans, (void*) getBuffer, sizeof(getBuffer)-1, &nRecv) > 0) break;
		if(nRecv == 0)	break;
		fwrite((char*) getBuffer, nRecv, 1, fp);
		if(cancel==HTTP_YES) break;

		if(length != 0) {
			prog_bar1_value += nRecv;
			if (show_progress)
				update_progress_bar(&prog_bar1_value, length, filename);
		}
	} while (nRecv != 0);
	fclose(fp);
	
	if(cancel==HTTP_YES) {
		unlink_secure((char*)local_dst);
		ret=HTTP_FAILED;
		cancel=HTTP_NO;
	}

	//END of TRANSFER
	ret=HTTP_SUCCESS;

end:
	if (show_progress) end_progress_bar();
	if(httpTrans) httpDestroyTransaction(httpTrans);
	if(httpClient) httpDestroyClient(httpClient);
	if(uri_pool) free(uri_pool);
	if(escaped_url) free(escaped_url);
	if(escaped_name) free(escaped_name);

	return ret;
}
