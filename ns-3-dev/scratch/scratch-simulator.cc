#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <array>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/error-model.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("WifiTopology");

#define MAPPERS_COUNT 3

EventId timeOut;
bool useTimeOut = false;

void
ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
  uint16_t i = 0;

  std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();

  Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin ();
       stats != flowStats.end (); ++stats)
    {
      Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);

      std::cout << "Flow ID			: " << stats->first << " ; "
                << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress
                << std::endl;
      std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
      std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
      std::cout << "Duration		: "
                << (stats->second.timeLastRxPacket.GetSeconds () -
                    stats->second.timeFirstTxPacket.GetSeconds ())
                << std::endl;
      std::cout << "Last Received Packet	: " << stats->second.timeLastRxPacket.GetSeconds ()
                << " Seconds" << std::endl;
      std::cout << "Throughput: "
                << stats->second.rxBytes * 8.0 /
                       (stats->second.timeLastRxPacket.GetSeconds () -
                        stats->second.timeFirstTxPacket.GetSeconds ()) /
                       1024 / 1024
                << " Mbps" << std::endl;

      i++;

      std::cout << "---------------------------------------------------------------------------"
                << std::endl;
    }

  Simulator::Schedule (Seconds (10), &ThroughputMonitor, fmhelper, flowMon, em);
}

void
AverageDelayMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
  uint16_t i = 0;

  std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();
  Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin ();
       stats != flowStats.end (); ++stats)
    {
      Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
      std::cout << "Flow ID			: " << stats->first << " ; "
                << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress
                << std::endl;
      std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
      std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
      std::cout << "Duration		: "
                << (stats->second.timeLastRxPacket.GetSeconds () -
                    stats->second.timeFirstTxPacket.GetSeconds ())
                << std::endl;
      std::cout << "Last Received Packet	: " << stats->second.timeLastRxPacket.GetSeconds ()
                << " Seconds" << std::endl;
      std::cout << "Sum of e2e Delay: " << stats->second.delaySum.GetSeconds () << " s"
                << std::endl;
      std::cout << "Average of e2e Delay: "
                << stats->second.delaySum.GetSeconds () / stats->second.rxPackets << " s"
                << std::endl;

      i++;

      std::cout << "---------------------------------------------------------------------------"
                << std::endl;
    }

  Simulator::Schedule (Seconds (10), &AverageDelayMonitor, fmhelper, flowMon, em);
}

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
  os << "data = " << m_data << endl;
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

class Master : public Application
{
public:
  Master (uint16_t port, array<uint16_t, MAPPERS_COUNT> mappersPorts, Ipv4InterfaceContainer &ip,
          Ipv4InterfaceContainer &mapperIp);
  virtual ~Master ();

private:
  virtual void StartApplication (void);
  void HandleRead (Ptr<Socket> socket);

  uint16_t port;
  array<uint16_t, MAPPERS_COUNT> mappersPorts;
  Ipv4InterfaceContainer ip;
  Ipv4InterfaceContainer mapperIp;
  Ptr<Socket> socket;
  array<Ptr<Socket>, MAPPERS_COUNT> mapperSockets;
};

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
          cout << "Mapper " << id << " Sent: " << static_cast<char> (destNewHeader.GetData ())
               << endl;
          newSocket->Close ();
        }
    }
}

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
static void GenerateTraffic (Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip);
void
CheckTimeOut (Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip)
{
  GenerateTraffic (socket, rand () % 26, port, ip);
}
static void
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
    timeOut = Simulator::Schedule (Seconds (0.1), &CheckTimeOut, socket, data, port, ip);
  else
    Simulator::Schedule (Seconds (0.1), &GenerateTraffic, socket, rand () % 26, port, ip);
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
      cout << "Client Recieved: " << static_cast<char> (destinationHeader.GetData ())
           << endl; /////////////////////////////////////////////////////////////
    }
  Simulator::Cancel (timeOut);
  GenerateTraffic (socket, rand () % 26, port, ip.GetAddress (0));
}

Master::Master (uint16_t port, array<uint16_t, MAPPERS_COUNT> mappersPorts,
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

int
main (int argc, char *argv[])
{
  double error = 0.000001;
  string bandwidth = "1Mbps";
  bool verbose = true;
  double duration = 60.0;
  bool tracing = false;

  srand (time (NULL));

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer wifiStaNodeClient;
  wifiStaNodeClient.Create (1);

  NodeContainer wifiStaNodeMaster;
  wifiStaNodeMaster.Create (1);

  NodeContainer wifiStaNodeMapper;
  wifiStaNodeMapper.Create (MAPPERS_COUNT);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();

  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDeviceClient;
  staDeviceClient = wifi.Install (phy, mac, wifiStaNodeClient);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer staDeviceMaster;
  staDeviceMaster = wifi.Install (phy, mac, wifiStaNodeMaster);

  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  NetDeviceContainer staDeviceMapper;
  staDeviceMapper = wifi.Install (phy, mac, wifiStaNodeMapper);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (error));
  phy.SetErrorRateModel ("ns3::YansErrorRateModel");

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY",
                                 DoubleValue (0.0), "DeltaX", DoubleValue (2.0), "DeltaY",
                                 DoubleValue (4.0), "GridWidth", UintegerValue (10), "LayoutType",
                                 StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds",
                             RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodeClient);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodeMaster);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodeMapper);

  InternetStackHelper stack;
  stack.Install (wifiStaNodeClient);
  stack.Install (wifiStaNodeMaster);

  stack.Install (wifiStaNodeMapper);

  Ipv4AddressHelper address;

  Ipv4InterfaceContainer staNodeClientInterface;
  Ipv4InterfaceContainer staNodesMasterInterface;

  Ipv4InterfaceContainer staNodesMapperInterface;

  address.SetBase ("10.1.3.0", "255.255.255.0");
  staNodeClientInterface = address.Assign (staDeviceClient);
  staNodesMasterInterface = address.Assign (staDeviceMaster);

  staNodesMapperInterface = address.Assign (staDeviceMapper);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 1102;
  uint16_t masterPort = 1102;
  Ptr<Client> clientApp =
      CreateObject<Client> (port, staNodeClientInterface, masterPort, staNodesMasterInterface);
  wifiStaNodeClient.Get (0)->AddApplication (clientApp);
  clientApp->SetStartTime (Seconds (0.0));
  clientApp->SetStopTime (Seconds (duration));

  array<uint16_t, MAPPERS_COUNT> mappersPorts = {8080, 8081, 8082};
  Ptr<Master> masterApp =
      CreateObject<Master> (port, mappersPorts, staNodesMasterInterface, staNodesMapperInterface);
  wifiStaNodeMaster.Get (0)->AddApplication (masterApp);
  masterApp->SetStartTime (Seconds (0.0));
  masterApp->SetStopTime (Seconds (duration));

  std::vector<Ptr<Mapper>> mapperApps;
  for (int i = 0; i < MAPPERS_COUNT; i++)
    {
      Ptr<Mapper> mapperApp = CreateObject<Mapper> (mappersPorts[i], staNodesMapperInterface, i);
      wifiStaNodeMapper.Get (i)->AddApplication (mapperApp);
      mapperApp->SetStartTime (Seconds (0.0));
      mapperApp->SetStopTime (Seconds (duration));
      mapperApps.push_back (mapperApp);
    }

  NS_LOG_INFO ("Run Simulation");

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll ();

  ThroughputMonitor (&flowHelper, flowMonitor, error);
  AverageDelayMonitor (&flowHelper, flowMonitor, error);

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();

  return 0;
}
