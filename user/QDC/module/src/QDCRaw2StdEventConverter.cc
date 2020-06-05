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
    auto block =  ev->GetBlock(0);

    std::vector<uint16_t> data;
    uint16_t value = 0;
    int count =0;
    for(auto val : block)
    {
        count++;
        if(count%2==0)
        {
            eudaq::StandardPlane plane(int((count-1)/2), "QDCRaw", "QDCRaw");
            plane.SetSizeZS(1,4096,1);
            value += (val <<8);
            plane.SetPixel(0,0,value,0);
            d2->AddPlane(plane);
            value =0;
        }
        else
        {
            value += val;
        }
    }
    return true;
}
