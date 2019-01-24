#pragma once
#include <WinSock2.h>


/* 4 bytes IP address */
typedef struct {
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
} ip_address;


/* Ethernet Header */
typedef struct {
    u_char  mac_dest[6];    //Total 48 bits
    u_char  mac_src[6];     //Total 48 bits 
    u_short type;           //16 bits 
} ethernet_header;


/* IPv4 header */
typedef struct {
    u_char		ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char		tos;            // Type of service 
    u_short		tlen;           // Total length 
    u_short		identification; // Identification
    u_short		flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char		ttl;            // Time to live
    u_char		proto;          // Protocol
    u_short		crc;            // Header checksum
    ip_address  saddr;			// Source address
    ip_address  daddr;			// Destination address
    u_int		op_pad;         // Option + Padding
} ip_header;


/* UDP header*/
typedef struct {
    u_short sport;          // Source port
    u_short dport;          // Destination port
    u_short len;            // Datagram length
    u_short crc;            // Checksum
} udp_header;


typedef struct { 
    unsigned __int16  tcp_src; 
    unsigned __int16  tcp_dst;
    unsigned __int32  tcp_seq; 
    unsigned __int32  tcp_ack; 
    unsigned __int16  tcp_ctl; 
    unsigned __int16  tcp_winsz; 
    unsigned __int16  tcp_csum; 
    unsigned __int16  tcp_urg; 
} tcp_header;


/* RTP header*/
typedef struct rtp_header {
    unsigned __int8		version_p_x_cc;
    unsigned __int8		m_pt;
    unsigned __int16	seq;            /* sequence number */
    unsigned __int32	ts;             /* timestamp */
    unsigned __int32	ssrc;           /* synchronization source */
    unsigned __int32	csrc[16];       /* optional CSRC list */
} rtp_header;
