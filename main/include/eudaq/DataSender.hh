#ifndef EUDAQ_INCLUDED_DataSender
#define EUDAQ_INCLUDED_DataSender

#include "eudaq/Platform.hh"
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_UDP_SIZE 65507

namespace eudaq {

  class TransportClient;
  class Event;
  class AidaPacket;

  class DLLEXPORT DataSender {
  public:
    DataSender(const std::string &type, const std::string &name);
    ~DataSender();
    void Connect(const std::string &server);
    void SendEvent(const Event &);
    void SendPacket(const AidaPacket &);

    // Functions for sending data blocks via UDP (#BL4S)
    int ConnectUDP(const std::string &ip_addr, const unsigned int port);
    int SendBlockUDP(const unsigned char *data, size_t len);

  private:
    std::string m_type, m_name;
    TransportClient *m_dataclient;

    // Variables for the UDP connection (#BL4S)
    int m_UDPsockfd;
    struct sockaddr_in m_UDPservaddr;
    char m_UDPbuffer[MAX_UDP_SIZE];
  };
}

#endif // EUDAQ_INCLUDED_DataSender
