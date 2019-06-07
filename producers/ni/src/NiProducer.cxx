#include "NiController.hh"
#include "eudaq/Producer.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Utils.hh"
#include "eudaq/Logger.hh"
#include "eudaq/OptionParser.hh"

#include <stdio.h>
#include <stdlib.h>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MIMOSA_NI_UDP_VERSION 0x1

using eudaq::RawDataEvent;

class NiProducer : public eudaq::Producer {
public:
  NiProducer(const std::string &runcontrol)
      : eudaq::Producer("MimosaNI", runcontrol), done(false), running(false),
        stopping(false) {

    configure = false;

    std::cout << "NI Producer was started successful " << std::endl;
  }
  void MainLoop() {
    do {
      if (!running) {
        eudaq::mSleep(50);
        continue;
      }
      if (running) {

        datalength1 = ni_control->DataTransportClientSocket_ReadLength("priv");
        std::vector<unsigned char> mimosa_data_0(datalength1);
        mimosa_data_0 =
            ni_control->DataTransportClientSocket_ReadData(datalength1);

        datalength2 = ni_control->DataTransportClientSocket_ReadLength("priv");
        std::vector<unsigned char> mimosa_data_1(datalength2);
        mimosa_data_1 =
            ni_control->DataTransportClientSocket_ReadData(datalength2);

        eudaq::RawDataEvent ev("NI", m_run, m_ev++);
        ev.AddBlock(0, mimosa_data_0);
        ev.AddBlock(1, mimosa_data_1);
        SendEvent(ev);

	// Sending UDP packages for #BL4S purposes. Adding header, flags and trailer
	if(SendUDP){

	  (reinterpret_cast<int32_t*>(m_buffer))[1] = m_ev; // place event number in buffer

	  memcpy(m_buffer + 2*4, &(mimosa_data_0[0]), mimosa_data_0.size()); // write data into buffer at correct position
	  m_buffer[3] = m_buffer[3] & 0xF0 | 0x00;
	  SendBlockUDP(m_buffer,mimosa_data_0.size() + 2*4);

	  memcpy(m_buffer + 2*4, &(mimosa_data_1[0]), mimosa_data_1.size());
	  m_buffer[3] = m_buffer[3] & 0xF0 | 0x01;
	  SendBlockUDP(m_buffer,mimosa_data_1.size() + 2*4);
	}
      }

    } while (!done);
  }
  virtual void OnConfigure(const eudaq::Configuration &param) {

    unsigned char configur[5] = "conf";

    try {
      if (!configure) {
        ni_control = std::make_shared<NiController>();
        ni_control->GetProduserHostInfo();
        ni_control->ConfigClientSocket_Open(param);
        ni_control->DatatransportClientSocket_Open(param);
        std::cout << " " << std::endl;
        configure = true;
      }

      // Getting UDP config parameters and connect to socket #BL4S
      SendUDP = param.Get("SendUDP", 0);
      if (SendUDP){
	UDPIP = param.Get("UDPReceiverIP", "127.0.0.1");
	UDPPort = param.Get("UDPReceiverPort", 8080);
	if(ConnectUDP(UDPIP,UDPPort) < 0){
	  EUDAQ_ERROR("UDP connection failed");
	}
      }

      // Default parameters
      TriggerType = param.Get("TriggerType", 1);
      Det = param.Get("Det", 255);
      Mode = param.Get("Mode", 255);
      NiVersion = param.Get("NiVersion", 1);
      NumBoards = param.Get("NumBoards", 6);
      FPGADownload = param.Get("FPGADownload", 1);
      for (unsigned char i = 0; i < 6; i++) {
        MimosaID[i] = param.Get("MimosaID_" + to_string(i + 1), i+1);
        MimosaEn[i] = param.Get("MimosaEn_" + to_string(i + 1), 1);
      }
      OneFrame = param.Get("OneFrame", 0);

      std::cout << "Configuring ...(" << param.Name() << ")" << std::endl;

      conf_parameters[0] = NiVersion;
      conf_parameters[1] = TriggerType;
      conf_parameters[2] = Det;
      conf_parameters[3] = MimosaEn[1];
      conf_parameters[4] = MimosaEn[2];
      conf_parameters[5] = MimosaEn[3];
      conf_parameters[6] = MimosaEn[4];
      conf_parameters[7] = MimosaEn[5];
      conf_parameters[8] = NumBoards;
      conf_parameters[9] = FPGADownload;

      ni_control->ConfigClientSocket_Send(configur, sizeof(configur));
      ni_control->ConfigClientSocket_Send(conf_parameters,
                                          sizeof(conf_parameters));

      ConfDataLength = ni_control->ConfigClientSocket_ReadLength("priv");
      ConfDataError = ni_control->ConfigClientSocket_ReadData(ConfDataLength);

      NiConfig = false;

      if ((ConfDataError[3] & 0x1) >> 0) {
        EUDAQ_ERROR("NI crate can not be configure: ErrorReceive Config");
        NiConfig = true;
      } // ErrorReceive Config
      if ((ConfDataError[3] & 0x2) >> 1) {
        EUDAQ_ERROR("NI crate can not be configure: Error FPGA open");
        NiConfig = true;
      } // Error FPGA open
      if ((ConfDataError[3] & 0x4) >> 2) {
        EUDAQ_ERROR("NI crate can not be configure: Error FPGA reset");
        NiConfig = true;
      } // Error FPGA reset
      if ((ConfDataError[3] & 0x8) >> 3) {
        EUDAQ_ERROR("NI crate can not be configure: Error FPGA download");
        NiConfig = true;
      } // Error FPGA download
      if ((ConfDataError[3] & 0x10) >> 4) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_0 Start");
        NiConfig = true;
      } // FIFO_0 Configure
      if ((ConfDataError[3] & 0x20) >> 5) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_1 Start");
        NiConfig = true;
      } // FIFO_0 Start
      if ((ConfDataError[3] & 0x40) >> 6) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_2 Start");
        NiConfig = true;
      } // FIFO_1 Configure
      if ((ConfDataError[3] & 0x80) >> 7) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_3 Start");
        NiConfig = true;
      } // FIFO_1 Start
      if ((ConfDataError[2] & 0x1) >> 0) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_4 Start");
        NiConfig = true;
      } // FIFO_2 Configure
      if ((ConfDataError[2] & 0x2) >> 1) {
        EUDAQ_ERROR("NI crate can not be configure: FIFO_5 Start");
        NiConfig = true;
      } // FIFO_2 Start

      if (NiConfig) {
        std::cout << "NI crate was Configured with ERRORs " << param.Name()
                  << " " << std::endl;
        SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Configuration Error");
      } else {
        std::cout << "... was Configured " << param.Name() << " " << std::endl;
        EUDAQ_INFO("Configured (" + param.Name() + ")");
        SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (" + param.Name() + ")");
      }
    } catch (const std::exception &e) {
      printf("Caught exception: %s\n", e.what());
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Configuration Error");
    } catch (...) {
      printf("Unknown exception\n");
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Configuration Error");
    }

    // Writing parts of the buffer that will not change
    if(SendUDP) {
      // m_MimosaBitmask stands for the enabled sensor planes
      m_MimosaBitmask = 0x0;
      for (int i = 0; i < 6; ++i) {
	m_MimosaBitmask = m_MimosaBitmask | (((MimosaEn[i])?0x01:0x00) << i);
      }

      m_buffer[0] = m_MimosaBitmask;
      m_buffer[3] = MIMOSA_NI_UDP_VERSION << 4;
    }
  }
  virtual void OnStartRun(unsigned param) {
    try {
      m_run = param;
      m_ev = 0;
      std::cout << "Start Run: " << param << std::endl;

      eudaq::RawDataEvent ev(RawDataEvent::BORE("NI", m_run));

      ev.SetTag("DET", "MIMOSA26");
      ev.SetTag("MODE", "ZS2");
      ev.SetTag("BOARDS", NumBoards);
      for (unsigned int i = 0; i < 6; i++)
        ev.SetTag("ID" + to_string(i), to_string(MimosaID[i]));
      for (unsigned int i = 0; i < 6; i++)
        ev.SetTag("MIMOSA_EN" + to_string(i), to_string(MimosaEn[i]));
      SendEvent(ev);
      eudaq::mSleep(500);

      ni_control->Start();
      running = true;

      SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "Started");
    } catch (const std::exception &e) {
      printf("Caught exception: %s\n", e.what());
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Start Error");
    } catch (...) {
      printf("Unknown exception\n");
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Start Error");
    }
  }
  virtual void OnStopRun() {
    try {
      std::cout << "Stop Run" << std::endl;

      ni_control->Stop();
      eudaq::mSleep(5000);
      running = false;
      eudaq::mSleep(100);
      // Send an EORE after all the real events have been sent
      // You can also set tags on it (as with the BORE) if necessary
      SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Stopped");
      SendEvent(eudaq::RawDataEvent::EORE("NI", m_run, m_ev));

    } catch (const std::exception &e) {
      printf("Caught exception: %s\n", e.what());
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Stop Error");
    } catch (...) {
      printf("Unknown exception\n");
      SetConnectionState(eudaq::ConnectionState::STATE_ERROR, "Stop Error");
    }
  }
  virtual void OnTerminate() {
    std::cout << "Terminate (press enter)" << std::endl;
    done = true;
    ni_control->DatatransportClientSocket_Close();
    ni_control->ConfigClientSocket_Close();
    eudaq::mSleep(1000);
  }

private:
  unsigned m_run, m_ev;
  bool done, running, stopping, configure;
  struct timeval tv;
  std::shared_ptr<NiController> ni_control;

  char *Buffer1;
  unsigned int datalength1;
  char *Buffer2;
  unsigned int datalength2;
  unsigned int ConfDataLength;
  std::vector<unsigned char> ConfDataError;
  int r;
  int t;

  unsigned int Header0;
  unsigned int Header1;
  unsigned int MIMOSA_Counter0;
  unsigned int MIMOSA_Counter1;
  unsigned int MIMOSA_Datalen0;
  unsigned int MIMOSA_Datalen1;
  unsigned int MIMOSA_Datalen;
  unsigned int MIMOSA_Trailer0;
  unsigned int MIMOSA_Trailer1;
  unsigned int StatusLine;
  unsigned int OVF;
  unsigned int NumOfState;
  unsigned int AddLine;
  unsigned int State;
  unsigned int AddColum;
  unsigned int HitPix;

  unsigned int data[7000];
  unsigned char NumOfDetector;
  unsigned int NumOfData;
  unsigned int NumOfLengt;
  unsigned int MIMOSA_DatalenTmp;
  int datalengthAll;

  unsigned TriggerType;
  unsigned Det;
  unsigned Mode;
  unsigned NiVersion;
  unsigned NumBoards;
  unsigned FPGADownload;
  unsigned MimosaID[6];
  unsigned MimosaEn[6];
  bool OneFrame;
  bool NiConfig;

  bool SendUDP;
  unsigned UDPPort;
  std::string UDPIP;
  unsigned char m_buffer[MAX_UDP_SIZE];

  char m_MimosaBitmask;
  
  unsigned char conf_parameters[10];
};
// ----------------------------------------------------------------------
int main(int /*argc*/, const char **argv) {
  std::cout << "Start Producer \n" << std::endl;

  eudaq::OptionParser op("EUDAQ NI Producer", "0.1",
                         "The Producer task for the NI crate");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
                                   "tcp://localhost:44000", "address",
                                   "The address of the RunControl application");
  eudaq::Option<std::string> level(
      op, "l", "log-level", "NONE", "level",
      "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name(op, "n", "name", "NI", "string",
                                  "The name of this Producer");
  try {
    op.Parse(argv);
    EUDAQ_LOG_LEVEL(level.Value());
    NiProducer producer(rctrl.Value());
    producer.MainLoop();
    eudaq::mSleep(500);
  } catch (...) {
    return op.HandleMainException();
  }
  return 0;
}
