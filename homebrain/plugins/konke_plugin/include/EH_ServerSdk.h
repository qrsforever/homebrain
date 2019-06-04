/*
 * EH_ServerSdk.h
 *
 */

#ifndef EH_SERVERSDK_H_
#define EH_SERVERSDK_H_

#include <map>

#define  REQEUST_MAX_WAITTIMEOUT   5

class EH_ServerSdk {

public:
	static EH_ServerSdk* GetInstance();

	int Open();

private:
	EH_ServerSdk();
	virtual ~EH_ServerSdk();

private:
	static EH_ServerSdk* ms_instance;

	bool start;
};

#endif /* EH_SERVERSDK_H_ */
