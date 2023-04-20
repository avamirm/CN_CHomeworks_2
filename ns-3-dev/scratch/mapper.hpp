#ifndef MAPPER_HPP
#define MAPPER_HPP

#include "myHeader.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
class Mapper : public Application
{
public:
  Mapper (uint16_t port, Ipv4InterfaceContainer &ip, uint16_t id);
  virtual ~Mapper ();
  void HandleRead (Ptr<Socket> socket);
  void HandleAccept (Ptr<Socket> socket, const Address &src);

private:
  virtual void StartApplication (void);

  uint16_t port;
  Ptr<Socket> socket;
  Ipv4InterfaceContainer ip;
  uint16_t id;
  std::map<uint16_t, char> mappings;
};

Mapper::Mapper (uint16_t port, Ipv4InterfaceContainer &ip, uint16_t id)
    : port (port), ip (ip), id (id)
{
  for (int i = 9 * id; i < 9 * (id + 1); i++)
    mappings[i] = 'a' + i;
}

Mapper::~Mapper ()
{
}

void
Mapper::HandleAccept (Ptr<Socket> socket, const Address &src)
{
  socket->SetRecvCallback (MakeCallback (&Mapper::HandleRead, this));
}

void
Mapper::StartApplication (void)
{
  socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
  InetSocketAddress localAddress = InetSocketAddress (ip.GetAddress (id), port);
  socket->Bind (localAddress);
  socket->Listen ();
  socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                             MakeCallback (&Mapper::HandleAccept, this));
}

void
Mapper::HandleRead (Ptr<Socket> socket)
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
      uint16_t data = destinationHeader.GetData ();
      auto it = mappings.find (data);
      if (it != mappings.end ())
        {

          MyHeader destNewHeader;
          destNewHeader.SetData (it->second);
          Ptr<Packet> newPacket = Create<Packet> ();
          newPacket->AddHeader (destNewHeader);
          Ptr<Socket> newSocket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
          uint16_t cPort = destinationHeader.GetClientPort ();
          Ipv4Address cAdr = destinationHeader.GetClientIp ();
          InetSocketAddress destAddress = InetSocketAddress (cAdr, cPort);
          newSocket->Connect (destAddress);
          newSocket->Send (newPacket);
          std::cout << "Mapper " << id << " Sent: " << static_cast<char> (destNewHeader.GetData ())
               << std::endl;
          newSocket->Close ();
        }
    }
}

#endif
