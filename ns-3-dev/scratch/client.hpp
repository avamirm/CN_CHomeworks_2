#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ctime>

#include "myHeader.hpp"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#define TIME_SPACE  0.01

EventId timeOut;
bool useTimeOut = false;

using namespace ns3;
class Client : public Application
{
public:
  Client (uint16_t port, Ipv4InterfaceContainer &ip, uint16_t masterPort,
          Ipv4InterfaceContainer &masterIp);
  virtual ~Client ();
  void HandleRead (Ptr<Socket> socket);

private:
  virtual void StartApplication (void);

  uint16_t port;
  Ptr<Socket> socket;
  Ipv4InterfaceContainer ip;
  uint16_t masterPort;
  Ipv4InterfaceContainer masterIp;
};

Client::Client (uint16_t port, Ipv4InterfaceContainer &ip, uint16_t masterPort,
                Ipv4InterfaceContainer &masterIp)
    : port (port), ip (ip), masterPort (masterPort), masterIp (masterIp)
{
  std::srand (time (0));
}

Client::~Client ()
{
}
inline static void GenerateTraffic (Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip);
inline void
CheckTimeOut (Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip)
{
  GenerateTraffic (socket, rand () % 26, port, ip);
}
inline static void
GenerateTraffic (Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip)
{
  Ptr<Packet> packet = new Packet ();
  MyHeader m;
  m.SetData (data);
  m.SetClientIp (ip);
  m.SetClientPort (port);
  packet->AddHeader (m);
  socket->Send (packet);

  if (useTimeOut)
    timeOut = Simulator::Schedule (Seconds (TIME_SPACE), &CheckTimeOut, socket, data, port, ip);
  else
    Simulator::Schedule (Seconds (TIME_SPACE), &GenerateTraffic, socket, rand () % 26, port, ip);
}

void
Client::StartApplication (void)
{
  Ptr<Socket> sock = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  InetSocketAddress sockAddr (masterIp.GetAddress (0), masterPort);
  sock->Connect (sockAddr);
  GenerateTraffic (sock, 0, port, ip.GetAddress (0));

  Ptr<Socket> socket;
  socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  InetSocketAddress local = InetSocketAddress (ip.GetAddress (0), port);
  socket->Bind (local);
  socket->SetRecvCallback (MakeCallback (&Client::HandleRead, this));
}

void
Client::HandleRead (Ptr<Socket> socket)
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
      std::cout << "Client Recieved: " << static_cast<char> (destinationHeader.GetData ())
           << std::endl; 
    }
  Simulator::Cancel (timeOut);
  GenerateTraffic (socket, rand () % 26, port, ip.GetAddress (0));
}

#endif