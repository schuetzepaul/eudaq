#include "eudaq/OptionParser.hh"
#include "eudaq/FileReader.hh"
#include "eudaq/StdEventConverter.hh"

#include <string>
#include "TFile.h"
#include "TTree.h"



#include <iostream>

int main(int /*argc*/, const char **argv) {
  eudaq::OptionParser op("EUDAQ Command Line FileReader modified for TLU", "2.1", "EUDAQ FileReader (TLU)");
  eudaq::Option<std::string> file_input(op, "i", "input", "", "string", "input file");
  eudaq::Option<std::string> file_output(op, "o", "output", "test.root", "string", "output file");
  eudaq::Option<uint32_t> eventl(op, "e", "event", 0, "uint32_t", "event number low");
  eudaq::Option<uint32_t> eventh(op, "E", "eventhigh", 0, "uint32_t", "event number high");
  eudaq::Option<uint32_t> triggerl(op, "tg", "trigger", 0, "uint32_t", "trigger number low");
  eudaq::Option<uint32_t> triggerh(op, "TG", "triggerhigh", 0, "uint32_t", "trigger number high");
  eudaq::Option<uint32_t> timestampl(op, "ts", "timestamp", 0, "uint32_t", "timestamp low");
  eudaq::Option<uint32_t> timestamph(op, "TS", "timestamphigh", 0, "uint32_t", "timestamp high");
  eudaq::OptionFlag stat(op, "s", "statistics", "enable print of statistics");
  eudaq::OptionFlag stdev(op, "std", "stdevent", "enable converter of StdEvent");

  op.Parse(argv);

  double event_count = 0;
  double xval[10], yval[10];


  /** Open root file and define branches*/
  TFile *RootFile = new TFile(file_output.Value().c_str(),"RECREATE");
  TTree *RootTree = new TTree("T","A Root Tree");

  std::string infile_path = file_input.Value();
  std::string type_in = infile_path.substr(infile_path.find_last_of(".")+1);
  if(type_in=="raw")
    type_in = "native";

  bool stdev_v = stdev.Value();


  uint32_t eventl_v = eventl.Value();
  uint32_t eventh_v = eventh.Value();
  uint32_t triggerl_v = triggerl.Value();
  uint32_t triggerh_v = triggerh.Value();
  uint32_t timestampl_v = timestampl.Value();
  uint32_t timestamph_v = timestamph.Value();
  bool not_all_zero = eventl_v||eventh_v||triggerl_v||triggerh_v||timestampl_v||timestamph_v;

  eudaq::FileReaderUP reader;
  reader = eudaq::Factory<eudaq::FileReader>::MakeUnique(eudaq::str2hash(type_in), infile_path);
//number of activated channels in the QDC
  int ch_num = 10;

  //Define Branches
  RootTree->Branch("EventNumber",&event_count);
  for(int i = 0; i<ch_num;i++){
      std::string cx = "x_";
      cx += std::to_string(i);
      char cxstr[cx.size() + 1];
      strcpy(cxstr, cx.c_str());
      std::string cy = "y_";
      cy += std::to_string(i);
      char cystr[cy.size() + 1];
      strcpy(cystr, cy.c_str());
      RootTree->Branch(cxstr,&xval[i]);
      RootTree->Branch(cystr,&yval[i]);
  }

  while(1){
    auto ev = reader->GetNextEvent();
    if(!ev)
      break;
    bool in_range_evn = false;
    if(eventl_v!=0 || eventh_v!=0){
      uint32_t ev_n = ev->GetEventN();
      if(ev_n >= eventl_v && ev_n < eventh_v){
        in_range_evn = true;
      }
    }
    else
      in_range_evn = true;

    bool in_range_tgn = false;
    if(triggerl_v!=0 || triggerh_v!=0){
      uint32_t tg_n = ev->GetTriggerN();
      if(tg_n >= triggerl_v && tg_n < triggerh_v){
        in_range_tgn = true;
      }
    }
    else
      in_range_tgn = true;

    bool in_range_tsn = false;
    if(timestampl_v!=0 || timestamph_v!=0){
      uint32_t ts_beg = ev->GetTimestampBegin();
      uint32_t ts_end = ev->GetTimestampEnd();
      if(ts_beg >= timestampl_v && ts_end <= timestamph_v){
        in_range_tsn = true;
      }
    }
    else
      in_range_tsn = true;


    //if((in_range_evn && in_range_tgn && in_range_tsn) && not_all_zero){
      //ev->Print(std::cout);
      //if(stdev_v){
        auto evstd = eudaq::StandardEvent::MakeShared();
        eudaq::StdEventConverter::Convert(ev, evstd, nullptr); // event comverter defined by modules
        /** you root code here
            -> Take pixels hits and copy to branch variables
            ->tree->Fill()
*/

        for(int i=0; i<evstd->NumPlanes(); i++){
            auto plane = evstd->GetPlane(i);
            if(plane.Type()=="QDCRaw")
            {
                xval[plane.ID()] = plane.ID();
                yval[plane.ID()] = plane.GetY(0);
            //    std::cout<< "---" <<plane.ID()<<"' "<< plane.GetY(0) << "---"<<std::endl;
            }
           // evstd->GetPlane(i).Print(std::cout);
        }
     //      std::cout<< ">>>>>"<< evstd->NumPlanes() <<"<<<<"<<std::endl;
     // }
    //}
    RootTree->Fill();
    event_count ++;
  }

  /** Write tree and close file*/
  RootTree->Write();
  RootFile->Close();
  std::cout<< "There are "<< event_count << "Events"<<std::endl;
  return 0;
}
