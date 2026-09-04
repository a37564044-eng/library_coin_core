cmake_minimum_required(VERSION 3.10)

project(library_coin_core)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_library(coin_core STATIC
    src/consensus/coin_core.cpp
    src/wallet/wallet.cpp
    src/address_codec.cpp
    src/crypto/pqc.cpp
    src/transaction.cpp
    src/utxo.cpp
    src/utxo_set.cpp
    src/block.cpp
    src/script.cpp
    src/genesis.cpp
    src/blockchain.cpp
    src/node.cpp
    src/p2p.cpp
    src/chain_codec.cpp
    src/transaction_validator.cpp
    src/message_transaction.cpp
    src/message.cpp
    src/block_validator.cpp
    src/consensus/pow.cpp
    src/persistence.cpp
    src/mainnet_final.cpp
)

target_include_directories(coin_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wallet
    /public/liboqs/build/include
)

target_link_libraries(coin_core PUBLIC
    /public/liboqs/build/lib/liboqs.a
    OpenSSL::Crypto
)

add_executable(main
    main.cpp
)

target_link_libraries(main PRIVATE
    coin_core
)

add_executable(mainnet_final
    mainnet_final.cpp
)

target_link_libraries(mainnet_final PRIVATE
    coin_core
)

add_executable(script_test
    tests/script_test.cpp
)

target_include_directories(script_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(script_test PRIVATE
    coin_core
)

add_executable(transaction_test
    tests/transaction_test.cpp
)

target_include_directories(transaction_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(transaction_test PRIVATE
    coin_core
)

add_executable(utxo_test
    tests/utxo_test.cpp
)

target_include_directories(utxo_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(utxo_test PRIVATE
    coin_core
)

add_executable(utxo_set_test
    tests/utxo_set_test.cpp
)

target_include_directories(utxo_set_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(utxo_set_test PRIVATE
    coin_core
)

add_executable(block_test
    tests/block_test.cpp
)

target_include_directories(block_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(block_test PRIVATE
    coin_core
)

add_executable(genesis_test
    tests/genesis_test.cpp
)

target_include_directories(genesis_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(genesis_test PRIVATE
    coin_core
)

add_executable(blockchain_test
    tests/blockchain_test.cpp
)

target_include_directories(blockchain_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(blockchain_test PRIVATE
    coin_core
)

add_executable(blockchain_tamper_test
    tests/blockchain_tamper_test.cpp
)

target_include_directories(blockchain_tamper_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(blockchain_tamper_test PRIVATE
    coin_core
)

add_executable(node_test
    tests/node_test.cpp
)

target_include_directories(node_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(node_test PRIVATE
    coin_core
)

add_executable(p2p_test
    tests/p2p_test.cpp
)

target_include_directories(p2p_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_test PRIVATE
    coin_core
)

add_executable(p2p_tcp_test
    tests/p2p_tcp_test.cpp
)

target_include_directories(p2p_tcp_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_tcp_test PRIVATE
    coin_core
)


add_executable(p2p_get_chain_test
    tests/p2p_get_chain_test.cpp
)

target_include_directories(p2p_get_chain_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_get_chain_test PRIVATE
    coin_core
)

add_executable(chain_serialization_test
    tests/chain_serialization_test.cpp
)

target_include_directories(chain_serialization_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chain_serialization_test PRIVATE
    coin_core
)

add_executable(chain_codec_test
    tests/chain_codec_test.cpp
)

target_include_directories(chain_codec_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chain_codec_test PRIVATE
    coin_core
)

add_executable(p2p_chain_test
    tests/p2p_chain_test.cpp
)

target_include_directories(p2p_chain_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_chain_test PRIVATE
    coin_core
)

add_executable(chain_decode_test
    tests/chain_decode_test.cpp
)

target_include_directories(chain_decode_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chain_decode_test PRIVATE
    coin_core
)

add_executable(halving_test
    tests/halving_test.cpp
)

target_include_directories(halving_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(halving_test PRIVATE
    coin_core
)

add_executable(utxo_basic_test
    tests/utxo_basic_test.cpp
)

target_include_directories(utxo_basic_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(utxo_basic_test PRIVATE
    coin_core
)

add_executable(coinbase_test
    tests/coinbase_test.cpp
)

target_include_directories(coinbase_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(coinbase_test PRIVATE
    coin_core
)

add_executable(block_reward_test
    tests/block_reward_test.cpp
)

target_include_directories(block_reward_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(block_reward_test PRIVATE
    coin_core
)

add_executable(coinbase_utxo_test
    tests/coinbase_utxo_test.cpp
)

target_include_directories(coinbase_utxo_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(coinbase_utxo_test PRIVATE
    coin_core
)

add_executable(transaction_validator_test
    tests/transaction_validator_test.cpp
)

target_include_directories(transaction_validator_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(transaction_validator_test PRIVATE
    coin_core
)

add_executable(transaction_value_test
    tests/transaction_value_test.cpp
)

target_include_directories(transaction_value_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(transaction_value_test PRIVATE
    coin_core
)

add_executable(transaction_apply_test
    tests/transaction_apply_test.cpp
)

target_include_directories(transaction_apply_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(transaction_apply_test PRIVATE
    coin_core
)


add_executable(transaction_codec_test
    tests/transaction_codec_test.cpp
)

target_include_directories(transaction_codec_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(transaction_codec_test PRIVATE
    coin_core
)

add_executable(block_validator_test
    tests/block_validator_test.cpp
)

target_include_directories(block_validator_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(block_validator_test PRIVATE
    coin_core
)

add_executable(block_atomicity_test
    tests/block_atomicity_test.cpp
)

target_link_libraries(block_atomicity_test PRIVATE coin_core)

add_executable(block_rules_test
    tests/block_rules_test.cpp
)

target_link_libraries(block_rules_test PRIVATE coin_core)

add_executable(double_spend_block_test
    tests/double_spend_block_test.cpp
)

target_link_libraries(double_spend_block_test PRIVATE coin_core)

add_executable(duplicate_input_test
    tests/duplicate_input_test.cpp
)

target_link_libraries(duplicate_input_test PRIVATE coin_core)

add_executable(money_range_test
    tests/money_range_test.cpp
)

target_link_libraries(money_range_test
    PRIVATE coin_core
)

add_executable(transaction_fee_test
    tests/transaction_fee_test.cpp
)

target_include_directories(transaction_fee_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(transaction_fee_test PRIVATE coin_core)

add_executable(block_fee_test
    tests/block_fee_test.cpp
)

target_include_directories(block_fee_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(block_fee_test PRIVATE coin_core)


add_executable(block_multi_fee_test
    tests/block_multi_fee_test.cpp
)

target_include_directories(block_multi_fee_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(block_multi_fee_test PRIVATE coin_core)

add_executable(pow_test
    tests/pow_test.cpp
)

target_include_directories(pow_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(pow_test PRIVATE coin_core)

add_executable(p2p_submit_block_test
    tests/p2p_submit_block_test.cpp
)

target_include_directories(p2p_submit_block_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_submit_block_test PRIVATE
    coin_core
)

add_executable(op_return_test
    tests/op_return_test.cpp
)

target_include_directories(op_return_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(op_return_test PRIVATE
    coin_core
)

add_executable(node_chain_sync_test
    tests/node_chain_sync_test.cpp
)

target_include_directories(node_chain_sync_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(node_chain_sync_test PRIVATE
    coin_core
)

add_executable(p2p_node_sync_test
    tests/p2p_node_sync_test.cpp
)

target_include_directories(p2p_node_sync_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_node_sync_test PRIVATE
    coin_core
)

add_executable(p2p_block_broadcast_test
    tests/p2p_block_broadcast_test.cpp
)

target_include_directories(p2p_block_broadcast_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_block_broadcast_test PRIVATE
    coin_core
)

add_executable(p2p_multi_node_broadcast_test
    tests/p2p_multi_node_broadcast_test.cpp
)

target_include_directories(p2p_multi_node_broadcast_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_multi_node_broadcast_test PRIVATE
    coin_core
)

add_executable(p2p_peer_broadcast_test
    tests/p2p_peer_broadcast_test.cpp
)

target_include_directories(p2p_peer_broadcast_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(p2p_peer_broadcast_test PRIVATE
    coin_core
)

add_executable(wallet_test
    tests/wallet_test.cpp
)

target_include_directories(wallet_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(wallet_test PRIVATE
    coin_core
)


add_executable(chainwork_test
    tests/chainwork_test.cpp
)

target_link_libraries(chainwork_test PRIVATE coin_core)

target_include_directories(chainwork_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(reorg_work_test
    tests/reorg_work_test.cpp
)

target_link_libraries(reorg_work_test PRIVATE coin_core)

target_include_directories(reorg_work_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(pqc_test
    tests/pqc_test.cpp
)

target_include_directories(pqc_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pqc_test PRIVATE
    coin_core
)


add_executable(transaction_signature_test
    tests/transaction_signature_test.cpp
)
target_link_libraries(transaction_signature_test PRIVATE coin_core)
target_include_directories(transaction_signature_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(transaction_multi_input_signature_test
    tests/transaction_multi_input_signature_test.cpp
)
target_link_libraries(transaction_multi_input_signature_test PRIVATE coin_core)
target_include_directories(transaction_multi_input_signature_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(block_timestamp_test
    tests/block_timestamp_test.cpp
)

target_link_libraries(block_timestamp_test
    PRIVATE coin_core
)

add_executable(fork_reorg_utxo_test
    tests/fork_reorg_utxo_test.cpp
)

target_link_libraries(fork_reorg_utxo_test
    PRIVATE coin_core
)

add_executable(blockchain_genesis_lock_test
    tests/blockchain_genesis_lock_test.cpp
)

target_include_directories(blockchain_genesis_lock_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(blockchain_genesis_lock_test
    PRIVATE coin_core
)

add_executable(fork_reorg_transaction_test
    tests/fork_reorg_transaction_test.cpp
)

target_link_libraries(fork_reorg_transaction_test
    PRIVATE coin_core
)

add_executable(message_blockchain_test
    tests/message_blockchain_test.cpp
)

target_link_libraries(message_blockchain_test PRIVATE coin_core)


add_executable(wallet_address_test
    tests/wallet_address_test.cpp
)

target_link_libraries(wallet_address_test PRIVATE coin_core)

target_include_directories(wallet_address_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(coinbase_address_test
    tests/coinbase_address_test.cpp
)

target_link_libraries(coinbase_address_test PRIVATE coin_core)

target_include_directories(coinbase_address_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(address_codec_test
    tests/address_codec_test.cpp
)

target_include_directories(address_codec_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(address_codec_test PRIVATE
    coin_core
)

add_executable(coinbase_invalid_address_test
    tests/coinbase_invalid_address_test.cpp
)

target_include_directories(coinbase_invalid_address_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(coinbase_invalid_address_test PRIVATE
    coin_core
)

add_executable(persistence_test
    tests/persistence_test.cpp
)
target_include_directories(persistence_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(persistence_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(persistence_test PRIVATE
    coin_core
    OpenSSL::Crypto
)



add_executable(fuzz_chain_codec tests/fuzz_chain_codec.cpp)
target_link_libraries(fuzz_chain_codec PRIVATE coin_core)

add_executable(fuzz_transaction_validator tests/fuzz_transaction_validator.cpp)
target_link_libraries(fuzz_transaction_validator PRIVATE coin_core)

add_executable(fuzz_block_validator tests/fuzz_block_validator.cpp)
target_link_libraries(fuzz_block_validator PRIVATE coin_core)

add_executable(fuzz_p2p_message tests/fuzz_p2p_message.cpp)
target_link_libraries(fuzz_p2p_message PRIVATE coin_core)

add_executable(oversized_message_test tests/oversized_message_test.cpp)
target_link_libraries(oversized_message_test PRIVATE coin_core)

add_executable(integer_overflow_test tests/integer_overflow_test.cpp)
target_link_libraries(integer_overflow_test PRIVATE coin_core)

add_executable(signature_abuse_test tests/signature_abuse_test.cpp)
target_link_libraries(signature_abuse_test PRIVATE coin_core)

add_executable(double_spend_test tests/double_spend_test.cpp)
target_link_libraries(double_spend_test PRIVATE coin_core)

add_executable(reorg_stress_test tests/reorg_stress_test.cpp)
target_link_libraries(reorg_stress_test PRIVATE coin_core)

add_executable(persistence_corruption_test tests/persistence_corruption_test.cpp)
target_link_libraries(persistence_corruption_test PRIVATE coin_core)

add_executable(multi_node_stress_test tests/multi_node_stress_test.cpp)
target_link_libraries(multi_node_stress_test PRIVATE coin_core)

add_executable(wallet_encryption_test tests/wallet_encryption_test.cpp)
target_link_libraries(wallet_encryption_test PRIVATE coin_core OpenSSL::Crypto)
# CTest registration
add_test(NAME script_test COMMAND script_test)
add_test(NAME transaction_test COMMAND transaction_test)
add_test(NAME utxo_test COMMAND utxo_test)
add_test(NAME utxo_set_test COMMAND utxo_set_test)
add_test(NAME block_test COMMAND block_test)
add_test(NAME genesis_test COMMAND genesis_test)
add_test(NAME blockchain_test COMMAND blockchain_test)
add_test(NAME blockchain_tamper_test COMMAND blockchain_tamper_test)
add_test(NAME node_test COMMAND node_test)
add_test(NAME p2p_test COMMAND p2p_test)
add_test(NAME p2p_tcp_test COMMAND p2p_tcp_test)
add_test(NAME p2p_get_chain_test COMMAND p2p_get_chain_test)
add_test(NAME chain_serialization_test COMMAND chain_serialization_test)
add_test(NAME chain_codec_test COMMAND chain_codec_test)
add_test(NAME p2p_chain_test COMMAND p2p_chain_test)
add_test(NAME chain_decode_test COMMAND chain_decode_test)
add_test(NAME halving_test COMMAND halving_test)
add_test(NAME utxo_basic_test COMMAND utxo_basic_test)
add_test(NAME coinbase_test COMMAND coinbase_test)
add_test(NAME block_reward_test COMMAND block_reward_test)
add_test(NAME coinbase_utxo_test COMMAND coinbase_utxo_test)
add_test(NAME transaction_validator_test COMMAND transaction_validator_test)
add_test(NAME transaction_value_test COMMAND transaction_value_test)
add_test(NAME transaction_apply_test COMMAND transaction_apply_test)
add_test(NAME transaction_codec_test COMMAND transaction_codec_test)
add_test(NAME block_validator_test COMMAND block_validator_test)
add_test(NAME block_atomicity_test COMMAND block_atomicity_test)
add_test(NAME block_rules_test COMMAND block_rules_test)
add_test(NAME double_spend_block_test COMMAND double_spend_block_test)
add_test(NAME duplicate_input_test COMMAND duplicate_input_test)
add_test(NAME money_range_test COMMAND money_range_test)
add_test(NAME transaction_fee_test COMMAND transaction_fee_test)
add_test(NAME block_fee_test COMMAND block_fee_test)
add_test(NAME block_multi_fee_test COMMAND block_multi_fee_test)
add_test(NAME pow_test COMMAND pow_test)
add_test(NAME p2p_submit_block_test COMMAND p2p_submit_block_test)
add_test(NAME op_return_test COMMAND op_return_test)
add_test(NAME node_chain_sync_test COMMAND node_chain_sync_test)
add_test(NAME p2p_node_sync_test COMMAND p2p_node_sync_test)
add_test(NAME p2p_block_broadcast_test COMMAND p2p_block_broadcast_test)
add_test(NAME p2p_multi_node_broadcast_test COMMAND p2p_multi_node_broadcast_test)
add_test(NAME p2p_peer_broadcast_test COMMAND p2p_peer_broadcast_test)
add_test(NAME wallet_test COMMAND wallet_test)
add_test(NAME chainwork_test COMMAND chainwork_test)
add_test(NAME reorg_work_test COMMAND reorg_work_test)
add_test(NAME pqc_test COMMAND pqc_test)
add_test(NAME transaction_signature_test COMMAND transaction_signature_test)
add_test(NAME transaction_multi_input_signature_test COMMAND transaction_multi_input_signature_test)
add_test(NAME block_timestamp_test COMMAND block_timestamp_test)
add_test(NAME fork_reorg_utxo_test COMMAND fork_reorg_utxo_test)
add_test(NAME blockchain_genesis_lock_test COMMAND blockchain_genesis_lock_test)
add_test(NAME fork_reorg_transaction_test COMMAND fork_reorg_transaction_test)
add_test(NAME wallet_address_test COMMAND wallet_address_test)
add_test(NAME coinbase_address_test COMMAND coinbase_address_test)
add_test(NAME address_codec_test COMMAND address_codec_test)
add_test(NAME coinbase_invalid_address_test COMMAND coinbase_invalid_address_test)
add_test(NAME persistence_test COMMAND persistence_test)

add_executable(consensus_constants_test
    tests/consensus_constants_test.cpp
)

target_link_libraries(consensus_constants_test PRIVATE coin_core)

add_executable(genesis_canonical_test
    tests/genesis_canonical_test.cpp
)

target_include_directories(genesis_canonical_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(genesis_canonical_test PRIVATE
    coin_core
)

add_executable(canonical_genesis_miner
    tests/canonical_genesis_miner.cpp
)

target_include_directories(canonical_genesis_miner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(canonical_genesis_miner PRIVATE
    coin_core
)

add_executable(larb_mainnet
    mainnet.cpp
)

target_include_directories(larb_mainnet PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(larb_mainnet PRIVATE
    coin_core
)

add_executable(wallet_persistence_test
    tests/wallet_persistence_test.cpp
)

target_include_directories(wallet_persistence_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(wallet_persistence_test PRIVATE
    coin_core
)

add_executable(larb_miner
    larb_miner.cpp
)

target_include_directories(larb_miner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(larb_miner PRIVATE
    coin_core
)

add_executable(larb_node
    larb_node.cpp
)

target_include_directories(larb_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(larb_node PRIVATE
    coin_core
)

install(TARGETS main larb_miner larb_node
        RUNTIME DESTINATION bin)
