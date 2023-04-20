# CN_CHomeworks_2

## How to run?

using the terminal cd to ns-3-dev directory in CN_CHomeworks_1 and then:

```cpp
$ ./waf -- run "scratch-simulator"
```

## Project Description

This project sets up a wireless network topology using NS-3 simulator. The client send a random number from 0 to 25 to the master node using UDP; The master node sends the recieved data to 3 mappers using TCP; If the random number is supported by the mapper, the mapper send the matched character to the client using UDP.

## Definition of the Classes

To start the program, we define 4 classes:

- Master  
  Master class inherits from Application class and implements StartApplication and HandleRead methods. We store master's port, master's ip, array of mapperPorts and sockets, and master's socket.

```cpp
class Master : public Application
{
public:
    Master(uint16_t port, array<uint16_t, MAPPERS_COUNT> mappersPorts, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &mapperIp);
    virtual ~Master();

private:
    virtual void StartApplication(void);
    void HandleRead(Ptr<Socket> socket);

    uint16_t port;
    array<uint16_t, MAPPERS_COUNT> mappersPorts;
    Ipv4InterfaceContainer ip;
    Ipv4InterfaceContainer mapperIp;
    Ptr<Socket> socket;
    array<Ptr<Socket>, MAPPERS_COUNT> mapperSocket;
};
```

- Client  
   Master class also inherits from Application class and implements StartApplication and HandleRead methods. We store client's port, master's ip, and master'sport in this class.

```cpp
class Client : public Application
{
public:
    Client(uint16_t port, Ipv4InterfaceContainer &ip, uint16_t masterPort, Ipv4InterfaceContainer &masterIp);
    virtual ~Client();
    void HandleRead(Ptr<Socket> socket);

private:
    virtual void StartApplication(void);

    uint16_t port;
    Ptr<Socket> socket;
    Ipv4InterfaceContainer ip;
    uint16_t masterPort;
    Ipv4InterfaceContainer masterIp;
};
```

- Mapper
  Mapper class also inherits from Application class and implements StartApplication, HandleRead, and HandleAccept methods. We store mapper's port, mapper id, mapper's ip, socket, a UDP socket for sending data to client, and mappings of numbers to chars for each mapper in this class.

```cpp
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
  Ptr<Socket> sendToClientSocket;
  Ipv4InterfaceContainer ip;
  uint16_t id;
  std::map<uint16_t, char> mappings;
};
```

- MyHeader  
   This class inherits from Header class. We store clientPort and clientIp in order to store them in the header of the packets we send from client to master nad master to mappers, so that the mappers have client's ip and address to send packets to client. the size of the data is 8 bytes (uint16_t m_data(2bytes), uint16_t clientPort(2 bytes), and Ipv4Address clientIp(4 bytes).

```cpp
class MyHeader : public Header
{
public:
    MyHeader();
    virtual ~MyHeader();
    void SetData(uint16_t data);
    void SetClientPort(uint16_t port);
    void SetClientIp(Ipv4Address ip);
    uint16_t GetClientPort(void) const;
    Ipv4Address GetClientIp(void) const;
    uint16_t GetData(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual uint32_t GetSerializedSize(void) const;

private:
    uint16_t m_data;
    uint16_t clientPort;
    Ipv4Address clientIp;
};
```

---

Three kinds of NodeContainer objects are created for the wireless client, master and 3 mappers.

```cpp
NodeContainer wifiStaNodeClient;
    wifiStaNodeClient.Create(1);

    NodeContainer wifiStaNodeMaster;
    wifiStaNodeMaster.Create(1);

    NodeContainer wifiStaNodeMapper;
    wifiStaNodeMapper.Create(MAPPERS_COUNT);
```

The YansWifiChannelHelper object is used to create a default wireless channel, and the YansWifiPhyHelper object is used to set the channel for the physical layer. The WifiHelper object is created, and the remote station manager is set to ns3::AarfWifiManager. Also the WifiMacHelper object is created, and the MAC addresses for the wireless client, master, and mappers are set. The NetDeviceContainer objects are created for the wireless client and wireless master, and they are assigned to the staDeviceClient and staDeviceMaster variables. An error rate model is created and assigned to the physical layer.

```cpp
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDeviceClient;
    staDeviceClient = wifi.Install(phy, mac, wifiStaNodeClient);

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDeviceMaster;
    staDeviceMaster = wifi.Install(phy, mac, wifiStaNodeMaster);

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

    NetDeviceContainer staDeviceMapper;
    staDeviceMapper = wifi.Install(phy, mac, wifiStaNodeMapper);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(error));
    phy.SetErrorRateModel("ns3::YansErrorRateModel");
```

The mobility of the wireless client is set using a GridPositionAllocator and a RandomWalk2dMobilityModel. The mobility of the wireless master and mappers is set using a ConstantPositionMobilityModel. The InternetStackHelper object is created, and the Internet protocol is installed on the wireless client, master and mappers.

```cpp
MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(2.0),
                                  "DeltaY", DoubleValue(4.0),
                                  "GridWidth", UintegerValue(10),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodeClient);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiStaNodeMaster);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiStaNodeMapper);

    InternetStackHelper stack;
    stack.Install(wifiStaNodeClient);
    stack.Install(wifiStaNodeMaster);

    stack.Install(wifiStaNodeMapper);

```

The IP addresses for the wireless client, master, and mappers are assigned.
The routing tables are populated. Two Ptr objects are created for the client and master and three Ptr objects are created for the mappers. The applications are added to the appropriate nodes.

```cpp
Ipv4AddressHelper address;

    Ipv4InterfaceContainer staNodeClientInterface;
    Ipv4InterfaceContainer staNodesMasterInterface;

    Ipv4InterfaceContainer staNodesMapperInterface;

    address.SetBase("10.1.3.0", "255.255.255.0");
    staNodeClientInterface = address.Assign(staDeviceClient);
    staNodesMasterInterface = address.Assign(staDeviceMaster);

    staNodesMapperInterface = address.Assign(staDeviceMapper);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t port = 1102;
    uint16_t masterPort = 1102;
    Ptr<Client> clientApp = CreateObject<Client>(port, staNodeClientInterface, masterPort, staNodesMasterInterface);
    wifiStaNodeClient.Get(0)->AddApplication(clientApp);
    clientApp->SetStartTime(Seconds(0.0));
    clientApp->SetStopTime(Seconds(duration));

    array<uint16_t, MAPPERS_COUNT> mappersPorts = {8080, 8081, 8082};
    Ptr<Master> masterApp = CreateObject<Master>(port, mappersPorts, staNodesMasterInterface, staNodesMapperInterface);
    wifiStaNodeMaster.Get(0)->AddApplication(masterApp);
    masterApp->SetStartTime(Seconds(0.0));
    masterApp->SetStopTime(Seconds(duration));

    std::vector<Ptr<Mapper>> mapperApps;
    for (int i = 0; i < MAPPERS_COUNT; i++)
    {
        Ptr<Mapper> mapperApp = CreateObject<Mapper>(mappersPorts[i], staNodesMapperInterface, i);
        wifiStaNodeMapper.Get(i)->AddApplication(mapperApp);
        mapperApp->SetStartTime(Seconds(0.0));
        mapperApp->SetStopTime(Seconds(duration));
        mapperApps.push_back(mapperApp);
    }
```

The IP addresses for the wireless client and wireless master are assigned.The routing tables are populated. Two Ptr objects are created for the client and master applications. The applications are added to the appropriate nodes.

```cpp
    NS_LOG_INFO("Run Simulation");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    ThroughputMonitor(&flowHelper, flowMonitor, error);
    AverageDelayMonitor(&flowHelper, flowMonitor, error);

    Simulator::Stop(Seconds(duration));
    Simulator::Run();
```

## Client Methods:

- Start Application </br>

In the StartApplication, We first create a UDP socket and also create a InetSocketAddress using master's port and ip. Then we connect the socket to the address and send data using GenerateTeraffic function. The first data we send should always be 0. </br>
The second part of the client's StartApplication method is for recieving the mappeers packets. In this part, we create a UDP socket and bind it to the clien's port and ip address. Then call the HandleRead function every time the client recieves a packet.

```cpp
void Client::StartApplication(void)
{
    Ptr<Socket> sock = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress sockAddr(masterIp.GetAddress(0), masterPort);
    sock->Connect(sockAddr);
    GenerateTraffic(sock, 0, port, ip.GetAddress(0));

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(ip.GetAddress(0), port);
    socket->Bind(local);
    socket->SetRecvCallback(MakeCallback(&Client::HandleRead, this));
}
```

- GenerateTeraffic </br>
  In this part we first create a packet and also a header that includes random number, client's ip, and port. Then add the header to the packet and send the packet using UDP to the master. We then schedule to call this function every 0.1 seconds to send data.

```cpp
static void GenerateTraffic(Ptr<Socket> socket, uint16_t data, uint16_t port, Ipv4Address ip)
{
    Ptr<Packet> packet = new Packet();
    MyHeader m;
    m.SetData(data);
    m.SetClientIp(ip);
    m.SetClientPort(port);
    packet->AddHeader(m);
    socket->Send(packet);

    if (useTimeOut)
        timeOut = Simulator::Schedule(Seconds(1), &CheckTimeOut, socket, data, port, ip);
    else
        Simulator::Schedule(Seconds(0.1), &GenerateTraffic, socket, rand() % 26, port, ip);
}
```

- HandleRead </br>
  We first create a packet and then assign it the recieved data; While we have data to recieve, we create a header and initial it with the RemoveHeader of th erecieved header and then print it.

```cpp
void Client::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    while ((packet = socket->Recv()))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }
        MyHeader destinationHeader;
        packet->RemoveHeader(destinationHeader);
        cout << "Client Recieved: " << static_cast<char>(destinationHeader.GetData()) << endl; /////////////////////////////////////////////////////////////
    }
    Simulator::Cancel(timeOut);
    GenerateTraffic(socket, rand() % 26, port, ip.GetAddress(0));
}
```

## Master Methods:

- StartApplication </br>
  In the first part we create a UDP socket for recieving packets from the client and bind it to the address of master port and ip and call the HandleRead function every time the master recieves a packet. </br>
  In the second part, for every mapper we create a TCP socket and connect the socket to the related mapper's port and ip.

```cpp
void Master::StartApplication(void)
{
    socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(ip.GetAddress(0), port);
    socket->Bind(local);

    for (int i = 0; i < MAPPERS_COUNT; i++)
    {
        mapperSockets[i] = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        InetSocketAddress mapperAddr = InetSocketAddress(mapperIp.GetAddress(i), mappersPorts[i]);
        mapperSockets[i]->Connect(mapperAddr);
    }

    socket->SetRecvCallback(MakeCallback(&Master::HandleRead, this));
}
```

- HandleRead <br>
  in This part we recieve the packet from the client and send it to all mappers.

```cpp
void Master::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;

    while ((packet = socket->Recv()))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader(destinationHeader);
        Ptr<Packet> sendPacket = new Packet();
        sendPacket->AddHeader(destinationHeader);
        for (int i = 0; i < MAPPERS_COUNT; i++)
        {
            mapperSockets[i]->Send(sendPacket);
        }
    }
}
```

## Mapper Methods:

- StartApplication </br>
  In this part we create a TCP socket and bind it to the mapper's port and ip and then listen on that socket. Then we call HandleAccept function every time the mapper needs to accept a socket.

```cpp
void
Mapper::StartApplication (void)
{
  sendToClientSocket  = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());//////////
  socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
  InetSocketAddress localAddress = InetSocketAddress (ip.GetAddress (id), port);
  socket->Bind (localAddress);
  socket->Listen ();
  socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                             MakeCallback (&Mapper::HandleAccept, this));
}
```

- HandleAccept </br>
  we call the handleRead funcrion every time the mapper accepts to recieve a packet.

```cpp
void Mapper::HandleAccept(Ptr<Socket> socket, const Address &src)
{
    socket->SetRecvCallback(MakeCallback(&Mapper::HandleRead, this));
}
```

- HandleRead </br>
  We create a pack and initial it by the Recv return value. Then try to find the recieved data in mapper's mapping data; If it finds it, We use UDP socket and connect it to the client's port and ip address and then send the packet to client. At last we close the socket.

```cpp
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

            Ipv4Address cAdr = destinationHeader.GetClientIp ();
            uint16_t cPort = destinationHeader.GetClientPort ();
            InetSocketAddress destAddress = InetSocketAddress (cAdr, cPort);
            sendToClientSocket->SendTo(newPacket, 0, destAddress);
            std::cout << "Mapper " << id << " Sent: " << static_cast<char> (destNewHeader.GetData ())
                << std::endl;
        }
    }
}
```
