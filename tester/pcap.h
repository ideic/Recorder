#pragma once
/*
This file is an subset of the original pcap.h version 2.4,
This only contains the field that we use
*/
#include <cstdint>
#include <WinSock2.h>

struct pcap_file_header {
	uint32_t magic;
	u_short version_major;
	u_short version_minor;
	int32_t thiszone;	/* gmt to local correction */
	uint32_t sigfigs;	/* accuracy of timestamps */
	uint32_t snaplen;	/* max length saved portion of each pkt */
	uint32_t linktype;	/* data link type (LINKTYPE_*) */
};


struct pcap_pkthdr {
	struct timeval ts;	/* time stamp */
	uint32_t caplen;	/* length of portion present */
	uint32_t len;	/* length this packet (off wire) */
};

