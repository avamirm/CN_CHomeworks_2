#ifndef MASTER_HPP
#define MASTER_HPP

#include <ctime>
#include <array>

#include "myHeader.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
#define MAPPERS_COUNT 3


class Master : public Application
{
public:
  Master (uint16_t port, std::array<uint16_t, MAPPERS_COUNT> mappersPorts, Ipv4InterfaceContainer &ip,
          Ipv4InterfaceContainer &mapperIp);
  virtual ~Master ();

private:
  virtual void StartApplication (void);
  void HandleRead (Ptr<Socket> socket);

  uint16_t port;
  std::array<uint16_t, MAPPERS_COUNT> mappersPorts;
  Ipv4InterfaceContainer ip;
  Ipv4InterfaceContainer mapperIp;
  Ptr<Socket> socket;
  std::array<Ptr<Socket>, MAPPERS_COUNT> mapperSockets;
};

Master::Master (uint16_t port, std::array<uint16_t, MAPPERS_COUNT> mappersPorts,
                Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &mapperIp)
    : port (port), mappersPorts (mappersPorts), ip (ip), mapperIp (mapperIp)
{
  std::srand (time (0));
}

Master::~Master ()
{
}

void
Master::StartApplication (void)
{
  socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  InetSocketAddress local = InetSocketAddress (ip.GetAddress (0), port);
  socket->Bind (local);

  for (int i = 0; i < MAPPERS_COUNT; i++)
    {
      mapperSockets[i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
      InetSocketAddress mapperAddr = InetSocketAddress (mapperIp.GetAddress (i), mappersPorts[i]);
      mapperSockets[i]->Connect (mapperAddr);
    }

  socket->SetRecvCallback (MakeCallback (&Master::HandleRead, this));
}

void
Master::HandleRead (Ptr<Socket> socket)
{
  Ptr<Packet> packet;

  while ((packet = socket->Recv ()))
    {
      if (packet->GetSize () == 0)
        {
          break;
        }

      MyHeader destinationHeader;
      packet->RemoveHeader (destinationHeader);
      Ptr<Packet> sendPacket = new Packet ();
      sendPacket->AddHeader (destinationHeader);
      for (int i = 0; i < MAPPERS_COUNT; i++)
        {
          mapperSockets[i]->Send (sendPacket);
        }
    }
}

#endif