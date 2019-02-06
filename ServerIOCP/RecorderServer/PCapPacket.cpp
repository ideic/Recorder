#include "stdafx.h"
#include "PCapPacket.h"
#include <stdexcept>
PcapPacket::PcapPacket(size_t bufferSize)
{
	this->bufferSize = bufferSize;
	buffer = new char[bufferSize];
	Reset();
}

PcapPacket::~PcapPacket()
{
	if (buffer)
		delete[] buffer;
}

void PcapPacket::Reset()
{
	memset(buffer, 0, bufferSize);
	SetPayloadSize(0);
	SetTimeH(0);
	SetTimeL(0);
}

bool PcapPacket::MaybeIP()
{
	if (PcapPacket::GetPayloadSize()>34 && ((EthernetPacket*)this)->GetEthPayload() == 0x0800)
		return true;
	else
		return false;
}

bool PcapPacket::MaybeUDP()
{
	if (MaybeIP() && PcapPacket::GetPayloadSize()>42 && ((IPPacket*)this)->GetProtocol() == 0x11)
		return true;
	else
		return false;
}

//Fucking Cisco-VTA may use any port not only even for RTP
/*bool PcapPacket::MaybeRTP()
{
if(PcapPacket::GetPayloadSize()>54
&& MaybeUDP()
&& ((RTPPacket*)this)->GetVersion()==2
&& (((RTPPacket*)this)->GetPType()<35 || ((RTPPacket*)this)->GetPType()>95)
&& (((UDPPacket*)this)->GetDstPort()%2)==0)
return true;
else
return false;
}*/


bool PcapPacket::MaybeRTP()
{
	if (PcapPacket::GetPayloadSize()>54
		&& MaybeUDP()
		&& ((RTPPacket*)this)->GetVersion() == 2
		&& (((RTPPacket*)this)->GetPType()<72 || ((RTPPacket*)this)->GetPType()>77))
		return true;
	else
		return false;
}


bool PcapPacket::MaybeRTCP()
{
	if (MaybeUDP() && !MaybeRTP() && ((RTPPacket*)this)->GetVersion() == 2)
		return true;
	else
		return false;
}

EthernetPacket::EthernetPacket(size_t bufferSize)
	:PcapPacket(bufferSize)
{
	Reset();
}

void EthernetPacket::Reset()
{
	PcapPacket::Reset();
	SetSrcMAC(0x000000000000);
	SetDstMAC(0xFFFFFFFFFFFF);
	SetEthPayload(0x0800);
	SetPayloadSize(0);
}

IPPacket::IPPacket(size_t bufferSize)
	:EthernetPacket(bufferSize)
{
	Reset();
}

void IPPacket::Reset()
{
	EthernetPacket::Reset();
	char *buffer = EthernetPacket::GetPayload();

	//Set some initial fields of header
	//Ethernet header
	//IP header
	//Version and header size
	buffer[0] = 0x45;
	//DSF
	buffer[1] = (char)0x88;
	//Length
	*(unsigned short*)&buffer[2] = ntohs(20);
	//ID
	*(unsigned short*)&buffer[4] = ntohs(rand());
	//Flags & offset
	*(unsigned short*)&buffer[6] = 0;
	//TTL
	buffer[8] = 64;
	//Protocol
	buffer[9] = 0x11;
	//Checksum
	*(unsigned short*)&buffer[10] = 0;
	//Src IP
	*(unsigned int*)&buffer[12] = inet_addr("127.0.0.1");;
	//Dst IP
	*(unsigned int*)&buffer[16] = inet_addr("127.0.0.1");;
	SetPayloadSize(0);
}

UDPPacket::UDPPacket(size_t bufferSize)
	:IPPacket(bufferSize)
{
	Reset();
}

void UDPPacket::Reset()
{
	IPPacket::Reset();
	char *buffer = IPPacket::GetPayload();
	*(unsigned short*)&buffer[0] = htons(1024);
	*(unsigned short*)&buffer[2] = htons(1024);
	*(unsigned short*)&buffer[4] = 0;
	*(unsigned short*)&buffer[6] = 0;
}

RTPPacket::RTPPacket(size_t bufferSize)
	:UDPPacket(bufferSize)
{
	Reset();
}

void RTPPacket::GetCSRC(unsigned int *csrc, unsigned int &count) const
{
	char *buffer = UDPPacket::GetPayload();
	unsigned int *p = (unsigned int*)&buffer[12];
	count = buffer[0] & 0x0F;
	for (unsigned int i = 0; i<count; i++)
		csrc[i] = ntohl(p[i]);
}

void RTPPacket::Reset()
{
	UDPPacket::Reset();
	SetVersion(2);
	SetPadding(0);
	SetExtension(0);
	SetCSRC(0, 0);
	SetMarker(0);
	SetPType(0);
	SetSeq(0);
	SetTS(0);
	SetSSRC(0);
	SetPayloadSize(0);
}

void RTPPacket::SetCSRC(unsigned int *csrc, unsigned int count)
{
	if (count>16)
		throw std::logic_error("Number of csrc cannot be greater than 16.");
	
	char *buffer = UDPPacket::GetPayload();
	if (csrc == 0 || count == 0)
	{
		count = 0;
	}
	else
	{
		unsigned int *p = (unsigned int*)&buffer[12];
		for (unsigned int i = 0; i<count; i++)
			p[i] = htonl(csrc[i]);
	}
	buffer[0] &= ~0x0F;
	buffer[0] |= count;
}