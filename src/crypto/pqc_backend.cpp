#include "pqc_backend.h"

namespace larb {

class UnavailablePQCBackend : public PQCBackend {
public:
    std::string algorithm() const override {
        return "PQC-NOT-INSTALLED";
    }

    bool available() const override {
        return false;
    }
};

}
