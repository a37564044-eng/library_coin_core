#include "src/script.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== LARB Script OP_RETURN Test ===" << std::endl;

    // Test 1: pesan biasa
    const std::string message = "LARB";

    const std::string script = larb::Script::op_return(message);

    assert(!script.empty());
    assert(larb::Script::is_op_return(script));

    const std::string extracted =
        larb::Script::extract_op_return_data(script);

    assert(extracted == message);

    std::cout << "OP_RETURN basic: OK" << std::endl;
    std::cout << "Extract data: OK" << std::endl;

    // Test 2: pesan kosong
    const std::string empty_message = "";

    const std::string empty_script =
        larb::Script::op_return(empty_message);

    assert(larb::Script::is_op_return(empty_script));

    const std::string empty_extracted =
        larb::Script::extract_op_return_data(empty_script);

    assert(empty_extracted == empty_message);

    std::cout << "Empty payload: OK" << std::endl;

    // Test 3: pesan agak panjang
    const std::string long_message(80, 'A');

    const std::string long_script =
        larb::Script::op_return(long_message);

    assert(larb::Script::is_op_return(long_script));

    const std::string long_extracted =
        larb::Script::extract_op_return_data(long_script);

    assert(long_extracted == long_message);

    std::cout << "80-byte payload: OK" << std::endl;

    // Test 4: script bukan OP_RETURN
    const std::string fake_script = "LARB_NORMAL_SCRIPT";

    assert(!larb::Script::is_op_return(fake_script));

    std::cout << "Non-OP_RETURN rejection: OK" << std::endl;

    // Test 5: payload harus tetap sama
    const std::string transaction_data =
        "LARB transaction data 123456789";

    const std::string transaction_script =
        larb::Script::op_return(transaction_data);

    const std::string recovered =
        larb::Script::extract_op_return_data(transaction_script);

    assert(recovered == transaction_data);

    std::cout << "Payload integrity: OK" << std::endl;

    std::cout << "=== ALL SCRIPT TESTS PASSED ==="
              << std::endl;

    return 0;
}
