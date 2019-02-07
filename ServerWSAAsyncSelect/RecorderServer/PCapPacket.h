#pragma once
#include <WinSock2.h>
#include <windows.h>
class PcapPacket
{
protected:
	char *buffer;
	size_t bufferSize;

public:
	PcapPacket(size_t bufferSize);
	virtual ~PcapPacket();

	size_t GetBufferSize() const { return bufferSize; };
	unsigned int GetPayloadSize() const { return *(unsigned int*)&buffer[8]; }
	unsigned int GetPayloadOffset() const { return 16; }
	char *GetPayload() const { return &this->buffer[GetPayloadOffset()]; }
	void GetPcapSize(unsigned int &incLen, unsigned int &origLen) const { incLen = *(unsigned int*)&buffer[8]; origLen = *(unsigned int*)&buffer[12]; }
	void GetPcapBuffer(char **buffer, unsigned int &size) { *buffer = this->buffer; size = GetPayloadSize() + 16; }
	unsigned int GetTimeH() const { return *(unsigned int*)&buffer[0]; }
	unsigned int GetTimeL() const { return *(unsigned int*)&buffer[4]; }

	void SetPayloadSize(unsigned int size) { *(unsigned int*)&buffer[8] = size; *(unsigned int*)&buffer[12] = size; }
	void SetPcapSize(unsigned int incLen, unsigned int origLen) { *(unsigned int*)&buffer[8] = incLen; *(unsigned int*)&buffer[12] = origLen; }
	void SetTimeH(unsigned int timeH) { *(unsigned int*)&buffer[0] = timeH; }
	void SetTimeL(unsigned int timeL) { *(unsigned int*)&buffer[4] = timeL; }

	bool MaybeIP();
	bool MaybeUDP();
	bool MaybeRTP();
	bool MaybeRTCP();

	void Reset();
};

class EthernetPacket : public PcapPacket
{
public:
	EthernetPacket(size_t bufferSize);

	unsigned long long GetSrcMAC() const
	{
		long long mac;

		char *buffer = PcapPacket::GetPayload();
		char *pMac = (char*)&mac;

		//Big Endian to Little Endian...
		pMac[5] = buffer[0];
		pMac[4] = buffer[1];
		pMac[3] = buffer[2];
		pMac[2] = buffer[3];
		pMac[1] = buffer[4];
		pMac[0] = buffer[5];

		return mac;
	}

	unsigned long long GetDstMAC() const
	{
		long long mac;

		char *buffer = PcapPacket::GetPayload();
		char *pMac = (char*)&mac;

		//Big Endian to Little Endian...
		pMac[5] = buffer[6];
		pMac[4] = buffer[7];
		pMac[3] = buffer[8];
		pMac[2] = buffer[9];
		pMac[1] = buffer[10];
		pMac[0] = buffer[11];

		return mac;
	}

	unsigned short GetEthPayload() const
	{
		unsigned short ethPayload = 0;
		const char *buffer = PcapPacket::GetPayload();
		ethPayload = ntohs(*(unsigned short*)&buffer[12]);
		return ethPayload;
	}

	unsigned int GetPayloadSize() const { return PcapPacket::GetPayloadSize() - 14; }
	unsigned int GetPayloadOffset() const { return 30; }
	char *GetPayload() const { return &this->buffer[GetPayloadOffset()]; }

	void SetSrcMAC(unsigned long long mac)
	{
		char *pMac = (char*)&mac;
		char *buffer = PcapPacket::GetPayload();
		//Little Endian to Big Endian...
		buffer[0] = pMac[5];
		buffer[1] = pMac[4];
		buffer[2] = pMac[3];
		buffer[3] = pMac[2];
		buffer[4] = pMac[1];
		buffer[5] = pMac[0];
	}

	void SetDstMAC(unsigned long long mac)
	{
		char *buffer = 0, *pMac = (char*)&mac;
		unsigned int size = 0;
		buffer = PcapPacket::GetPayload();
		//Little Endian to Big Endian...
		buffer[6] = pMac[5];
		buffer[7] = pMac[4];
		buffer[8] = pMac[3];
		buffer[9] = pMac[2];
		buffer[10] = pMac[1];
		buffer[11] = pMac[0];
	}

	void SetEthPayload(unsigned short ethPayload) { char *buffer = PcapPacket::GetPayload(); *(unsigned short*)&buffer[12] = htons(ethPayload); }
	void SetPayloadSize(unsigned int size) { PcapPacket::SetPayloadSize(size + 14); }

	void Reset();
};

class IPPacket : public EthernetPacket
{
public:
	IPPacket(size_t bufferSize);

	unsigned int GetSrcAddr() const { char *buffer = EthernetPacket::GetPayload(); return ntohl(*(unsigned int*)&buffer[12]); }
	unsigned int GetDstAddr() const { char *buffer = EthernetPacket::GetPayload(); return ntohl(*(unsigned int*)&buffer[16]); }
	unsigned char GetProtocol() const { char *buffer = EthernetPacket::GetPayload(); return buffer[9]; }
	unsigned int GetPayloadSize() const { char *buffer = EthernetPacket::GetPayload(); return ntohs(*(unsigned short*)&buffer[3]) - (buffer[0] & 0x0F) * 4; }
	unsigned int GetPayloadOffset() const { char *buffer = EthernetPacket::GetPayload(); return EthernetPacket::GetPayloadOffset() + (buffer[0] & 0x0F) * 4; }
	char *GetPayload() const { return &this->buffer[GetPayloadOffset()]; }

	void SetSrcAddr(unsigned int addr) { char *buffer = EthernetPacket::GetPayload(); *(unsigned int*)&buffer[12] = htonl(addr); }
	void SetDstAddr(unsigned int addr) { char *buffer = EthernetPacket::GetPayload(); *(unsigned int*)&buffer[16] = htonl(addr); }
	void SetProtocol(unsigned char protocol) { char *buffer = EthernetPacket::GetPayload(); buffer[9] = protocol; }

	void SetPayloadSize(unsigned int size)
	{
		char *buffer = EthernetPacket::GetPayload();
		size += (buffer[0] & 0x0F) * 4;
		EthernetPacket::SetPayloadSize(size);
		*(unsigned short*)&buffer[2] = htons(size);
	}

	void Reset();
};

class UDPPacket : public IPPacket
{
public:
	UDPPacket(size_t bufferSize);

	unsigned short GetSrcPort() const { char* buffer = IPPacket::GetPayload(); return ntohs(*(unsigned short*)&buffer[0]); }
	unsigned short GetDstPort() const { char* buffer = IPPacket::GetPayload(); return ntohs(*(unsigned short*)&buffer[2]); }
	unsigned int GetPayloadSize() const { char *buffer = IPPacket::GetPayload(); return ntohs(*(unsigned short*)&buffer[4]) - 8; }
	unsigned int GetPayloadOffset() const { return IPPacket::GetPayloadOffset() + 8; }
	char *GetPayload() const { return &this->buffer[UDPPacket::GetPayloadOffset()]; }

	void SetSrcPort(unsigned short srcPort) { char *buffer = IPPacket::GetPayload(); *(unsigned short*)&buffer[0] = htons(srcPort); }
	void SetDstPort(unsigned short dstPort) { char *buffer = IPPacket::GetPayload(); *(unsigned short*)&buffer[2] = htons(dstPort); }

	void SetPayloadSize(unsigned int size)
	{
		IPPacket::SetPayloadSize(size + 8);
		char *buffer = IPPacket::GetPayload();
		*(unsigned short*)&buffer[4] = htons(size + 8);
	}

	void Reset();
};

class RTPPacket : public UDPPacket
{
public:
	RTPPacket(size_t bufferSize);

	unsigned int GetTS() const { char *buffer = UDPPacket::GetPayload(); return ntohl(*(unsigned int*)&buffer[4]); }
	unsigned int GetSSRC() const { char *buffer = UDPPacket::GetPayload(); return ntohl(*(unsigned int*)&buffer[8]); }
	void GetCSRC(unsigned int *cssrc, unsigned int &count) const;
	unsigned short GetSeq() const { char *buffer = UDPPacket::GetPayload(); return ntohs(*(unsigned short*)&buffer[2]); }
	unsigned char GetVersion() const { unsigned char *buffer = (unsigned char*)UDPPacket::GetPayload(); return buffer[0] >> 6; }
	unsigned char GetPadding() const { char *buffer = UDPPacket::GetPayload(); return buffer[0] & 0x20; }
	unsigned char GetExtension() const { char *buffer = UDPPacket::GetPayload(); return buffer[0] & 0x10; }
	unsigned int GetPayloadSize() const { char *buffer = UDPPacket::GetPayload(); return UDPPacket::GetPayloadSize() - (12 + (buffer[0] & 0x0F) * 4); }
	unsigned int GetPayloadOffset() const { char *buffer = UDPPacket::GetPayload(); return UDPPacket::GetPayloadOffset() + 12 + (buffer[0] & 0x0F) * 4; }
	unsigned char GetPType() const { char *buffer = UDPPacket::GetPayload(); return buffer[1] & 0x7F; }
	unsigned char GetMarker() const { char *buffer = UDPPacket::GetPayload(); return buffer[1] & 0x80; }
	char *GetPayload() const { return &this->buffer[RTPPacket::GetPayloadOffset()]; }

	void SetCSRC(unsigned int *csrc, unsigned int count);
	void SetExtension(unsigned char extension) { char *buffer = UDPPacket::GetPayload(); extension ? buffer[0] |= 0x10 : buffer[0] &= ~0x10; }
	void SetMarker(unsigned char marker) { char *buffer = UDPPacket::GetPayload(); marker ? buffer[1] |= 0x80 : buffer[1] &= ~0x80; }
	void SetPayloadSize(unsigned int size) { char *buffer = UDPPacket::GetPayload(); UDPPacket::SetPayloadSize(12 + (buffer[0] & 0x0F) * 4 + size); }
	void SetPadding(unsigned char padding) { char *buffer = UDPPacket::GetPayload(); padding ? buffer[0] |= 0x20 : buffer[0] &= ~0x20; }
	void SetTS(unsigned int ts) { char *buffer = UDPPacket::GetPayload(); *(unsigned int*)&buffer[4] = htonl(ts); }
	void SetSSRC(unsigned int ssrc) { char *buffer = UDPPacket::GetPayload(); *(unsigned int*)&buffer[8] = htonl(ssrc); }
	void SetSeq(unsigned short seq) { char *buffer = UDPPacket::GetPayload(); *(unsigned short*)&buffer[2] = htons(seq); }

	void SetPType(unsigned char ptype)
	{
		char *buffer = UDPPacket::GetPayload();
		ptype &= 0x7F;
		buffer[1] &= ~0x7F;
		buffer[1] |= ptype;
	}

	void SetVersion(unsigned char version)
	{
		unsigned char *buffer = (unsigned char*)UDPPacket::GetPayload();
		version &= 0x03;
		version <<= 6;
		buffer[0] &= ~0xc0;
		buffer[0] |= version;
	}

	void Reset();
};