#include "eudaq/Producer.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <unistd.h>
#include <sys/time.h>
#define Sleep(x) usleep((x)*1000)
#include <CAENVMElib.h>
#include <CAENVMEtypes.h>
#include "QDCControl.hh"
//#include "console.h"


class QDCProducer : public eudaq::Producer {
public:
  QDCProducer(const std::string name, const std::string &runcontrol);
  ~QDCProducer() override;
  void DoInitialise() override;
  void DoConfigure() override;
  void DoStartRun() override;
  void DoStopRun() override;
  void DoReset() override;
  void DoTerminate() override;
  void RunLoop() override;

  static const uint32_t m_id_factory = eudaq::cstr2hash("QDCProducer");
private:
  bool m_running;
  QDCControl * m_qdc;
};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::Producer>::
    Register<QDCProducer, const std::string&, const std::string&>(QDCProducer::m_id_factory);
}

QDCProducer::QDCProducer(const std::string name, const std::string &runcontrol)
  : eudaq::Producer(name, runcontrol), m_running(false){
  m_running = false;
}

QDCProducer::~QDCProducer(){
  m_running = false;
  if(m_qdc != nullptr)
      delete m_qdc;
}

void QDCProducer::RunLoop(){
//    while(m_running)
//        if(m_qdc->ReadData())
//            m_qdc->Write();
}

void QDCProducer::DoInitialise(){
// m_qdc = new QDCControl();
// if(!m_qdc->Connect())
//     EUDAQ_THROW("Cannot connect to QDC");
//  auto conf = GetInitConfiguration();
}

void QDCProducer::DoConfigure(){
//  auto conf = GetConfiguration();
//  m_qdc->setUPChannels();
//  m_qdc->SelectFiletoWtieTo(std::strign filename);
}

void QDCProducer::DoStartRun(){
  m_running = true;
}

void QDCProducer::DoStopRun(){
  m_running = false;
}

void QDCProducer::DoReset(){
  m_running = false;
}

void QDCProducer::DoTerminate() {
  m_running = false;
}
