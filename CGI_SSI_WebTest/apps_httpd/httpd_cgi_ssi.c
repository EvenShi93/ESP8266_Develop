#include "httpd.h"
#include "lwip/tcp.h"

const char *Print_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

const char* TAGS[] = {"echo", "here"};
static char* display_1 = " echo: hi, world! - 0<br />";
static char* display_2 = " here: hi, world! - 0<br />";

static uint8_t req_cnt_1 = 0;
static uint8_t req_cnt_2 = 0;

const tCGI PRINT_CGI = {"/kyChu/print.cgi", Print_Handler};
tCGI CGI_TAB[1];

u16_t StringHandler(int iIndex, char *pcInsert, int iInsertLen)
{
	uint8_t i = 0;
//	printf("handler process.\n");
	if(iIndex == 0) {
		for(i = 0; i < 28; i ++)
			pcInsert[i] = display_1[i];

		pcInsert[20] += req_cnt_1;
		req_cnt_1 ++;
//		printf("insert %s.\n", pcInsert);
		return 28;
	} else if(iIndex == 1) {
		for(i = 0; i < 28; i ++)
			pcInsert[i] = display_2[i];

		pcInsert[20] += req_cnt_2;
		req_cnt_2 ++;
//		printf("insert %s.\n", pcInsert);
		return 28;
	}
	return 0;
}

const char *Print_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	int i = 0;
	if(iIndex == 0) {
		printf("CGI > got param num: %d.\n", iNumParams);
		for(i = 0; i < iNumParams; i ++) {
			printf("param is: %s, Vaule is %s.\n", pcParam[i], pcValue[i]);
		}
	}
	return "/index.shtml";
}

static uint8_t UpgradeCmd = 0;

static char req_uri[20];
#define FILE_NAME_MAX_LENGTH 40
static char filename[FILE_NAME_MAX_LENGTH + 1];
static const char* default_uri = "/index.shtml";
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
//	printf("a POST request has been received.\n");
//	printf("content_len: %d.\n", content_len);
//	printf("uri is %s, len = %d.\n", uri, strlen(uri));
	if(!strcmp(uri, "/upgrade/wifi.cgi")) {
		UpgradeCmd = 1;//upgrade start flag.
	}
	memcpy(req_uri, uri, strlen(uri));//save the origin request uri.
	req_uri[strlen(uri)] = 0;
	return ERR_OK;
}

char cache[20];
err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
//	printf("each pbuf of data that has been received for a POST.\n");
//	printf("tot_len = %d, len = %d.\n", p->tot_len, p->len);
//	((char *)p->payload)[p->len] = 0;
//	printf("payload is %s.\n", p->payload);
	if(!strcmp(req_uri, "/kyChu/print.cgi")) {
		int i = 0, j = 0;
		while(i < p->len && ((char *)p->payload)[i] != '=') i ++;
		do {
			i ++;
			cache[j ++] = ((char *)p->payload)[i];
		} while(i < p->len && ((char *)p->payload)[i] != '&');
		cache[j - 1] = 0;
		printf("%s\n", cache);
		memcpy(req_uri, default_uri, strlen(default_uri));
		req_uri[strlen(default_uri)] = 0;
	} else if((!strcmp(req_uri, "/upgrade/wifi.cgi")) || (!strcmp(req_uri, "/upgrade/fc.cgi"))) {
		uint32_t FileNameOffset = 0;
		uint32_t i = 0;
		while(++ i < p->len) {
			if(((char *)p->payload)[i] == 'f') {
				if(strncmp(&((char *)p->payload)[i], "filename=", 9) == 0) {
					FileNameOffset = i + 10;
					break;
				}
			}
		}
		if(FileNameOffset) {
			i = 0;
			while(((char *)p->payload)[FileNameOffset] != '\"' && i < FILE_NAME_MAX_LENGTH) {
				filename[i ++] = ((char *)p->payload)[FileNameOffset];
				FileNameOffset ++;
			}
			filename[i] = 0;
			printf("file name is %s.\n", filename);
		}
		memcpy(req_uri, default_uri, strlen(default_uri));//jump to default page(index page).
		req_uri[strlen(default_uri)] = 0;
	}
	if(UpgradeCmd == 1) {
		/* !!!need another process!!! */
//		printf("!program wifi %s", p->payload);//programming...
	}
	return ERR_OK;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
	uint32_t i = 0;
//	printf("all data is received or the connection is closed.\n");
	if(UpgradeCmd) UpgradeCmd = 0;
	memcpy(response_uri, req_uri, strlen(req_uri));/* jump to the specific page. */
	response_uri[strlen(req_uri)] = 0;
//	printf("rsp uri: %s.\n", response_uri);
}

void Httpd_cgi_ssi_init(void)
{
	http_set_ssi_handler(StringHandler, (char const **)TAGS, 2);
	CGI_TAB[0] = PRINT_CGI;
	http_set_cgi_handlers(CGI_TAB, 1);
}
