/*
 * EH_Client.h
 *
 */

#ifndef EH_CLIENT_H_
#define EH_CLIENT_H_

#include <sys/types.h>
#include <vector>
#include <string>
using namespace std;

#include "json/value.h"

#include "EH_FindCCUCallBackHandler.h"

class EH_FindCCU;

class EH_Client : public EH_FindCCUCallBackHandler {
public:
	enum ClientStatus {
		UNINIT,
		INITING,
		INIT_FINISH,
		FIND_CCU_IP,
		LINK_BUILDING,
		LINK_BUILD_OK,
		LINK_BUILD_FAILED,
		LINK_BROKEN,
		LOGINING,
		LOGIN_FINISH_OK,
		LOGIN_FINISH_FAILED,
		WORKING,
		STOP
	};

public:
class EH_ClientCallBackHandler {
	public:
		virtual ~EH_ClientCallBackHandler() {} ;

		virtual void onClientStatusChanged(ClientStatus currentStatus) = 0;
		virtual void onRecvMsg(string nodeId, string opcode, string arg, string status) = 0;
	};

public:
    EH_Client();
    virtual ~EH_Client();

    int init(string ccuId, string accessKey, EH_ClientCallBackHandler* cb,
    		                                     bool autoDestroyHandler);
    ClientStatus getClientStatus() const;
    int sendRequest(string nodeId, string opcode, string arg, string requester);
	void destroy();

public:
	static string getClientStatusStr(ClientStatus status);
	static void* run0(void* opt);

private:
	void* run(void* opt);

    void sendLoginReq();
    void changeClientStatus(ClientStatus newStatus);

    int sendMsgToServer(string msg);
    void processRecvMsg(string msg);
    EH::Json::Value parseRecvMsg(string msg, bool& error);

private:
	void onCCUIpChanged(string ccuNewIp);

private:
	ClientStatus currentStatus;

	string ccuId;
	string accessKey;
    string ccuServerIp;
    u_short ccuServerPort;
    EH_ClientCallBackHandler* cb;
    bool autoDestroyHandler;
    int connectToCcuFd;
    bool destroyed;

    int sendMsgErrorSum;

    EH_FindCCU* findCCU;

    string cacheInputMsg;
};

#endif /* EH_CLIENT_H_ */

