/*
 * CNetwork.cpp
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#include "CNetwork.h"

CNetwork::CNetwork() {
	setNetworkInfo();

}

CNetwork::~CNetwork() {
	// TODO Auto-generated destructor stub
}

template <typename M, typename V>
    static void MapToVec( const  M & m, V & v ) {
    for( typename M::const_iterator it = m.begin(); it != m.end(); ++it ) {
        v.push_back( it->second );
    }
}

void CNetwork::setNetworkInfo()
{
	m_vNetworkData.clear();
    struct ifaddrs *ifaddr=NULL;
    struct ifaddrs *ifa = NULL;
    unsigned char *ptr;
    void * tmpAddrPtr=NULL;
    
    std::map<std::string, NetworkData> mapNetwork;
    std::map<std::string, NetworkData>::iterator it;

    if (getifaddrs(&ifaddr) == -1)
    {
         std::cerr << "getifaddrs() FAIL!" << std::endl;
    }
    else
    {
         for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
         {
             
            #ifdef __linux__
             int i = 0;
        	 //Gets MAC address.
        	 if ( ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_PACKET) )
             {
                  struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                  std::ostringstream mac;
                  for (i=0; i < s->sll_halen; i++)
                  {
                      //printf("%02x%c", (s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\n');
                      mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)s->sll_addr[i];
                      if(m_insertMACColonSeparator && i+1!=s->sll_halen) mac << ":";
                  }
                  it = mapNetwork.find(ifa->ifa_name);
                  if(it != mapNetwork.end())
                  {
                	  it->second.macAddress = mac.str();
                  }else
                  {
                	  NetworkData nt;
                	  nt.macAddress = mac.str();
                	  nt.interfaceName = ifa->ifa_name;
                	  mapNetwork.insert(std::make_pair(nt.interfaceName, nt));
                  }
             }
            #endif

            #ifdef __APPLE__
            if (((ifa)->ifa_addr)->sa_family == AF_LINK) {
                ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifa)->ifa_addr);
                std::ostringstream mac;
                
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*ptr << ":" ;
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*(ptr+1) << ":";
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*(ptr+2) << ":";
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*(ptr+3) << ":";
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*(ptr+4) << ":";
                mac << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)*(ptr+5);
                it = mapNetwork.find(ifa->ifa_name);
                if(it != mapNetwork.end())
                {
                    it->second.macAddress = mac.str();
                }else
                {
                    NetworkData nt;
                    nt.macAddress = mac.str();
                    nt.interfaceName = ifa->ifa_name;
                    mapNetwork.insert(std::make_pair(nt.interfaceName, nt));
                }                                    
            }
            #endif


        	 // check it is IP4
             if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET)
             {
                 // is a valid IP4 Address
                 tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                 char addressBuffer[INET_ADDRSTRLEN];
                 inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                 //printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
                 it = mapNetwork.find(ifa->ifa_name);
                 if(it != mapNetwork.end())
                 {
                	 it->second.ip4 = addressBuffer;
                 }else
                 {
					  NetworkData nt;
					  nt.ip4 = addressBuffer;
					  nt.interfaceName = ifa->ifa_name;
					  mapNetwork.insert(std::make_pair(nt.interfaceName, nt));
                 }

              }
             // check it is IP6
             else if ((ifa->ifa_addr) && ifa->ifa_addr->sa_family==AF_INET6)
             {
                     // is a valid IP6 Address
                     tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                     char addressBuffer[INET6_ADDRSTRLEN];
                     inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                     //printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
                     it = mapNetwork.find(ifa->ifa_name);
                     if(it != mapNetwork.end())
                     {
                    	 it->second.ip6 = addressBuffer;
                     }else
                     {
    					  NetworkData nt;
    					  nt.ip6 = addressBuffer;
    					  nt.interfaceName = ifa->ifa_name;
    					  mapNetwork.insert(std::make_pair(nt.interfaceName, nt));
                     }
             }
         }

         MapToVec(mapNetwork, m_vNetworkData);
         freeifaddrs(ifaddr);
    }
}

NetworkData CNetwork::getActiveAdaptorData()
{
    NetworkData adaptor;
    for(std::vector<NetworkData>::const_iterator it = m_vNetworkData.begin(); it < m_vNetworkData.end(); ++it)
	{
        if(!it->ip4.empty() && it->ip4 != "127.0.0.1")
        {
            adaptor.interfaceName = it->interfaceName;
            adaptor.macAddress = it->macAddress;
            adaptor.ip4 = it->ip4;
            adaptor.ip6 = it->ip6;

        }
	}
    return adaptor;
}

void CNetwork::PrintData()
{
	std::cout << "Network map:" << std::endl;
	for(std::vector<NetworkData>::const_iterator it = m_vNetworkData.begin(); it < m_vNetworkData.end(); ++it)
	{
		std::cout << "  Adaptor Name : '" << it->interfaceName
				  << ((!it->ip4.empty() && it->ip4 != "127.0.0.1")? "' [ACTIVE]":"'")<< std::endl;
		std::cout << "\tMac          : " << it->macAddress << std::endl;
		std::cout << "\tIp4          : " << it->ip4 << std::endl;
		std::cout << "\tIp6          : " << it->ip6 << std::endl;
		std::cout << std::endl;
	}
}
