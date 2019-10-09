#include<string.h>
#include<stdio.h>
#include <unistd.h>

#define OTA_SCRIPT "./ota.sh"
#define OTA_LOCAL_VERSION OTA_SCRIPT" local_version"
#define OTA_VERSION OTA_SCRIPT" version"
#define OTA_DOWNLOAD OTA_SCRIPT" download"
#define OTA_UPGRADE OTA_SCRIPT" upgrade"

#define OTA_UPGRADE_FLAG "./newupgrade" 

enum STATUS {
	INIT = 1,
	NEWVERSION,
	DOWNLOADING,
	DOWNLOAD_SUCCESS,
	DOWNLOAD_FAILED,
	UPGRADING,
	UPGRADE_SUCCESS,
};

enum STATUS g_Status =  INIT; 

int (*ota_callback)(int status) = NULL;
void ota_callback_run(int status)
{
	if(ota_callback != NULL)
		ota_callback(status);
}

extern "C" int ota_init(void)
{
	if (access(OTA_UPGRADE_FLAG, R_OK) == 0) {
        ota_callback_run(UPGRADE_SUCCESS);
		remove(OTA_UPGRADE_FLAG);
        return 1;
	}
    return 0;
}

int ota_cmd(char* buf, int len, char *script)
{
	FILE*   fp;
	int ret = -1;
	if (NULL != (fp = popen(script, "r"))) {
		if(fgets(buf, len, fp) != NULL) {
			ret = 0;
        }
		fclose(fp);
	}
	return ret;
}
//
int ota_get_local_version(char *buf, int len)
{
	return ota_cmd(buf, len, OTA_LOCAL_VERSION);
}
int ota_get_new_version(char *buf, int len)
{
	return ota_cmd(buf, len, OTA_VERSION);
}
// return value 
// -1: local & new version both failed to get
//  0: only local version got
//  1:both version got
extern "C" int ota_get_version(char *local, char* newv, int len)
{
	int ret;
	
	ret = ota_get_local_version(local, len);
	if( ret < 0 )
		return -1;
	ret = ota_get_new_version(newv, len);
	if(ret < 0)
		ret = 0;
	else
		ret = 1;
	return ret;
}

int ota_download(char *buf, int len)
{
	int ret = -1;
	ota_callback_run(DOWNLOADING);
	ret = ota_cmd(buf, len, OTA_DOWNLOAD);
	if (ret >= 0) {
		if(strncmp(buf,"download complete",len) == 0) {
			ota_callback_run(DOWNLOAD_SUCCESS);
		}else {
			ota_callback_run(DOWNLOAD_FAILED);
		}
	}
	return ret;
}
extern "C" int ota_upgrade(char *buf, int len)
{
	ota_callback_run(UPGRADING);
	return ota_cmd(buf, len, OTA_UPGRADE);
}

#if 0
#define MAX_LEN 1024
int main(int argc, char* argv[])
{
	char buf[MAX_LEN]={0};
	if(strncmp(argv[1], "version", 7) == 0) {
	        char buf_local[MAX_LEN]={0};
		if(ota_get_version(buf_local, buf, MAX_LEN) < 0) {
			printf("get version failed\n");
			return -1;	
		}
	        printf("local_version=%s\n", buf_local);
	}
	if(strncmp(argv[1], "download", 8) == 0) {
		if(ota_download(buf, MAX_LEN) < 0) {
			printf("download failed\n");
			return -1;	
		}
	}
	if(strncmp(argv[1], "upgrade", 7) == 0) {
		if(ota_upgrade(buf, MAX_LEN) < 0) {
			printf("upgrade failed\n");
			return -1;	
		}
	}

	printf("result=%s\n", buf);
	return 0;
}
#endif
