#ifndef CRC32_H_
#define CRC32_H_

class Crc32 {
protected:
    unsigned long crc;
public:
    Crc32();
    void update(const uint8_t data);
    void update(const uint16_t data);
    void update(const uint32_t data);
    void update(const uint64_t data);
    void update(const char *s);
    void update(const uint8_t *buf, const int length);
    unsigned long get();
};

#endif /* CRC32_H_ */
