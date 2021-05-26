/**
 * Create and parse messages that hold various data.
 * Effectively a serialization/deserialization library
 *
 * Each message begins with endianness mark of 2 bytes:
 * 0x07 0x09 indicates big-endian, 0x09 0x07 indicates low-endian
 * Then client data is written, each is described by flag
 *
 * Primitive types:
 * - 0x1        int8
 * - 0x2        uint8
 * - 0x3        int16
 * - 0x4        uint16
 * - 0x5        int32
 * - 0x6        uint32
 * - 0x7        int64
 * - 0x8        uint64
 * - 0x9        bool (1 byte)
 * - 0xA        char (1 byte)
 * - 0xB        float
 * - 0xC        double
 *
 * Flags:
 * - 0000 0000  End of message
 * - 0000 XXXX  Value of primitive type
 * - 0001 XXXX  Primitive type without value (null equivalent)
 * - 0010 XXXX  Array of primitive types
 *              Array begins with its size (uint32) followed by elements
 * - 1000 0000  Name of the next value/block.
 *              Name begins with its length (uint8) followed by chars.
 *              At the end 0x00 is placed which doesn't count in name length
 * - 1000 0001  Block start
 * - 1000 0010  Block end
 * - 1000 0011  Protocol version (uint32)
 * - 1000 0100  Protocol marker which is composed of up to 255 bytes
 *              Marker begins with its size (uint8) followed by its bytes
 * - 1000 0101  Protocol metadata start
 * - 1000 0110  Protocol metadata end
 * - 1000 1000  CRC32 hash of everything up to this point (excluding endian
 *              indicator and this flag)
 */
#ifndef COMPOSITE_MESSAGE_H
#define COMPOSITE_MESSAGE_H

#define CM_ERROR_NONE 0
#define CM_ERROR_NO_ENDIAN 1
#define CM_ERROR_NO_SPACE 2
#define CM_ERROR_NO_VALUE 3

#ifdef __cplusplus

extern "C" {
#else

#include <stdbool.h>

#endif

// cstdint is available only since C++11, so use C header instead
#include <stdint.h>

/**
 * After finished writing, check if firstError is CM_ERROR_NONE.
 * This ensures that all written values are correct and message can be
 * used in CompositeMessageReader.
 * When first error is encountered, all other operations on writer
 * are no-op.
 * Do not modify fields directly
 */
typedef struct {
    uint8_t *buffer;
    uint32_t bufferSize;
    uint32_t usedSize;
    uint32_t firstError;
} CompositeMessageWriter;

/**
 * After finished reading, check if firstError is CM_ERROR_NONE.
 * This ensures that all read values are correct.
 * When first error is encountered, all other operations on reader
 * are no-op.
 */
typedef struct {
    const uint8_t *message;

    /**
     * Total number of bytes in message
     */
    uint32_t totalSize;

    /**
     * How much bytes are already read
     */
    uint32_t readSize;

    /**
     * First error occurred when reading bytes
     */
    uint32_t firstError;

    /**
     * Whether endianness of this system matches endianness of message
     */
    bool endianMatch;
} CompositeMessageReader;

/**
 * Initialize message writer with given buffer and size
 * Statically allocated struct is used, so calling this method while
 * previous writer is still in use leads to undefined behavior.
 * You can allocate CompositeMessageWriter yourself and then
 * call cmInitWriter()
 * @param buffer - pointer to buffer for message building
 * @size size - size of buffer in bytes
 * @return pointer to initialized CompositeMessageWriter
 */
CompositeMessageWriter *cmGetStaticWriter(void *buffer, uint32_t size);

/**
 * Initialize message reader with given message and size
 * Statically allocated struct is used, so calling this method while
 * previous reader is still in use leads to undefined behavior.
 * You can allocate CompositeMessageReader yourself and then call
 * cmInitWriter()
 * @param message - pointer to message that should be read
 * @size size - size of message in bytes
 * @return pointer to initialized CompositeMessageReader
 */
CompositeMessageReader *cmGetStaticReader(const void *message, uint32_t size);

/**
 * Write signed 8-bit integer to message. If buffer can't hold integer then
 * firstError is set to CM_ERROR_NO_SPACE
 * @param writer
 * @param i
 */
void cmWriteI8(CompositeMessageWriter *writer, int8_t i);

/**
 * Read signed 8-bit integer from message.
 * If there is no int8 at this place, firstError is set to CM_ERROR_NO_VALUE
 * @param reader
 * @return read value
 */
int8_t cmReadI8(CompositeMessageReader *reader);

void cmWriteU8(CompositeMessageWriter *writer, uint8_t i);

uint8_t cmReadU8(CompositeMessageReader *reader);

void cmWriteI16(CompositeMessageWriter *writer, int16_t i);

int16_t cmReadI16(CompositeMessageReader *reader);

void cmWriteU16(CompositeMessageWriter *writer, uint16_t i);

uint16_t cmReadU16(CompositeMessageReader *reader);

void cmWriteI32(CompositeMessageWriter *writer, int32_t i);

int32_t cmReadI32(CompositeMessageReader *reader);

void cmWriteU32(CompositeMessageWriter *writer, uint32_t i);

uint32_t cmReadU32(CompositeMessageReader *reader);

void cmWriteI64(CompositeMessageWriter *writer, int64_t i);

int64_t cmReadI64(CompositeMessageReader *reader);

void cmWriteU64(CompositeMessageWriter *writer, uint64_t i);

uint64_t cmReadU64(CompositeMessageReader *reader);

void cmWriteF(CompositeMessageWriter *writer, float f);

float cmReadF(CompositeMessageReader *reader);

void cmWriteD(CompositeMessageWriter *writer, double d);

double cmReadD(CompositeMessageReader *reader);

void cmWriteVersion(CompositeMessageWriter *writer, uint32_t ver);

uint32_t cmReadVersion(CompositeMessageReader *writer);

#ifdef __cplusplus
}
#endif

#endif //COMPOSITE_MESSAGE_H
