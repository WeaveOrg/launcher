#pragma once

#include "result.hpp"
#include "crypto_primitives.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace secure_proto {

constexpr uint32_t MAGIC_CLIENT_HELLO = 0x53515001; // "SQP1"
constexpr uint32_t MAGIC_SERVER_HELLO = 0x53515002; // "SQP2"
constexpr size_t CLIENT_HELLO_SIZE = 60; // 4 (magic) + 32 (pub) + 8 (timestamp) + 16 (tag)
constexpr size_t SERVER_HELLO_SIZE = 52; // 4 (magic) + 32 (pub) + 16 (tag)
constexpr size_t FRAME_HEADER_SIZE = 12; // 4 (length) + 8 (seq)
constexpr size_t MAX_FRAME_SIZE = 128 * 1024 * 1024; // 128 MB

constexpr uint8_t FRAME_TYPE_JSON_LEGACY = 0x7B; // '{'
constexpr uint8_t FRAME_TYPE_JSON        = 0x01;
constexpr uint8_t FRAME_TYPE_BIN_REQ     = 0x02;
constexpr uint8_t FRAME_TYPE_BIN_RESP    = 0x03;

struct SessionKeys {
    uint8_t client_tx_key[32];
    uint8_t server_tx_key[32];
    uint64_t client_seq{0};
    uint64_t server_seq{0};

    void clear() noexcept {
        crypto::secure_zero(client_tx_key, sizeof(client_tx_key));
        crypto::secure_zero(server_tx_key, sizeof(server_tx_key));
        client_seq = 0;
        server_seq = 0;
    }
};

struct Request {
    uint32_t id{0};
    std::string method{"GET"};
    std::string path{"/"};
    std::string headers{};
    std::string body{};
    std::vector<uint8_t> raw_body{}; // Direct zero-copy binary body
    bool is_binary{false};
};

struct Response {
    uint32_t id{0};
    uint16_t status_code{200};
    std::string status_message{"OK"};
    std::string headers{};
    std::string body{};
    std::vector<uint8_t> raw_body{}; // Direct zero-copy binary body
    bool is_binary{false};
};

class Protocol {
public:
    // Построение ClientHello пакета
    static NetResult create_client_hello(
        const uint8_t server_static_pub[32],
        const uint8_t client_ephemeral_priv[32],
        const uint8_t client_ephemeral_pub[32],
        uint8_t out_hello[CLIENT_HELLO_SIZE]
    ) noexcept;

    // Верификация ServerHello и деривация ключей сессии на клиенте
    static NetResult client_process_server_hello(
        const uint8_t client_hello[CLIENT_HELLO_SIZE],
        const uint8_t server_hello[SERVER_HELLO_SIZE],
        const uint8_t server_static_pub[32],
        const uint8_t client_ephemeral_priv[32],
        SessionKeys* out_session
    ) noexcept;

    // Упаковка и шифрование кадра (Client -> Server)
    static NetResult encrypt_frame(
        SessionKeys* session,
        bool is_client,
        const uint8_t* payload,
        size_t payload_len,
        std::vector<uint8_t>& out_packet
    ) noexcept;

    // Расшифровка и проверка подлинности кадра
    static NetResult decrypt_frame(
        SessionKeys* session,
        bool is_client,
        uint64_t seq_num,
        const uint8_t* ciphertext_and_tag,
        size_t data_len,
        std::vector<uint8_t>& out_payload
    ) noexcept;

    // Высокопроизводительная сериализация запроса (Binary или JSON)
    static void serialize_request(const Request& req, std::vector<uint8_t>& out_bytes) noexcept;

    // Высокопроизводительная десериализация ответа (Binary или JSON)
    static bool deserialize_response(const uint8_t* data, size_t len, Response* out_resp) noexcept;
};

} // namespace secure_proto
