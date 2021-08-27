/*
 * CNetwork.h
 *
 *  Created on: 26 de mai. de 2021
 *      Author: milvus
 */

#ifndef CNETWORK_H_
#define CNETWORK_H_

#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#ifdef __linux__
    #include <arpa/inet.h>
    #include <netpacket/packet.h>
    #include <net/ethernet.h>
#endif
#ifdef __APPLE__
    #include <net/if_dl.h>
#endif
#include <arpa/inet.h>
#include <map>
#include <vector>
#include <sstream> 

typedef struct NetworkData{
	std::string ip4;
	std::string ip6;
	std::string interfaceName;
	std::string macAddress;
} NetworkData;

class CNetwork {
public:
	CNetwork();
	virtual ~CNetwork();
	//std::string getIP(){return m_ip;}
	//std::string getInterfaceName(){return m_interfaceName;}
	//std::string getMAC(){return m_macAddress;}
	std::vector<NetworkData> GetNetworkData(){return m_vNetworkData;}
	NetworkData getActiveAdaptorData();
	const std::vector<NetworkData>& getAdaptorList(){return m_vNetworkData;}
	void PrintData();

private:
	void setNetworkInfo();
	std::vector<NetworkData> m_vNetworkData;

	#ifdef __linux__
	const bool m_insertMACColonSeparator = true;
	#endif

};

#endif /* CNETWORK_H_ */
