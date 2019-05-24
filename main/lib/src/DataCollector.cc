#include "eudaq/DataCollector.hh"
#include "eudaq/TransportFactory.hh"
#include "eudaq/BufferSerializer.hh"
#include "eudaq/DetectorEvent.hh"
#include "eudaq/Logger.hh"
#include "eudaq/Utils.hh"
#include <iostream>
#include <ostream>

namespace eudaq {

  namespace {

    void *DataCollector_thread(void *arg) {
      DataCollector *dc = static_cast<DataCollector *>(arg);
      dc->DataThread();
      return 0;
    }

  } // anonymous namespace

  DataCollector::DataCollector(const std::string &name,
                               const std::string &runcontrol,
                               const std::string &listenaddress,
                               const std::string &runnumberfile)
      : CommandReceiver("DataCollector", name, runcontrol, false),
        m_runnumberfile(runnumberfile), m_done(false), m_listening(true),
        m_dataserver(TransportFactory::CreateServer(listenaddress)), m_thread(),
        m_itlu((size_t)-1), m_slow(0),
        m_runnumber(ReadFromFile(runnumberfile, 0U)), m_eventnumber(0),
        m_runstart(0) {
    m_dataserver->SetCallback(
        TransportCallback(this, &DataCollector::DataHandler));
    EUDAQ_DEBUG("Instantiated datacollector with name: " + name);
    m_thread = std::unique_ptr<std::thread>(
        new std::thread(DataCollector_thread, this));
    EUDAQ_DEBUG("Listen address=" +
                to_string(m_dataserver->ConnectionString()));
    CommandReceiver::StartThread();
  }

  DataCollector::~DataCollector() {
    m_done = true;
    m_thread->join();
    delete m_dataserver;
  }

  void DataCollector::OnServer() {
    m_connectionstate.SetTag("_SERVER", m_dataserver->ConnectionString());
  }

  void DataCollector::OnGetRun() {
    // std::cout << "Sending run number: " << m_runnumber << std::endl;
    m_connectionstate.SetTag("_RUN", to_string(m_runnumber));
  }

  void DataCollector::OnConnect(const ConnectionInfo &id) {
    EUDAQ_INFO("Connection from " + to_string(id));
    Info info;
    info.id = std::shared_ptr<ConnectionInfo>(id.Clone());
    m_buffer.push_back(info);
    if (id.GetType() == "Producer" && id.GetName() == "TLU") {
      m_itlu = m_buffer.size() - 1;
    }
    // One should count the number of SlowProducers to know how many Producers
    // have to be synchronised.
    if (id.GetType() == "SlowProducer") {
      m_slow++;
    }
  }

  void DataCollector::OnDisconnect(const ConnectionInfo &id) {
    EUDAQ_INFO("Disconnected: " + to_string(id));
    size_t i = GetInfo(id);
    if (i == m_itlu) {
      m_itlu = (size_t)-1;
    } else if (i < m_itlu) {
      --m_itlu;
    }
    // Substract disconnected slow producer from the number of SlowProducers.
    if (id.GetType() == "SlowProducer") {
      m_slow--;
    }
    m_buffer.erase(m_buffer.begin() + i);
  }

  void DataCollector::OnInitialise(const Configuration &init) {
    m_init = init;
    SetConnectionState(eudaq::ConnectionState::STATE_UNCONF, "Initialised (" + init.Name() + ")");
  }
  
/* Upon receiving the Configure command the DataCollector function uses
 * Configure object to obtain configuration setting from the .conf file. The
 * important settings that it aquires are the file Type and Pattern that the
 * DataCollector will write. These settings are stored in the varible m_writer
 * which is an instance of the standard FileWriter Class.
*/
  void DataCollector::OnConfigure(const Configuration &param) {
    m_config = param;
    m_writer = std::shared_ptr<eudaq::FileWriter>(
        FileWriterFactory::Create(m_config.Get("FileType", "")));
    m_writer->SetFilePattern(m_config.Get("FilePattern", ""));

    // Also initialize a UDP connection
    char buffer[1024];
  
    // Creating socket file descriptor
    close(sockfd);
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        EUDAQ_THROW("UDP socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    
    memset(&servaddr, 0, sizeof(servaddr)); 

    const char* clientIP = m_config.Get("ClientIP", "127.0.0.1").c_str();
    int clientPort = m_config.Get("ClientPort", 8080);
    
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(clientPort);
    servaddr.sin_addr.s_addr = inet_addr(clientIP); 

    EUDAQ_INFO("UDP socket was set up");
    std::cout << "UDP socket was set up" << std::endl;
    
  }

  void DataCollector::OnPrepareRun(unsigned runnumber) {
    // if (runnumber == m_runnumber && m_ser.get()) return false;
    EUDAQ_INFO("Preparing for run " + to_string(runnumber));
    m_runstart = Time::Current();
    try {
      if (!m_writer) {
        EUDAQ_THROW("You must configure before starting a run");
      }
      m_writer->StartRun(runnumber);
      WriteToFile(m_runnumberfile, runnumber);
      m_runnumber = runnumber;
      m_eventnumber = 0;

      for (size_t i = 0; i < m_buffer.size(); ++i) {
        if (m_buffer[i].events.size() > 0) {
          EUDAQ_WARN("Buffer " + to_string(*m_buffer[i].id) + " has " +
                     to_string(m_buffer[i].events.size()) +
                     " events remaining.");
          m_buffer[i].events.clear();
        }
      }
      // needs to be cleared - Scenario: 2 Producers.  Producer A had remaining data, which is deleted above.
      // Now producer B sends data - event *seemingly* complete - OnCompleteEvent called, which accesses
      // nonexistent Producer A data - segfault.
      m_ireceived.clear();

      SetConnectionState(ConnectionState::STATE_RUNNING);
    } catch (const Exception &e) {
      std::string msg =
          "Error preparing for run " + to_string(runnumber) + ": " + e.what();
      EUDAQ_ERROR(msg);
      SetConnectionState(ConnectionState::STATE_ERROR);
    }
  }

  void DataCollector::OnStopRun() {
    std::cout<<"Stop Run for datacollector called \n";
    EUDAQ_INFO("Start stopping run" + to_string(m_runnumber));
    eudaq::mSleep(6000); // lets give the other prducers time to sent the last data packets. Mimosa (NI) takes 5 Seconds - luetticke
    auto fastwaiting = std::count_if(m_ireceived.begin(), m_ireceived.end(), [](std::pair<size_t, std::string> i) -> bool {return i.second != "SlowProducer";});
    if (fastwaiting == m_buffer.size() - m_slow){
        // Even late events are welcome. - luetticke
        EUDAQ_WARN("Building unfinished events while stopping" + to_string(m_runnumber));
        OnCompleteEvent();
    }
    EUDAQ_INFO("End of run " + to_string(m_runnumber));

    if(m_connectionstate.GetState()!= ConnectionState::STATE_ERROR)
      SetConnectionState(ConnectionState::STATE_CONF);
    // Leave the file open, more events could still arrive
    // m_ser = counted_ptr<FileSerializer>();
  }

  void DataCollector::OnReceive(const ConnectionInfo &id,
                                std::shared_ptr<Event> ev) {
    Info &inf = m_buffer[GetInfo(id)];

    // Prevent the accumulation of old information from slow producer.
    if(id.GetType() == "SlowProducer")
      inf.events.clear();
    inf.events.push_back(ev);

    // Print if the received event is the EORE of this producer:
    if (inf.events.back()->IsEORE()){
         EUDAQ_INFO("Received EORE Event from " + id.GetName());
         std::cout << "Received EORE Event from " << id << ": " << *ev
                << std::endl;
    }


    // There are two types of producers now: "fast" producer and slow producer.
    // Before starting OnComplete function we need to receive events from all
    // fast producers, but we don't have to wait for all slow producers.
    // So we count only the number of fast producers events. And if it exceeds
    // the number of fast producers we can go to the OnComplete function. The
    // id of producers which sent an event are stored in m_ireceived array
    // in order to know which slow producers sent an event.

    m_ireceived[GetInfo(id)] = id.GetType();

    int fastwaiting = std::count_if(m_ireceived.begin(), m_ireceived.end(), [](std::pair<size_t, std::string> i) -> bool {return i.second != "SlowProducer";});

    if (fastwaiting == m_buffer.size() - m_slow)
      OnCompleteEvent();
  }

  void DataCollector::OnStatus() {
    std::string evt;
    if (m_eventnumber > 0)
      evt = to_string(m_eventnumber - 1);
    m_connectionstate.SetTag("EVENT", evt);
    m_connectionstate.SetTag("RUN", to_string(m_runnumber));
    if (m_writer.get())
      m_connectionstate.SetTag("FILEBYTES", to_string(m_writer->FileBytes()));
  }

  void DataCollector::OnCompleteEvent() {
    bool more = true;
    bool found_bore = false;
    while (more) {
      if (m_eventnumber < 10 || m_eventnumber % 1000 == 0) {
        std::cout << "Complete Event: " << m_runnumber << "." << m_eventnumber
                  << std::endl;
      }
      unsigned n_run = m_runnumber, n_ev = m_eventnumber;
      uint64_t n_ts = (uint64_t)-1;
      if (m_itlu != (size_t)-1) {
        TLUEvent *ev =
            static_cast<TLUEvent *>(m_buffer[m_itlu].events.front().get());
        n_run = ev->GetRunNumber();
        n_ev = ev->GetEventNumber();
        n_ts = ev->GetTimestamp();
      }
      DetectorEvent ev(n_run, n_ev, n_ts);
      unsigned tluev = 0;
      for (size_t i = 0; i < m_buffer.size(); ++i) {
        // Checking if the producer has sent the data to read.
        //if (std::find(m_ireceived.begin(), m_ireceived.end(), i) == m_ireceived.end())
        //  continue;
        if (m_ireceived.count(i) == 0)
          continue;
        if (m_buffer[i].events.front()->GetRunNumber() != m_runnumber) {
          EUDAQ_ERROR("Run number mismatch in event " +
                      to_string(ev.GetEventNumber()));
        }
        if (m_ireceived[i] != "SlowProducer") {
          if ((m_buffer[i].events.front()->GetEventNumber() != m_eventnumber) &&
              (m_buffer[i].events.front()->GetEventNumber() !=
               m_eventnumber - 1)) {
            if (ev.GetEventNumber() % 1000 == 0) {
              // dhaas: added if-statement to filter out TLU event number 0, in
              // case of bad clocking out
              if (m_buffer[i].events.front()->GetEventNumber() != 0)
                EUDAQ_WARN(
                    "Event number mismatch > 1 in event " +
                    to_string(ev.GetEventNumber()) + " " +
                    to_string(m_buffer[i].events.front()->GetEventNumber()) +
                    " " + to_string(m_eventnumber));
              if (m_buffer[i].events.front()->GetEventNumber() == 0)
                EUDAQ_WARN("Event number mismatch > 1 in event " +
                           to_string(ev.GetEventNumber()));
            }
          }
        }
        ev.AddEvent(m_buffer[i].events.front());
        m_buffer[i].events.pop_front();
        if (m_buffer[i].events.size() == 0) {
          // I am unsure if we want more = false here. What if the current producer is a slow producer? - luetticke
          more = false;
          // keep track here about if data from producer x is available.  - luetticke
          m_ireceived.erase(i);
        }
      }

      if (ev.IsBORE()) {
        ev.SetTag("STARTTIME", m_runstart.Formatted());
        ev.SetTag("INIT", to_string(m_init));
        ev.SetTag("CONFIG", to_string(m_config));
        found_bore = true;
      }
      if (ev.IsEORE()) {
        ev.SetTag("STOPTIME", Time::Current().Formatted());
        EUDAQ_INFO("Run " + to_string(ev.GetRunNumber()) + ", EORE = " +
                   to_string(ev.GetEventNumber()));
      }
      if (m_writer.get()) {
        try {
          m_writer->WriteEvent(ev);
        } catch (const Exception &e) {
          std::string msg = "Exception writing to file: ";
          msg += e.what();
          EUDAQ_ERROR(msg);
          SetConnectionState(ConnectionState::STATE_ERROR);
        }
      } else {
        EUDAQ_ERROR("Event received before start of run");
      }

      // Now send a UDP package for every event.
      int n; 
      socklen_t len;
      
      sendto(sockfd, &ev, sizeof(ev), 
	     MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
	     sizeof(servaddr)); 
      EUDAQ_DEBUG("Event sent via UDP"); 

      // Only increase the internal event counter for non-BORE events.
      // This is required since all producers start sending data with event ID 0
      // but the data collector would already be at 1, since BORE was 0.
      if (!found_bore)
        ++m_eventnumber;
    }
    // Don't delete all. What if events from everywhere but one place are still there
    // and then and then the data from the last producer arrives?
    // This is the reason why there sometimes are remaining events from
    // all modules in the buffer. Better: Keep track upstream.  - luetticke
    //m_ireceived.clear();
  }

  size_t DataCollector::GetInfo(const ConnectionInfo &id) {
    for (size_t i = 0; i < m_buffer.size(); ++i) {
      // std::cout << "Checking " << *m_buffer[i].id << " == " << id<<
      // std::endl;
      if (m_buffer[i].id->Matches(id))
        return i;
    }
    EUDAQ_THROW("Unrecognised connection id");
  }

  void DataCollector::DataHandler(TransportEvent &ev) {
    // std::cout << "Event: ";
    switch (ev.etype) {
    case (TransportEvent::CONNECT):
      // std::cout << "Connect:    " << ev.id << std::endl;
      if (m_listening) {
        m_dataserver->SendPacket("OK EUDAQ DATA DataCollector", ev.id, true);
      } else {
        m_dataserver->SendPacket(
            "ERROR EUDAQ DATA Not accepting new connections", ev.id, true);
        m_dataserver->Close(ev.id);
      }
      break;
    case (TransportEvent::DISCONNECT):
      // std::cout << "Disconnect: " << ev.id << std::endl;
      OnDisconnect(ev.id);
      break;
    case (TransportEvent::RECEIVE):
      if (ev.id.GetState() == 0) { // waiting for identification
        // check packet
        do {
          size_t i0 = 0, i1 = ev.packet.find(' ');
          if (i1 == std::string::npos)
            break;
          std::string part(ev.packet, i0, i1);
          if (part != "OK")
            break;
          i0 = i1 + 1;
          i1 = ev.packet.find(' ', i0);
          if (i1 == std::string::npos)
            break;
          part = std::string(ev.packet, i0, i1 - i0);
          if (part != "EUDAQ")
            break;
          i0 = i1 + 1;
          i1 = ev.packet.find(' ', i0);
          if (i1 == std::string::npos)
            break;
          part = std::string(ev.packet, i0, i1 - i0);
          if (part != "DATA")
            break;
          i0 = i1 + 1;
          i1 = ev.packet.find(' ', i0);
          part = std::string(ev.packet, i0, i1 - i0);
          ev.id.SetType(part);
          i0 = i1 + 1;
          i1 = ev.packet.find(' ', i0);
          part = std::string(ev.packet, i0, i1 - i0);
          ev.id.SetName(part);
        } while (false);
        // std::cout << "client replied, sending OK" << std::endl;
        m_dataserver->SendPacket("OK", ev.id, true);
        ev.id.SetState(1); // successfully identified
        OnConnect(ev.id);
      } else {
        // std::cout << "Receive: " << ev.id << " " << ev.packet.size() <<
        // std::endl;
        // for (size_t i = 0; i < 8 && i < ev.packet.size(); ++i) {
        //    std::cout << to_hex(ev.packet[i], 2) << ' ';
        //}
        // std::cout << ")" << std::endl;
        BufferSerializer ser(ev.packet.begin(), ev.packet.end());
        // std::cout << "Deserializing" << std::endl;
        std::shared_ptr<Event> event(EventFactory::Create(ser));
        // std::cout << "Done" << std::endl;
        OnReceive(ev.id, event);
        // std::cout << "End" << std::endl;
      }
      break;
    default:
      std::cout << "Unknown:    " << ev.id << std::endl;
    }
  }

  void DataCollector::DataThread() {
    try {
      while (!m_done) {
        m_dataserver->Process(100000);
      }
    } catch (const std::exception &e) {
      std::cout << "Error: Uncaught exception: " << e.what() << "\n"
                << "DataThread is dying..." << std::endl;
    } catch (...) {
      std::cout << "Error: Uncaught unrecognised exception: \n"
                << "DataThread is dying..." << std::endl;
    }
  }
}
