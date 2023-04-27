#define RTI_USES_STD_FSTREAM true
#include <iostream>
#include "RTI.hh"
#include "fedtime.hh"


int main(int argc, char* argv[])
{
    try {
        //RTI::RTIambassador* rtiAmbassador = new RTI::RTIambassador();
        RTI::RTIambassador* rtiAmbassador = RTI::RTIambassadorFactory::getRTIambassador();
        rtiAmbassador->connect(*new RTI::NullFederateAmbassador(), RTI::HLA_EVOKED, *new RTI::UnicodeString("127.0.0.1"), *new RTI::ULong(8989));
        std::cout << "Connected to HLA RTI server" << std::endl;
    }
    catch (RTI::Exception& e) {
        std::cerr << "Error: " << e._name << std::endl;
        std::cerr << e._reason << std::endl;
    }
    return 0;
}
