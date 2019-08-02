#include "eudaq/StdEventConverter.hh"
#include "eudaq/RawEvent.hh"


class QDCRawEvent2StdEventConverter: public eudaq::StdEventConverter{
public:
    bool Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf) const override;
    static const uint32_t m_id_factory = eudaq::cstr2hash("QDCRaw");
};

namespace{
auto dummy0 = eudaq::Factory<eudaq::StdEventConverter>::
        Register<QDCRawEvent2StdEventConverter>(QDCRawEvent2StdEventConverter::m_id_factory);
}

bool QDCRawEvent2StdEventConverter::Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf) const{
    auto ev = std::dynamic_pointer_cast<const eudaq::RawEvent>(d1);
    auto block =  ev->GetBlock(0); // vector<uint8_t> data
    uint32_t numChannels =32;
    eudaq::StandardPlane plane(0, "QDCRaw", "QDCRaw");
    plane.SetSizeZS(32,65536,numChannels);
    std::vector<uint16_t> data;
    uint16_t value = 0;
    int count =0;
    for(auto val : block)
    {
        count++;
        if(count%2==0)
        {
            value += val;
            data.push_back(value);
        }
        else
            value = val <<0xFF;
    }
    int channel = 0;
    for(auto d : data){
        plane.SetPixel(channel,channel,d,0);
        channel++;
    }
    d2->AddPlane(plane);
    return true;
}
