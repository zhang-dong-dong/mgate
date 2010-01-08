
/*
 *      qq.cpp
 *
 *      Copyright 2009 microcai <microcai@microcai.com>
 *
 *      This program is non-free software; you can not redistribute it
 *      and/or modify it.
 *
 *      This program is distributed as a plugin for monitor.
 *      This plugin is Copyrighted! It contains my company's hard work.
 *      QQ is so hard too analyse! You should respect our work.
 */

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <string.h>

#include "libdreamtop.h"

#include <map>
#include <ctime>
std::map<std::string, time_t> qq_time_map;

#define QQ_DPORT  0x401F //8000
#define QQ_HTTPDPORT  0x5000 //80
#define QQ_VIPDPORT  0xBB01 //443
#define QQ_SPORT1 0x8813 //5000
#define QQ_SPORT2 0xA00F //4000


std::string Type_QQ("1002");

static int record_QQ_number(u_int qq, in_addr_t ip,u_char*packet)
{
	//syslog(LOG_NOTICE,"QQ number is : %u\n",qq);

	static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

	char qqnum[80];
	sprintf(qqnum, "%u", qq);

#if 1
	time_t tmNow = time(NULL);
	pthread_mutex_lock(&lock);
	std::map<std::string, time_t>::iterator it = qq_time_map.begin();
	for (; it != qq_time_map.end();)
	{
		if ((tmNow - it->second) > 120)
			qq_time_map.erase(it++);
		else
			++it;
	}
	if (qq_time_map.find(qqnum) != qq_time_map.end())
	{
		pthread_mutex_unlock(&lock);
		return 0;
	}
	qq_time_map[qqnum] = tmNow;
	pthread_mutex_unlock(&lock);
#endif

	struct tcphdr* tcp = (tcphdr*)(packet + 14 + sizeof(iphdr));

    struct NetAcount na(NetAcountType_QQ,packet);
    na.ip = ip;
    strcpy(na.strType, Type_QQ.c_str());
	na.data="";
    na.passwd = qqnum;
    na.ip = ip;

    na.dstip= * ( in_addr_t *) (packet +  28);
    na.dport = ntohs(tcp->dest);

    //RecordAccout(&na);
    RecordAccout(&na);

    return 1;

}

static int qq_packet_callback ( struct so_data* sodata,u_char * packet )
{
	u_int	iQQnum=0;
	u_char *pQQNumber = ( u_char* ) &iQQnum ;

	struct iphdr * ip_head = ( struct iphdr* ) ( packet + 14 );

	if ( ip_head->protocol ==IPPROTO_UDP )
	{
		struct udphdr *udphead = ( struct udphdr * ) ( ( char* ) ip_head + ip_head->ihl*4 );
		u_char * udp_packet = ( u_char* ) udphead + sizeof ( struct udphdr );

		if ( udphead->len <=5 ) return 0;

		switch ( udphead->dest )
		{
			case QQ_DPORT:
				if ( ( udp_packet[0] == 0x02 ) && ( udp_packet[3] == 0x00 ) && ( udp_packet[4] == 0x62||udp_packet[4] == 0x22||udp_packet[4] == 0x91 ) )
				{
					for ( int i=0; i<4; i++ )
					{
						pQQNumber[3-i] =  udp_packet[7+i];
					}
					return record_QQ_number ( iQQnum , ip_head->saddr,packet);
				}
		}
	}
	else if ( ip_head->protocol == IPPROTO_TCP )
	{
		struct tcphdr *tcphead = ( struct tcphdr* ) ( ( char* ) ip_head + ip_head->ihl*4 );
		u_char*tcpdata = ( u_char* ) ( tcphead + tcphead->doff*4 );
		int tcpdataLen = ntohs(ip_head->tot_len) - ip_head->ihl*4 -  tcphead->doff*4;
		if ( tcpdataLen < 5 )
			return 0;
		switch ( tcphead->dest )
		{
			case QQ_HTTPDPORT:
				if ( tcpdata[0] == 0x00 && tcpdata[2] == 0x02
				        && ( tcpdata[3] == 0x0c || tcpdata[3] == 0x0d ) )
				{
					for ( int i=0; i<4; i++ )
					{
						pQQNumber[3-i] = tcpdata[9+i];
					}
					return record_QQ_number ( iQQnum, ip_head->saddr ,packet);
				}
				break;
			case QQ_VIPDPORT:
				if ( tcpdata[2] == 0x02 && tcpdata[5] == 0x00&&tcpdata[6] == 0x22 )
				{
					for ( int i=0; i<4; i++ )
					{
						pQQNumber[3-i] = tcpdata[9+i];
					}
					return record_QQ_number ( iQQnum , ip_head->saddr,packet);
				}
				break;
		}
	}
	return 0;
}

static void* protohander[3];
static void* base_addr;

extern "C" int __module_init(struct so_data*so)
{
	protohander[1] = register_protocol_handler ( qq_packet_callback,QQ_HTTPDPORT,IPPROTO_TCP );
	protohander[2] = register_protocol_handler ( qq_packet_callback,QQ_VIPDPORT,IPPROTO_TCP );

	protohander[0] = register_protocol_handler ( qq_packet_callback,QQ_DPORT ,IPPROTO_UDP );
	base_addr = so->module;
    return 0;
}

extern "C" int	so_can_unload(  )
{


	return 1;
}

static void __attribute__((destructor)) so__unload(void)
{
	// here, we need to
	for ( int i=0; i< 3;++i )
		un_register_protocol_handler ( protohander[i] );
	sleep(4);
}


char module_name[]="QQ号码分析";
