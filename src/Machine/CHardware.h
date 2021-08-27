/*
 * CHardware.h
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#ifndef CHARDWARE_H_
#define CHARDWARE_H_

#include <iostream>
#include <string>
#include <fstream>
//#include <usb.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/sysctl.h>

typedef struct USBPair{
	std::string vendor;
	std::string product;
}USBPair;

class CHardware {
public:
	CHardware();
	virtual ~CHardware();
	std::string getCPUName(){return m_cpuName;}
	std::string getMotherBoardName(){return m_mbName;}
	std::vector<USBPair> getUSBDevices(){return m_USBDevices;}
	bool isNotebook();

private:
	void setHWInfo();
	void setProcInfo();
	//int usbGetDescriptorString(usb_dev_handle *dev, int index, int langid, char *buf, int buflen);
	//void usbOpenDevice(void);

	std::string m_cpuName;
	std::string m_mbName;
	std::vector<USBPair> m_USBDevices;
};



#endif /* CHARDWARE_H_ */
