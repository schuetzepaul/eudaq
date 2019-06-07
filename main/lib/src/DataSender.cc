#include "eudaq/Event.hh"
#include "eudaq/AidaPacket.hh"
#include "eudaq/TransportClient.hh"
#include "eudaq/TransportFactory.hh"
#include "eudaq/Exception.hh"
#include "eudaq/BufferSerializer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/DataSender.hh"

namespace eudaq {

  DataSender::DataSender(const std::string &type, const std::string &name)
    : m_type(type), m_name(name), m_dataclient(0), m_UDPsockfd(0), m_UDPservaddr() {}

  void DataSender::Connect(const std::string &server) {
    delete m_dataclient;
    m_dataclient = TransportFactory::CreateClient(server);

    std::string packet;
    if (!m_dataclient->ReceivePacket(&packet, 1000000))
      EUDAQ_THROW("No response from DataCollector server");
    size_t i0 = 0, i1 = packet.find(' ');
    if (i1 == std::string::npos)
      EUDAQ_THROW("Invalid response from DataCollector server");
    std::string part(packet, i0, i1);
    if (part != "OK")
      EUDAQ_THROW("Invalid response from DataCollector server: " + packet);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    if (i1 == std::string::npos)
      EUDAQ_THROW("Invalid response from DataCollector server");
    part = std::string(packet, i0, i1 - i0);
    if (part != "EUDAQ")
      EUDAQ_THROW("Invalid response from DataCollector server, part=" + part);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    if (i1 == std::string::npos)
      EUDAQ_THROW("Invalid response from DataCollector server");
    part = std::string(packet, i0, i1 - i0);
    if (part != "DATA")
      EUDAQ_THROW("Invalid response from DataCollector server, part=" + part);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    part = std::string(packet, i0, i1 - i0);
    if (part != "DataCollector")
      EUDAQ_THROW("Invalid response from DataCollector server, part=" + part);
    if (part != "DataCollector")
      EUDAQ_THROW("Invalid response from DataCollector server, part=" + part);

    m_dataclient->SendPacket("OK EUDAQ DATA " + m_type + " " + m_name);
    packet = "";
    if (!m_dataclient->ReceivePacket(&packet, 1000000))
      EUDAQ_THROW("No response from DataCollector server");
    i1 = packet.find(' ');
    if (std::string(packet, 0, i1) != "OK")
      EUDAQ_THROW("Connection refused by DataCollector server: " + packet);
  }

  void DataSender::SendEvent(const Event &ev) {
    if (!m_dataclient)
      EUDAQ_THROW("Transport not connected error");
    // EUDAQ_DEBUG("Serializing event");
    BufferSerializer ser;
    ev.Serialize(ser);
    // EUDAQ_DEBUG("Sending event");
    m_dataclient->SendPacket(ser);
    // EUDAQ_DEBUG("Sent event");
  }

  void DataSender::SendPacket(const AidaPacket &packet) {
    //    EUDAQ_DEBUG("Serializing packet");
    BufferSerializer ser;
    packet.Serialize(ser);
    //    EUDAQ_DEBUG("Sending packet");
    m_dataclient->SendPacket(ser);
    //    EUDAQ_DEBUG("Sent packet");
  }


  // Connecting to a UDP socket (#BL4S)
  int DataSender::ConnectUDP(const std::string &ip_addr, const unsigned int port){

    // Creating socket file descriptor

    if(m_UDPsockfd != 0){
      close(m_UDPsockfd);
    }
    
    if ( (m_UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      EUDAQ_ERROR("Could not open a UDP socket.");
    }
  
    memset(&m_UDPservaddr, 0, sizeof(m_UDPservaddr)); 

    // Filling server information
    m_UDPservaddr.sin_family = AF_INET;
    m_UDPservaddr.sin_port = htons(port);
    m_UDPservaddr.sin_addr.s_addr = inet_addr(ip_addr.c_str());

    *(reinterpret_cast<int32_t*>(m_UDPbuffer)) = 0x0D15EA5E;
    EUDAQ_INFO("Successfully set up a UDP connection, sending to receiver: " + to_string(ip_addr.c_str()) + ":" + to_string(port));

    return m_UDPsockfd;
  }

  // Sending a data block via UDP (#BL4S)
  int DataSender::SendBlockUDP(const unsigned char *data, size_t len){

    if(len > MAX_UDP_SIZE - 2*(4))
    {
      return -1;
    }

    memcpy(m_UDPbuffer + 4, data, len);
    *(reinterpret_cast<int32_t*>(m_UDPbuffer + len + 4)) = 0xD15EA5E5;

    int sent_size = sendto(m_UDPsockfd, m_UDPbuffer, len + 2*4, 
			   MSG_CONFIRM, (const struct sockaddr *) &m_UDPservaddr,  
			   sizeof(m_UDPservaddr));
    
    return sent_size;
  }

  DataSender::~DataSender() { delete m_dataclient; }
}
