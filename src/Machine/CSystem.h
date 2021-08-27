/*
 * CSystem.h
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#ifndef CSYSTEM_H_
#define CSYSTEM_H_

#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/types.h>
#ifdef __linux__
#include <sys/sysinfo.h>
#include <proc/readproc.h>
#endif
#ifdef __APPLE__
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#endif
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <cstdint>

#include <streambuf>
#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <map>


//#include "Util.h"

const int NUM_CPU_STATES = 10;

enum CPUStates
{
	S_USER = 0,
	S_NICE,
	S_SYSTEM,
	S_IDLE,
	S_IOWAIT,
	S_IRQ,
	S_SOFTIRQ,
	S_STEAL,
	S_GUEST,
	S_GUEST_NICE
};

typedef struct CPUData
{
	std::string cpu;
	size_t times[NUM_CPU_STATES];
} CPUData;

typedef struct CPUStat{
	float active;
	float idle;
	std::string core;
} CPUStat;

typedef struct PROCKey{
	double mem;
	int pid;
    bool const operator==(const PROCKey &o) const{
        return pid == o.pid;
    }

    bool const operator<(const PROCKey &o) const {
        return mem < o.mem;
    }
}PROCKey;

enum class so_code
{
	windows = 1,
	android = 2,
	linux = 3,
	mac = 4,
	manual = 5,
	nobreak = 6,
	periferico = 7
};

typedef struct AdaptedServiceInfo{
	std::string program;
	std::string user;
	std::string usedMemory;
	std::string usedCPU;
	std::string fullPath;
	int PID;
	int upTime;
}AdaptedServiceInfo;

class CSystem {
public:
	CSystem();
	virtual ~CSystem();
	std::string getOSName(){return m_distOS;}
	std::string getOSVersion(){return m_distVersion;}
	std::string getOSHostName(){return m_distHostname;}

	double getRAMTotal(){return m_RAMTotal;}
	double getRAMFree(){return m_RAMFree;}
	double getRAMAvailable(){return m_RAMAvailable;}
	double getRAMCached(){return m_RAMCached;}
	double getRAMSwapTotal() {return m_RAMSwapTotal;}
	double getRAMSwapFree() {return m_RAMSwapFree;}
	double getRAMSwapCached() {return m_RAMSwapCached;}
	double getHDDSize(){return m_HDDSize;}
	double getHDDFreeSpace(){return m_HDDFree;}
	std::string getHDDFileSys(){return m_HDDFileSys;}

	double getCPUUsed(){SetCPUInfo(); return m_CPUActive;}
	int getCPUCores();

	double getCPUTempCelsius();
	double getCPUTempFahrenheit(){return getCPUTempCelsius() * 32;}
	std::vector<AdaptedServiceInfo> getProcByMemoryUsageVector(bool printRootProcs=false);
	void printProcByMemoryUsage(bool printRootProcs = false);
	int getSOCode(std::string soName);


private:
	void SetDistributionInfo();
	void SetRAMInfoInGB();
	void SetHDInfo();

	void ReadStatsCPU(std::vector<CPUData> & entries);
	size_t GetCPUIdleTime(const CPUData & e) const;
	size_t GetCPUActiveTime(const CPUData & e) const;
	void SetCPUInfo();
	void SetProcInfoByMemoryUsage();

	std::string m_distOS;
	std::string m_distVersion;
	std::string m_distHostname;
	double m_RAMTotal;
	double m_RAMFree;
	double m_RAMAvailable;
	double m_RAMCached;
	double m_RAMSwapTotal;
	double m_RAMSwapFree;
	double m_RAMSwapCached;
	double m_HDDSize;
	double m_HDDFree;
	const std::string m_HDDFileSys = "/";
	double m_CPUActive;
	std::vector<CPUStat> m_CPUStatCores;
	#ifdef __linux__
	std::map<PROCKey, proc_t> m_ProcStat;
	#endif

	#ifdef __APPLE__
	std::vector<AdaptedServiceInfo> m_ProcStat;
	#endif	
	inline static unsigned long long _previousTotalTicks = 0;
    inline static unsigned long long _previousIdleTicks = 0;

};

#endif /* CSYSTEM_H_ */
