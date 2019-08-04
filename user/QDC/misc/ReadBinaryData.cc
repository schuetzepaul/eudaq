#include <fstream>
#include <cstdint>
#include <vector>
#include <iostream>
using namespace std;
int main(){
    std::ifstream InputBinary;
    std::ofstream OutputNormal;
    std::vector<uint16_t> Data;
    InputBinary.open("Data.dat", std::ios::binary);
    OutputNormal.open("De_Binarized_Data.txt");
    uint16_t cData;
    uint32_t cData_32;
    int count = 0;
    do{
        cData=0;
        cData_32 = 0;
        if(count % 11 == 0 ){
            InputBinary.read((char *) &cData_32, sizeof(uint32_t));
            OutputNormal<<cData_32<<std::endl;
        }
        else{
            InputBinary.read((char *) &cData, sizeof(uint16_t));
            OutputNormal<<cData<<std::endl;
            Data.push_back(cData);
        }
        count = count +1 ;
        cout << count << "\t" << cData<< "\t" << cData_32<<endl;


    }while(cData != 0 || cData_32 !=0 || count == 1);
//    for(int i = 0; i < count; i++){
//        OutputNormal<<Data[i]<<std::endl;
//    }
}
