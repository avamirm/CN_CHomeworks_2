#ifndef MYHEADER_HPP
#define MYHEADER_HPP

#include "ns3/udp-header.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;
class MyHeader : public Header
{
public:
  MyHeader ();
  virtual ~MyHeader ();
  void SetData (uint16_t data);
  void SetClientPort (uint16_t port);
  void SetClientIp (Ipv4Address ip);
  uint16_t GetClientPort (void) const;
  Ipv4Address GetClientIp (void) const;
  uint16_t GetData (void) const;
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;

private:
  uint16_t m_data;
  uint16_t clientPort;
  Ipv4Address clientIp;
};

MyHeader::MyHeader ()
{
}

MyHeader::~MyHeader ()
{
}

TypeId
MyHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyHeader").SetParent<Header> ().AddConstructor<MyHeader> ();
  return tid;
}

TypeId
MyHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
MyHeader::Print (std::ostream &os) const
{
  os << "data = " << m_data << std::endl;
}

uint32_t
MyHeader::GetSerializedSize (void) const
{
  return 8;
}

void
MyHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_data);
  start.WriteHtonU32 (clientIp.Get ());
  start.WriteHtonU16 (clientPort);
}

uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
  m_data = start.ReadNtohU16 ();
  clientIp = Ipv4Address (start.ReadNtohU32 ());
  clientPort = start.ReadNtohU16 ();

  return 8;
}

void
MyHeader::SetData (uint16_t data)
{
  m_data = data;
}

void
MyHeader::SetClientPort (uint16_t port)
{
  clientPort = port;
}

void
MyHeader::SetClientIp (Ipv4Address ip)
{
  clientIp = ip;
}

uint16_t
MyHeader::GetClientPort (void) const
{
  return clientPort;
}

Ipv4Address
MyHeader::GetClientIp (void) const
{
  return clientIp;
}

uint16_t
MyHeader::GetData (void) const
{
  return m_data;
}

#endif