/*
 * EH_FindCCUCallBackHandler.h
 *
 */

#ifndef EH_FINDCCUCALLBACKHANDLER_H_
#define EH_FINDCCUCALLBACKHANDLER_H_

#include <string>
using namespace std;

class EH_FindCCUCallBackHandler {
public:
	virtual void onCCUIpChanged(string ccuNewIp) {};
};


#endif /* EH_FINDCCUCALLBACKHANDLER_H_ */
