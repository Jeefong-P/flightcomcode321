#ifndef LORA_CODEC_H
#define LORA_CODEC_H

//#include <random>
//#include <cstring>

struct packet {
    float altitude;  // in meters
    float latitude;  // in degrees
    float longitude; // in degrees
    uint8_t flightState;
};

// Encode packet into a raw buffer
inline void encode_packet(const packet& pkt, char* buffer) {
    std::memcpy(buffer, &pkt, sizeof(packet));
}

// Decode packet from a raw buffer
inline void decode_packet(const char* buffer, packet& pkt) {
    std::memcpy(&pkt, buffer, sizeof(packet));
}

// Generate a dummy packet with random values
//inline packet dummy_packet() {
//    static std::mt19937 gen(12345); // replace 12345 with millis, tick, etc.
//
//    std::uniform_real_distribution<> alt_dist(0.0f, 10000.0f);
//    std::uniform_real_distribution<> lat_dist(-90.0f, 90.0f);
//    std::uniform_real_distribution<> lon_dist(-180.0f, 180.0f);
//
//    return {
//        (float)alt_dist(gen),
//        (float)lat_dist(gen),
//        (float)lon_dist(gen)
//    };
//}


#endif // LORA_CODEC_H
