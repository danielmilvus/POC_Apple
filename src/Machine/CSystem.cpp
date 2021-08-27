/*
 * CSystem.cpp
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#include "CSystem.h"

CSystem::CSystem() {
	SetDistributionInfo();
	SetRAMInfoInGB();
	SetHDInfo();
	SetCPUInfo();
	SetProcInfoByMemoryUsage();
}

CSystem::~CSystem() {
	// TODO Auto-generated destructor stub
}

void CSystem::SetDistributionInfo()
{
	m_distOS = m_distVersion = m_distHostname = "";
	#ifdef __linux__
	struct utsname name;
	if(uname(&name))
	{
		std::cerr << "Could not get distribution information!" << std::endl;
		return;
	}
	m_distOS = name.sysname;
	m_distVersion = name.version;
	m_distHostname = name.nodename;
	#endif

	#ifdef __APPLE__
    size_t size = 256;

    char hostName[size];
    if (sysctlbyname("kern.hostname", &hostName, &size, NULL, 0) < 0)
    {
        perror("sysctl kern.hostname:");
    }else{
        m_distHostname = hostName;
    }	

    char oSys[size];
    if (sysctlbyname("kern.ostype", &oSys, &size, NULL, 0) < 0)
    {
        perror("sysctl kern.ostype:");
    }else{
        m_distOS = oSys;
    }

    char oSysVer[size];
    if (sysctlbyname("kern.osversion", &oSysVer, &size, NULL, 0) < 0)
    {
        perror("sysctl kern.osversion:");
    }else{
        m_distVersion = oSysVer;
    }	
	
	#endif
}

void CSystem::SetRAMInfoInGB()
{
	#ifdef __linux__
	std::ifstream fileStat("/proc/meminfo");

	std::string line;

	const std::string STR_RAM_TOTAL("MemTotal");
	const std::string STR_RAM_FREE("MemFree");
	const std::string STR_RAM_AVAILABLE("MemAvailable");
	const std::string STR_RAM_CASHED("Cached");
	const std::string STR_SWAP_TOTAL("SwapTotal");
	const std::string STR_SWAP_FREE("SwapFree");
	const std::string STR_SWAP_CASHED("SwapCached");
	//const std::size_t LEN_STR_CPU = STR_CPU.size();
	//const std::string STR_TOT("tot");

	while(std::getline(fileStat, line))
	{
		//Check if the string is found at the beginning of the line, that's what I want.

		if(line.find(STR_RAM_TOTAL) == 0)
		{
			m_RAMTotal = (double)Util::GetFirstNumberInLine(line) * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_RAM_FREE) == 0)
		{
			m_RAMFree = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_RAM_AVAILABLE) == 0)
		{
			m_RAMAvailable = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_RAM_CASHED) == 0)
		{
			m_RAMCached = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_SWAP_TOTAL) == 0)
		{
			m_RAMSwapTotal = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_SWAP_FREE) == 0)
		{
			m_RAMSwapFree = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
		if(line.find(STR_SWAP_CASHED) == 0)
		{
			m_RAMSwapCached = (double)Util::GetFirstNumberInLine(line)  * MemoryMultiplier * MemoryMultiplier;
		}
	}
	#endif

	#ifdef __APPLE__

	#endif	
}

void CSystem::SetHDInfo()
{
	#ifdef __linux__
	struct statvfs fsinfo;
	statvfs(m_HDDFileSys.c_str(), &fsinfo);
	m_HDDSize = (double)(fsinfo.f_frsize * fsinfo.f_blocks) * ByteInGBMultiplier;
	m_HDDFree = (double)(fsinfo.f_bsize * fsinfo.f_bfree) * ByteInGBMultiplier;
	#endif

	#ifdef __APPLE__
	struct statfs statf;
	statfs(".", &statf);
	m_HDDSize = statf.f_bsize * statf.f_blocks;
	m_HDDFree = statf.f_bsize * statf.f_bavail;
	#endif
}

#ifdef __linux__
void CSystem::ReadStatsCPU(std::vector<CPUData> & entries)
{
	//unsigned num_cpu = std::thread::hardware_concurrency();
	std::ifstream fileStat("/proc/stat");

	std::string line;

	const std::string STR_CPU("cpu");
	const std::size_t LEN_STR_CPU = STR_CPU.size();
	const std::string STR_TOT("tot");

	while(std::getline(fileStat, line))
	{
		// cpu stats line found
		if(!line.compare(0, LEN_STR_CPU, STR_CPU))
		{
			std::istringstream ss(line);

			// store entry
			entries.emplace_back(CPUData());
			CPUData & entry = entries.back();

			// read cpu label
			ss >> entry.cpu;

			// remove "cpu" from the label when it's a processor number
			if(entry.cpu.size() > LEN_STR_CPU)
				entry.cpu.erase(0, LEN_STR_CPU);
			// replace "cpu" with "tot" when it's total values
			else
				entry.cpu = STR_TOT;

			// read times
			for(int i = 0; i < NUM_CPU_STATES; ++i)
				ss >> entry.times[i];
		}
	}
}

size_t CSystem::GetCPUIdleTime(const CPUData & e) const
{
	return	e.times[S_IDLE] +
			e.times[S_IOWAIT];
}

size_t CSystem::GetCPUActiveTime(const CPUData & e) const
{
	return	e.times[S_USER] +
			e.times[S_NICE] +
			e.times[S_SYSTEM] +
			e.times[S_IRQ] +
			e.times[S_SOFTIRQ] +
			e.times[S_STEAL] +
			e.times[S_GUEST] +
			e.times[S_GUEST_NICE];
}
#endif

void CSystem::SetCPUInfo()
{
	#ifdef __linux__
	std::vector<CPUData> entries1;
	std::vector<CPUData> entries2;

	// snapshot 1
	ReadStatsCPU(entries1);
	// 100ms pause
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// snapshot 2
	ReadStatsCPU(entries2);
	// 100ms pause
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	const size_t NUM_ENTRIES = entries1.size();

	m_CPUStatCores.clear();

	for(size_t i = 0; i < NUM_ENTRIES; ++i)
	{
		const CPUData & e1 = entries1[i];
		const CPUData & e2 = entries2[i];

		const float ACTIVE_TIME	= static_cast<float>(GetCPUActiveTime(e2) - GetCPUActiveTime(e1));
		const float IDLE_TIME	= static_cast<float>(GetCPUIdleTime(e2) - GetCPUIdleTime(e1));
		//const float TOTAL_TIME	= ACTIVE_TIME + IDLE_TIME;
		if(e1.cpu == "tot")
		{
			m_CPUActive = ACTIVE_TIME;
		}else
		{
			CPUStat cpuStat;
			cpuStat.active = ACTIVE_TIME;
			cpuStat.idle = IDLE_TIME;
			cpuStat.core = e1.cpu;
			m_CPUStatCores.push_back(cpuStat);
		}
	}
	#endif

	#ifdef __APPLE__
	host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS)
    {
        unsigned long long totalTicks = 0;
        for(int i=0; i<CPU_STATE_MAX; i++)
		{
            totalTicks += cpuinfo.cpu_ticks[i];
		}
		unsigned long long idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
		unsigned long long totalTicksSinceLastTime = totalTicks-_previousTotalTicks;
		unsigned long long idleTicksSinceLastTime  = idleTicks-_previousIdleTicks;
		m_CPUActive = 1.0f-((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime)/totalTicksSinceLastTime : 0);
		_previousTotalTicks = totalTicks;
		_previousIdleTicks  = idleTicks;
    }
    else
       m_CPUActive =  -1.0f;

	#endif
}

int CSystem::getCPUCores()
{
	#ifdef __linux__
	return m_CPUStatCores.size();
	#endif

	#ifdef __APPLE__
    int mib[2];
    int cpus;
    size_t length;

    // Get the Physical memory size
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    length = sizeof(int);
    sysctl(mib, 2, &cpus, &length, NULL, 0);  

    return cpus;  
	#endif

}

//Not sorted by memory in APPLE
void CSystem::SetProcInfoByMemoryUsage()
{
	#ifdef __linux__
	PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR);

	proc_t proc_info;
	memset(&proc_info, 0, sizeof(proc_info));

	while (readproc(proc, &proc_info) != NULL) {
		PROCKey pk;
		pk.mem = proc_info.resident;
		pk.pid = proc_info.tid;
		m_ProcStat.insert(std::make_pair(pk, proc_info));
	}

	closeproc(proc);
	#endif

	#ifdef __APPLE__
    int numberOfProcesses = proc_listpids(PROC_PGRP_ONLY, KERN_PROC, NULL, 0);
    pid_t pids[numberOfProcesses];
    bzero(pids, sizeof(pids));
    proc_listpids(PROC_ALL_PIDS, KERN_PROC, pids, sizeof(pids));
	std::vector<std::string> vName;
    for (int i = 0;   i < numberOfProcesses;  ++i) 
	{
        if (pids[i] == 0) { continue;  }
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));

        if (strlen(pathBuffer) > 0) {
			std::string procName(pathBuffer);
			procName = procName.substr(procName.find_last_of("/") + 1, procName.npos);			
			std::vector<std::string>::iterator it = std::find(vName.begin(), vName.end(), procName);
			if(it == vName.end())
			{
				vName.push_back(procName);
				AdaptedServiceInfo adpInf;
				adpInf.PID = pids[i];
				adpInf.fullPath = pathBuffer;
				//remove full path
				adpInf.program = procName;				
				m_ProcStat.push_back(adpInf);
			}
            //std::cout << i << " - Pid: " << pids[i] << " :" << pathBuffer << std::endl;
            //printf("path : %s\n", pathBuffer);
        }
    }  
    //std::cout << "\n*\n#\n*# EXE Qtd Proc: " << numberOfProcesses << std::endl; 
	
	#endif			
}

//Not sorted by memory in APPLE
std::vector<AdaptedServiceInfo> CSystem::getProcByMemoryUsageVector(bool printRootProcs)
{
	#ifdef __linux__
	std::vector<AdaptedServiceInfo> procs;
	for(std::map<PROCKey, proc_t>::const_reverse_iterator it = m_ProcStat.rbegin(); it != m_ProcStat.rend(); ++it)
	{
		if (std::string(it->second.euser) == "root" && !printRootProcs)
			continue;
		
		AdaptedServiceInfo srv;
		srv.program = it->second.cmd;
		srv.user = it->second.euser;
		srv.usedMemory = std::to_string(it->second.resident  * MemoryMultiplier);
		srv.usedCPU = std::to_string(it->second.pcpu);
		srv.PID = it->second.tid;
		srv.upTime = (int)it->second.utime;
		
		procs.push_back(srv);
	}
	return procs;
	#endif

	#ifdef __APPLE__
	return m_ProcStat;
	#endif
	
}

//Not sorted by memory in APPLE
void CSystem::printProcByMemoryUsage(bool printRootProcs)
{
	#ifdef __linux__
 	//std::cout << "Program\tPID\tPPID\tMEM\tutime\tstime" << "\tUser" << std::endl;
 	std::cout << std::setw(35) << std::fixed <<  "\nProgram"
 			  << std::setw(8) << std::fixed  << "PID"
			  << std::setw(8) << std::fixed << "PPID"
			  << std::setw(12) << std::fixed << "Mem MB"
			  << std::setw(8) << std::fixed << "% CPU"
			  << std::setw(8) << std::fixed << "utime"
			  << std::setw(8) << std::fixed<< "stime"
			  << std::setw(15) << std::fixed<< "User" << std::endl;

	for(std::map<PROCKey, proc_t>::const_reverse_iterator it = m_ProcStat.rbegin(); it != m_ProcStat.rend(); ++it)
	{
		if (std::string(it->second.euser) == "root" && !printRootProcs)
			continue;
		std::cout << std::setw(35) << std::fixed << it->second.cmd
				<< std::setw(8) << std::fixed << it->second.tid
				<< std::setw(8) << std::fixed << it->second.ppid
				<< std::setw(12) << std::fixed << std::setprecision(2) <<  (double)(it->second.resident  * MemoryMultiplier) //  << " MB"
				<< std::setw(8) << std::fixed << it->second.pcpu// << "%"
				<< std::setw(8) << std::fixed << it->second.utime
				<< std::setw(8) << std::fixed << it->second.stime
				<< std::setw(15) << std::fixed << it->second.euser
				<< std::endl;
	}
	#endif

	#ifdef __APPLE__
	 	//std::cout << "Program\tPID\tPPID\tMEM\tutime\tstime" << "\tUser" << std::endl;
 	std::cout << std::setw(6) << std::fixed  << "PID" << std::fixed <<  "\nProgram" << std::endl;

	for(std::vector<AdaptedServiceInfo>::const_reverse_iterator it = m_ProcStat.rbegin(); it != m_ProcStat.rend(); ++it)
	{
		std::cout << std::setw(6) << std::fixed << it->PID << std::fixed << it->program << std::endl;
	}
	#endif
}


double CSystem::getCPUTempCelsius()
{
	#ifdef __linux__
	std::string baseDir("/sys/class/thermal/thermal_zone");
	for(int i = 0; i<3; ++i)
	{
		std::string strDir(baseDir + std::to_string(i));
		//syslog(LOG_NOTICE, "Thermal check on dir:");
		//syslog(LOG_NOTICE, strDir.c_str(), LOG_DAEMON);
		if(Util::isDir(strDir))
		{
			std::string fType(strDir + "/type");
			std::ifstream sType(fType);
			if(sType.fail())
			{
				syslog(LOG_ERR, "Fail to check thermal type, thermal information will not be loaded.");
				continue;
			}
			fType.clear();
			sType >> fType;
			if(fType == "x86_pkg_temp")
			{
				std::ifstream sTemp(strDir + "/temp");
				if(sTemp.fail())
				{
					syslog(LOG_ERR, "Could not load thermal information from 'temp' file on:");
					syslog(LOG_ERR, strDir.c_str(), LOG_DAEMON);
					return 0;
				}
				std::string fTemp;
				sTemp >> fTemp;
				//std::string logit("CPU Temp is: " + fTemp + " Milicelcius!");
				//syslog(LOG_NOTICE, logit.c_str() , LOG_DAEMON);
				return std::atof(fTemp.c_str()) / 1000;
			}
		}else
		{
			syslog(LOG_ERR, "Could not load thermal information, directory not found!");
			return 0;
		}
	}
	return 0;
	#endif

	#ifdef __APPLE__

	return 0;
	#endif		
}

int CSystem::getSOCode(std::string soName)
{
	if(soName == "Linux")
		return (int)so_code::linux;
	if(soName == "Windows")
		return (int)so_code::windows;
	if(soName == "MacOS")
		return (int)so_code::mac;

	return 0;
}