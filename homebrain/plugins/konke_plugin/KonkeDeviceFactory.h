/*
 * KonkeDeviceFactory.h
 *
 */

#ifndef KONKE_DEVICE_FACTORY_H_
#define KONKE_DEVICE_FACTORY_H_

#include <string>
#include <vector>

class KonkeDeviceFactory {
	public:
		static KonkeDevice::Ptr getInstance(const char* type);
	private:
		KonkeDeviceFactory();
};

#endif /* KONKE_DEVICE_FACTORY_H_ */
