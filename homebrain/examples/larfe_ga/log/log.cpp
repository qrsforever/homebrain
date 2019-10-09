#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define EXE_SCRIPT "./log.sh"
#define BUFLEN 1024

// return value 0:success, -1:failed
extern "C" int upload_log(char *logfilename,  char* elinkid, char *logid)
{
	FILE*   fp;
	int ret = -1;
	char cmd[BUFLEN] = {0};
	char buf[BUFLEN] = {0};
	snprintf(cmd, BUFLEN, EXE_SCRIPT " %s %s %s", logfilename, elinkid, logid);
	if (NULL != (fp = popen(cmd, "r"))) {
		if(fgets(buf, BUFLEN, fp) != NULL) {
			if(strncmp(buf, "success", 7) == 0)
				ret = 0;
		}
		fclose(fp);
	}
	return ret;
}
#if 0
int main(int argc, char* argv[])
{
	char buf[BUFLEN]={0};
	if (upload_log(argv[1], argv[2], argv[3]) == 0)
		printf("upload success\n");
	else
		printf("upload failed\n");
	return 0;
}
#endif
