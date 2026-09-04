#pragma once

#include "transaction.h"
#include <string>

namespace larb {

class MessageTransaction {
public:

    static Transaction create(
        const std::string& document_id,
        const std::string& message
    );

};

}
