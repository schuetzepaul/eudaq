#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <unistd.h>
#include <sys/time.h>
#define Sleep(x) usleep((x)*1000)

#include "eudaq/Producer.hh"

#include <CAENVMElib.h>
#include <CAENVMEtypes.h>
#include "QDCControl.hh"



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
    uint32_t tmp;
    QDCControl * m_qdc;
};

namespace{
auto dummy0 = eudaq::Factory<eudaq::Producer>::
        Register<QDCProducer, const std::string&, const std::string&>(QDCProducer::m_id_factory);
}

QDCProducer::QDCProducer(const std::string name, const std::string &runcontrol)
    : eudaq::Producer(name, runcontrol), m_running(false), tmp(0){
    m_running = false;
}

QDCProducer::~QDCProducer(){
    m_running = false;
    if(m_qdc != nullptr)
        delete m_qdc;
}

void QDCProducer::RunLoop(){
    while(m_running){
        if(m_qdc -> ReadData()){
            auto ev = eudaq::Event::MakeUnique("QDCRaw");
            ev->SetTag("Plane ID", std::to_string(0));
            std::vector<uint16_t>  data =  m_qdc->copyData();
            ev->SetTriggerN(tmp);
            tmp++;
            ev->AddBlock(0,data);
            SendEvent(std::move(ev));
        }
    }
}

void QDCProducer::DoInitialise(){
    m_qdc = new QDCControl();
    if(!m_qdc->Connect())
        EUDAQ_THROW("Can not connect to QDC");
    auto conf = GetInitConfiguration();

    //Get channels to be activated out of the config file
    //Read config file
    std::string info = conf -> Get("activeChannels","0");
    m_qdc->SetupChannels(info);
    EUDAQ_USER(info);
}

void QDCProducer::DoConfigure(){
    //reset variables
    m_qdc -> ResetVariables();

    //Configure
    auto conf = GetConfiguration();
    std::string TextFile = conf -> Get("TextFile","Output.txt");
    std::string BinaryOutputFile = conf -> Get("BinaryFile","Output.dat");
    EUDAQ_USER(TextFile);
    m_qdc -> SelectFileToWriteTo(TextFile);
    m_qdc -> SelectBinFileToWriteTo(BinaryOutputFile);
}

void QDCProducer::DoStartRun(){
    if(m_qdc -> StartDataTaking()){
        m_running = true;
    }
    else{
        EUDAQ_THROW("Can not start the run");
    }

}

void QDCProducer::DoStopRun(){
    m_running = false;
    //reset variables
    m_qdc -> ResetVariables();

}

void QDCProducer::DoReset(){
    m_running = false;
}

void QDCProducer::DoTerminate() {
    m_running = false;
}
