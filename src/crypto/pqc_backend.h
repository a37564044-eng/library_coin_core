#pragma once

#include <string>

namespace larb {

class PQCBackend {
public:
    virtual ~PQCBackend() = default;

    virtual std::string algorithm() const = 0;
    virtual bool available() const = 0;
};

}
