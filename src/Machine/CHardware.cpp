/*
 * CHardware.cpp
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#include "CHardware.h"

CHardware::CHardware() {
	setHWInfo();
	//usbOpenDevice();
}

CHardware::~CHardware() {
	// TODO Auto-generated destructor stub
}


void CHardware::setHWInfo()
{
	m_cpuName = "";
	m_mbName = "";

    size_t size = 256;
    char cpuName[size];
    if (sysctlbyname("machdep.cpu.brand_string", &cpuName, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }else{
        m_cpuName = cpuName;
    }

    //Motherboard in an Apple env is linked to machine model, information you can find at HW_TARGET as below.
    int mib[2];
    size_t len;
    char *p;

    mib[0] = CTL_HW;
    mib[1] = HW_TARGET;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len]; //(char*)malloc(len);
    sysctl(mib, 2, p, &len, NULL, 0);
    m_mbName = p;
    delete p;

}



bool CHardware::isNotebook()
{
    #ifdef __linux__
	//Here we check for power suply dir.
	//If it's empty, there is no batery, then we can conclude it's not a notebook.
	const std::filesystem::path dir("/sys/class/power_supply");
	return !std::filesystem::is_empty(dir);
    #endif
    #ifdef __APPLE__
    int mib[2];
    size_t len;
    char *p;

    mib[0] = CTL_HW;
    mib[1] = HW_PRODUCT;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len]; //(char*)malloc(len);
    sysctl(mib, 2, p, &len, NULL, 0);
    bool isNote = (std::string(p).find_first_of("Book") != std::string::npos ? true:false);
    delete p;

    return isNote;
    #endif
}


