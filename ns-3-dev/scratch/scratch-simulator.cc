#include "client.hpp"
#include "mapper.hpp"
#include "master.hpp"

#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WifiTopology");

#define MAPPERS_COUNT 3
#define MONITORE_TIME 1

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

  Simulator::Schedule (Seconds (MONITORE_TIME), &ThroughputMonitor, fmhelper, flowMon, em);
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

  Simulator::Schedule (Seconds (MONITORE_TIME), &AverageDelayMonitor, fmhelper, flowMon, em);
}


int
main (int argc, char *argv[])
{
  double error = 0.000001;
  string bandwidth = "1Mbps";
  bool verbose = true;
  double duration = 10;
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
