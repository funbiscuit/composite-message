/**
 * Create and parse messages that hold various data.
 * Effectively a serialization/deserialization library
 *
 * Each message begins with endianness mark of 2 bytes:
 * 0x07 0x09 indicates big-endian, 0x09 0x07 indicates low-endian
 * Then client data is written, each is described by flag
 *
 * Primitive types are represented by 5 bits:
 * - ABCXX      XX represent type length (2^XX bytes), ABC represent type
 *              001 - unsigned int, 010 - signed int, 011 - float/double
 *              100 - bool, 101 - char:
 * - 00100      uint8
 * - 01000      int8
 * - 00101      uint16
 * - 01001      int16
 * - 00110      uint32
 * - 01010      int32
 * - 00111      uint64
 * - 01011      int64
 * - 01110      float
 * - 01111      double
 * - 10000      bool (1 byte)
 * - 10100      char (1 byte)
 *
 * Flags:
 * - 0000 0000  End of message
 * - 000X XXXX  Value of primitive type (first 5 bits represent type)
 * - 001X XXXX  Primitive type without value (null equivalent)
 * - 010X XXXX  Array of primitive types (first 5 bits represent type)
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
#define CM_ERROR_INVALID_ARG 4

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
    uint8_t *message;

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
} CompositeMessageReader;

/**
 * Initialize message writer with given buffer and size
 * @param buffer - pointer to buffer for message building
 * @size size - size of buffer in bytes
 * @return initialized CompositeMessageWriter
 */
CompositeMessageWriter cmGetWriter(void *buffer, uint32_t size);

/**
 * Initialize message reader with given message and size
 * Message must be non const since it will be modified internally
 * @param message - pointer to message that should be read
 * @size size - size of message in bytes
 * @return initialized CompositeMessageReader
 */
CompositeMessageReader cmGetReader(void *message, uint32_t size);

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

void cmWriteBool(CompositeMessageWriter *writer, bool val);

bool cmReadBool(CompositeMessageReader *reader);

void cmWriteChar(CompositeMessageWriter *writer, char val);

char cmReadChar(CompositeMessageReader *reader);

/**
 * Write array of values. Each value can be up to 16 bytes long
 * If buffer can't hold array of this size
 * firstError is set to CM_ERROR_NO_SPACE
 * itemSize must be power of two (maximum 8) otherwise
 * firstError is set to CM_ERROR_INVALID_ARG
 * @param writer
 * @param data
 * @param itemCount how many items are in data array
 * @param itemSize size of each item (maximum 16 bytes)
 */
void cmWriteArray(CompositeMessageWriter *writer,
                  const void *data, uint32_t itemCount, uint8_t itemSize);

/**
 * Read array of values into provided buffer. Each value can be up to 16 bytes
 * If next element is not an array, firstError is set to CM_ERROR_NO_VALUE
 * If provided buffer is insufficient to store array, firstError
 * is set to CM_ERROR_NO_SPACE
 * If itemSize is 0 or greater than 16, firstError is set to CM_ERROR_INVALID_ARG
 * If actual array in message has different item size, firstError
 * is set to CM_ERROR_INVALID_ARG
 * If read is successful, size of read array is returned
 * @param reader
 * @param buffer
 * @param maxItems how many items buffer can store
 * @param itemSize size of each item (maximum 16 bytes)
 * @return
 */
uint32_t cmReadArray(CompositeMessageReader *reader,
                     void *buffer, uint32_t maxItems, uint8_t itemSize);

/**
 * Read size of next array. This function doesn't change state of
 * reader if next element is array so it can be called multiple times.
 * If next element is not array, firstError is set to CM_ERROR_NO_VALUE
 * @param writer
 * @return
 */
uint32_t cmPeekArraySize(CompositeMessageReader *reader);

void cmWriteVersion(CompositeMessageWriter *writer, uint32_t ver);

uint32_t cmReadVersion(CompositeMessageReader *reader);

#ifdef __cplusplus
}
#endif

#endif //COMPOSITE_MESSAGE_H
