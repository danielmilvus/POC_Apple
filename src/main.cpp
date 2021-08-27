#include <iostream>
#include <sys/sysctl.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include "CNetwork.h"
#include "CHardware.h"
#include "CSystem.h"

#include <unistd.h>

void find_pids(const char *name)
{
    pid_t pids[2048];
    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    int n_proc = bytes / sizeof(pids[0]);
    for (int i = 0; i < n_proc; i++) {
        struct proc_bsdinfo proc;
        int st = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0,
                             &proc, PROC_PIDTBSDINFO_SIZE);
        if (st == PROC_PIDTBSDINFO_SIZE) {
            if (strcmp(name, proc.pbi_name) == 0) {
                /* Process PID */
                printf("%d [%s] [%s]\n", pids[i], proc.pbi_comm, proc.pbi_name);                
            }
        }       
    }
}

void print_proc_EXE_path()
{
// #define PROC_ALL_PIDS           1
// #define PROC_PGRP_ONLY          2
// #define PROC_TTY_ONLY           3
// #define PROC_UID_ONLY           4
// #define PROC_RUID_ONLY          5
// #define PROC_PPID_ONLY          6
// #define PROC_KDBG_ONLY          7

    int numberOfProcesses = proc_listpids(PROC_PGRP_ONLY, KERN_PROC, NULL, 0);
    pid_t pids[numberOfProcesses];
    bzero(pids, sizeof(pids));
    proc_listpids(PROC_ALL_PIDS, KERN_PROC, pids, sizeof(pids));
    for (int i = 0;   i < numberOfProcesses;  ++i) {
        if (pids[i] == 0) { continue;  }
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));

        //char pInfo[PROC_PIDFDPSEMINFO_SIZE];
        //bzero(pInfo, PROC_PIDPATHINFO_MAXSIZE);
        //proc_pidinfo(pid[i], pInfo, )
        if (strlen(pathBuffer) > 0) {
            std::cout << i << " - Pid: " << pids[i] << " :" << pathBuffer << std::endl;
            //printf("path : %s\n", pathBuffer);
        }
    }  
    std::cout << "\n*\n#\n*# EXE Qtd Proc: " << numberOfProcesses << std::endl; 
}

void print_process_info()
{

    int i, mib[4];
    size_t len;
    struct kinfo_proc kp;

    /* Fill out the first three components of the mib */
    len = 4;
    sysctlnametomib("kern.proc.pid", mib, &len);
    std::cout << "mib[0]:" << mib[0] << " mib[1]:" << mib[1] << " mib[2]" << mib[2] << std::endl; 
    std::vector<std::string> procNames;
    int j = 0;
    /* Fetch and print entries for pid's < 100 */
    for (i = 0; i < 10000; i++) {
            mib[3] = i;
            len = sizeof(kp);
            if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1)
            {
                std::cerr << "sysctl error" << std::endl;
                    perror("sysctl");
            }
            else if (len > 0)
            {
                ++j;
                std::vector<std::string>::iterator it = std::find( procNames.begin(), procNames.end(), kp.kp_proc.p_comm );
                if(it == procNames.end())
                {
                    std::cout << "PID '" << kp.kp_proc.p_pid  << "'  Name: " << kp.kp_proc.p_comm <<  std::endl;
                    procNames.push_back(kp.kp_proc.p_comm);
                }
                
            } 
    }
    std::cout << "&& 16 BITS Qtd Proc: " << procNames.size() << "  _J: " << j << std::endl;
    std::cout << "mib[0]:" << mib[0] << " mib[1]:" << mib[1] << " mib[2]" << mib[2] << std::endl; 
}

uint getUidUsingSysctl(uint pid)
{
    struct kinfo_proc *sProcesses = NULL, *sNewProcesses;
    int    aiNames[4];
    size_t iNamesLength;
    int    i, iRetCode, iNumProcs;
    size_t iSize;

    iSize = 0;
    aiNames[0] = CTL_KERN;
    aiNames[1] = KERN_PROC;
    aiNames[2] = KERN_PROC_ALL;
    aiNames[3] = 0;
    iNamesLength = 3;

    iRetCode = sysctl(aiNames, iNamesLength, NULL, &iSize, NULL, 0);

    /* allocate memory and populate info in the  processes structure */
    do
    {
        iSize += iSize / 10;
        sNewProcesses = (kinfo_proc *)realloc(sProcesses, iSize);

        if (sNewProcesses == 0)
        {
            if (sProcesses)
                free(sProcesses);
            /* could not realloc memory, just return */
            return -1;
        }
        sProcesses = sNewProcesses;
        iRetCode = sysctl(aiNames, iNamesLength, sProcesses, &iSize, NULL, 0);

        

    } while (iRetCode == -1 && errno == ENOMEM);

    iNumProcs = iSize / sizeof(struct kinfo_proc);
    int j = 0;
    for (i = 0; i < iNumProcs; i++)
    {
        ++j;
        std::cout << i << " Proc: " << sProcesses[i].kp_proc.p_comm << std::endl;
        if (sProcesses[i].kp_proc.p_pid == pid) 
        {
            return sProcesses[i].kp_eproc.e_ucred.cr_uid;
        }

    }
    std::cout << "Totla = " << j << std::endl;
    /* clean up and return to the caller */
    free(sProcesses);

    return -1;
}

unsigned long long getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}



#include <cmath>
#define MemoryMultiplier 0.0009765625
#define ByteInGBMultiplier 0.000000001

std::string FormatBytes(unsigned long long bytes)
{
    std::vector<std::string> Suffix = { "B", "KB", "MB", "GB", "TB" };
    int i;
    double dblSByte = bytes;
    for (i = 0; i < Suffix.size() && bytes >= 1024; ++i, bytes /=1024) 
    {
        //std::cout << dblSByte << Suffix[i] << std::endl;
        dblSByte = bytes / 1024;
    }
    return std::string(std::to_string(bytes) + Suffix[i]);
}

uint64_t getTotalMemorySysctl()
{
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    u_int namelen = sizeof(mib) / sizeof(mib[0]);
    uint64_t size;
    size_t len = sizeof(size);

    if (sysctl(mib, namelen, &size, &len, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    else
    {
        //printf("HW.HW_MEMSIZE = %llu bytes\n", size * ByteInGBMultiplier);
        //std::cout << FormatBytes(size) << std::endl;
        //FormatBytes(size);
        return size;
    }
    return 0;
}

int printMemoryInfo()
{
    //struct vmmeter vmm;
    // struct vmtotal vmt;

    // int mib[2] = {CTL_VM , VM_METER};
    // size_t len = sizeof(vmt);
    // vm_size_t pagesize = getpagesize();
    // //char *p;
    // std::cout << "\n** Pages: " << pagesize << std::endl;
    // sysctl(mib, 2, &vmt, &len, NULL, 0);

    // std::cout << "Frai: " << vmt.t_free << std::endl;
    // std::string strFree(FormatBytes( vmt.t_free * pagesize ));
    // std::cout << "** Free: " << strFree << " | Tot vm: " << FormatBytes(vmt.t_vm * pagesize)  << " | Tot Rm use: " << FormatBytes(vmt.t_rm) << std::endl;//  << "  Used: " << FormatBytes( vmm.v_page_size * (vmm.v_active_count + vmm.v_inactive_count + vmm.v_wire_count)) << std::endl;
    // return 0;  

    // mach_port_t host_port;
    // mach_msg_type_number_t host_size;
    // vm_size_t pagesize;

    // host_port = mach_host_self();
    // host_size = sizeof(vm_statistics_data_t) / sizeof(natural_t);
    // host_page_size(host_port, &pagesize);        

    // vm_statistics_data_t vm_stat;

    // if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) {
    //     std::cerr << "Failed to fetch vm statistics" << std::endl;
    // }else
    // {
    //     std::cout << "Pagesize: " << pagesize << std::endl;

    //     /* Stats in bytes */ 
    //     int64_t mem_used = (int64_t)(vm_stat.active_count +
    //                         vm_stat.inactive_count +
    //                         vm_stat.wire_count ) * (int64_t)pagesize;
    //     int64_t mem_free = (int64_t)vm_stat.speculative_count * (int64_t)pagesize;
    //     int64_t mem_total = (int64_t)mem_used + mem_free;

    //     std::cout << "Used: " << FormatBytes(mem_used) << " Free: " << FormatBytes(mem_free)  << " Total: " << FormatBytes(mem_total)  << std::endl;
    // }


    mach_msg_type_number_t host_size = HOST_VM_INFO64_COUNT;
    mach_port_t host_port = mach_host_self();
    vm_size_t pagesize;

    //host_size = sizeof(vm_statistics64_data_t) / sizeof(natural_t);
    host_page_size(host_port, &pagesize);        

    vm_statistics64_data_t vm_stat;

    if (host_statistics64(host_port, HOST_VM_INFO64, (host_info64_t)&vm_stat, &host_size) != KERN_SUCCESS) {
        std::cerr << "Failed to fetch vm statistics" << std::endl;
    }else
    {
        /* Stats in bytes */ 
        natural_t mem_used = (vm_stat.active_count +
                            vm_stat.inactive_count +
                            vm_stat.wire_count + 
                            vm_stat.compressor_page_count +
                            vm_stat.speculative_count) * pagesize;
                            //std::cout << "total_uncompressed_pages_in_compressor: " <<vm_stat.total_uncompressed_pages_in_compressor << std::endl;
        natural_t mem_free = vm_stat.free_count * pagesize;
        natural_t mem_total = mem_used + mem_free;
        

        std::cout << "\nactive_count: " << vm_stat.active_count  << "\ninactive_count: " << vm_stat.inactive_count  << "\nwire_count: " << vm_stat.wire_count <<
                "\nthrottled_count: " << vm_stat.throttled_count  << "\npurgeable_count: " << vm_stat.purgeable_count  << "\nvm_stat.zero_fill_count: " << vm_stat.internal_page_count <<
                "\nexternal_page_count: " << vm_stat.external_page_count  << "\ntotal_uncompressed_pages_in_compressor: " << vm_stat.total_uncompressed_pages_in_compressor  << "\nvm_stat.cow_faults: " << vm_stat.cow_faults <<
                "\ncompressor_page_count: " << vm_stat.compressor_page_count  << "\nspeculative_count: " << vm_stat.speculative_count  << "\nvm_stat.zero_fill_count: " << vm_stat.zero_fill_count << 
                "\nfree_count: " << vm_stat.free_count  << "\nspeculative_count: " << vm_stat.speculative_count << std::endl;
        
        std::cout << "\nUsed: " << FormatBytes(mem_used)  << " Free: " << FormatBytes(mem_free)  << " Total: " << FormatBytes(mem_total)  << std::endl;
    }//4068746

    return 0;
}

#include <syslog.h>

int main ()
{
    std::cerr << "Hello Mac" << std::endl;
    //perror("Teste saida de logs...");
    syslog(LOG_ERR, "MILVUS TEST OUTPUT LOG");
    CNetwork cNet;
    cNet.PrintData();

    CHardware oHW;
		std::cout << std::left << std::setw(15) << "Station:" << (oHW.isNotebook() ? "Notebook":"Desktop") << std::endl;
		std::cout << std::left << std::setw(15) << "CPU-Name:" << oHW.getCPUName() << std::endl;
		std::cout << std::left << std::setw(15) << "MB-Name:" << oHW.getMotherBoardName() << std::endl;    

    CSystem oSys;
		//CSystem oSys;
		std::cout << std::left << std::setw(15) << "OS:" << oSys.getOSName() << std::endl;
		std::cout << std::left << std::setw(15) << "Version:" << oSys.getOSVersion() << std::endl;
		std::cout << std::left << std::setw(15) << "Hostname:" << oSys.getOSHostName() << std::endl;

		std::cout << std::left << std::setw(15) << "\nHDD Size:" << FormatBytes(oSys.getHDDSize()) << std::endl;
		std::cout << std::left << std::setw(15) << "HDD Free:" << FormatBytes(oSys.getHDDFreeSpace())<< std::endl;

        oSys.printProcByMemoryUsage();

        std::cout << "getCPUCores: " << oSys.getCPUCores() << std::endl;
        
        int i = 10;
        do
        {
            std::cout << "<>< ><> <>< ><> <>< ><> <>< ><> <>< ><> <>< ><> <>< ><> <>< ><> <>< ><> <>< ><>" << std::endl;
            std::cout << "getTotalSystemMemory: " << FormatBytes(getTotalSystemMemory()) << std::endl;
            std::cout << "getTotalMemorySysctl: " << FormatBytes(getTotalMemorySysctl()) << std::endl;

                // std::cout << std::fixed;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "\nRAM-Total:" << oSys.getRAMTotal() << " GB" << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Free:" << oSys.getRAMFree() << " GB"  << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Available:" << oSys.getRAMAvailable() << " GB"  << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Cached:" << oSys.getRAMCached() << " GB"  << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Swap-Total:" << oSys.getRAMSwapTotal() << " GB"  << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Swap-Free:" << oSys.getRAMSwapFree() << " GB"  << std::endl;
                // std::cout << std::left << std::setw(15) << std::setprecision(2) << "RAM-Swap-Cashed:" << oSys.getRAMSwapCached() << " GB"  << std::endl;             
            //std::cout << "printMemoryInfo: \n" << printMemoryInfo() << std::endl;            
            printMemoryInfo();
            std::cout << "<>< CPU Load: " << oSys.getCPUUsed() * 100 << "%" << std::endl;
            sleep(1);
            --i;
        }while(i);

        //std::cout << std::left << std::setw(15) << "Logged User:" << getenv("USER") << std::endl;
		
        // std::cout << "\nCPU Temperature:" << std::setprecision(2) << oSys.getCPUTempCelsius() << "\u2103" << std::endl;
		// std::cout << "\nCPU Temperature:" << std::setprecision(2) << oSys.getCPUTempFahrenheit() << "\u2109" << std::endl;

    return 0;
}
