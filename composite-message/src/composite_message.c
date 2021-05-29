#include "composite_message.h"

#include <string.h>

#define CM_TYPE_MASK     0x1Cu
#define CM_TYPE_LEN_MASK 0x03u

#define CM_ARRAY    0x40u

#define CM_END_OF_MESSAGE   0x00u

#define CM_VERSION   0x83u

#define ENDIAN_MARK     0x0709u
#define ENDIAN_INV_MARK 0x0907u

/**
 * Ensures that current writer has enough space to write 'size' bytes
 * @param writer
 * @param size
 * @return true if there is enough space
 */
static bool ensureSpace(CompositeMessageWriter *writer, uint32_t size);

/**
 * Check if there is value of specified type and size at current position
 * @param type
 * @param size
 * @return true if value is present
 */
static bool checkValue(CompositeMessageReader *reader,
                       uint8_t type, uint8_t size);

/**
 * Write single value
 * @param writer
 * @param val pointer to value
 * @param len size of value type in bytes
 * @param type type of value as defined by CM_TYPE_*
 * @return
 */
static bool writeValue(CompositeMessageWriter *writer, void *val, uint8_t len,
                       uint8_t type);

/**
 * Read single value
 * @param reader
 * @param val pointer where value should be stored
 * @param len size of value type in bytes
 * @param type expected type of value as defined by CM_TYPE_*
 * @return
 */
static bool readValue(CompositeMessageReader *reader, void *val, uint8_t len,
                      uint8_t type);

/**
 * Get flag of primitive type that is defined by type (CM_TYPE_*) and length in
 * bytes
 * @param type
 * @param len
 * @return type flag or 0 if length is invalid
 */
static uint8_t getTypeFlag(uint8_t type, uint8_t len);

/**
 * Split provided flag into type of primitive and its length in bytes
 * @param flag
 * @param type
 * @param len
 */
static void splitTypeFlag(uint8_t flag, uint8_t *type, uint8_t *len);

/**
 * Write array of bytes directly into message
 * Writer must have enough space in its internal buffer
 * @param writer
 * @param data
 * @param size
 */
static bool writeBytes(CompositeMessageWriter *writer,
                       const void *data, uint32_t size);

/**
 * Inverse byte order in given array
 * @param data
 * @param size
 */
static void inverseByteOrder(void *data, uint8_t size);

/**
 * Convert endianness in whole message
 * @param message
 * @param size
 * @return true if conversion was successful
 */
static bool convertEndianness(void *message, uint32_t size);

static bool isSingleValue(uint8_t flag);

static bool isArray(uint8_t flag);

static bool isVersion(uint8_t flag);

CompositeMessageWriter cmGetWriter(void *buffer, uint32_t size) {
    static CompositeMessageWriter writer;
    writer.buffer = (uint8_t *) buffer;
    writer.bufferSize = size;
    writer.usedSize = 0;
    writer.firstError = CM_ERROR_NONE;
    if (size < 2) {
        writer.firstError = CM_ERROR_NO_SPACE;
    } else {
        *(uint16_t *) writer.buffer = ENDIAN_MARK;
        writer.usedSize = 2;
    }
    return writer;
}

CompositeMessageReader cmGetReader(void *message, uint32_t size) {
    static CompositeMessageReader reader;
    uint8_t *m = (uint8_t *) message;
    reader.message = m;
    reader.totalSize = size;
    reader.readSize = 0;
    reader.firstError = CM_ERROR_NONE;
    uint16_t e = *(uint16_t *) m;
    if (size < 2 || (e != ENDIAN_MARK && e != ENDIAN_INV_MARK)) {
        reader.firstError = CM_ERROR_NO_ENDIAN;
    } else {
        // in case of inversed endianness, swap all groups of bytes
        // so message can be processed in normal way
        if (e != ENDIAN_MARK && !convertEndianness(&m[2], size - 2)) {
            reader.firstError = CM_ERROR_NO_ENDIAN;
        } else {
            reader.readSize = 2;
        }
    }
    return reader;
}

void cmWriteI8(CompositeMessageWriter *writer, int8_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_INT);
}

int8_t cmReadI8(CompositeMessageReader *reader) {
    int8_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_INT);
    return val;
}

void cmWriteU8(CompositeMessageWriter *writer, uint8_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_UINT);
}

uint8_t cmReadU8(CompositeMessageReader *reader) {
    uint8_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_UINT);
    return val;
}

void cmWriteI16(CompositeMessageWriter *writer, int16_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_INT);
}

int16_t cmReadI16(CompositeMessageReader *reader) {
    int16_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_INT);
    return val;
}

void cmWriteU16(CompositeMessageWriter *writer, uint16_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_UINT);
}

uint16_t cmReadU16(CompositeMessageReader *reader) {
    uint16_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_UINT);
    return val;
}

void cmWriteI32(CompositeMessageWriter *writer, int32_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_INT);
}

int32_t cmReadI32(CompositeMessageReader *reader) {
    int32_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_INT);
    return val;
}

void cmWriteU32(CompositeMessageWriter *writer, uint32_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_UINT);
}

uint32_t cmReadU32(CompositeMessageReader *reader) {
    uint32_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_UINT);
    return val;
}

void cmWriteI64(CompositeMessageWriter *writer, int64_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_INT);
}

int64_t cmReadI64(CompositeMessageReader *reader) {
    int64_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_INT);
    return val;
}


void cmWriteU64(CompositeMessageWriter *writer, uint64_t i) {
    writeValue(writer, &i, sizeof(i), CM_TYPE_UINT);
}

uint64_t cmReadU64(CompositeMessageReader *reader) {
    uint64_t val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_UINT);
    return val;
}


void cmWriteF(CompositeMessageWriter *writer, float f) {
    writeValue(writer, &f, sizeof(f), CM_TYPE_FLOAT);
}

float cmReadF(CompositeMessageReader *reader) {
    float val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_FLOAT);
    return val;
}

void cmWriteD(CompositeMessageWriter *writer, double d) {
    writeValue(writer, &d, sizeof(d), CM_TYPE_FLOAT);
}

double cmReadD(CompositeMessageReader *reader) {
    double val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_FLOAT);
    return val;
}

void cmWriteBool(CompositeMessageWriter *writer, bool val) {
    writeValue(writer, &val, sizeof(val), CM_TYPE_BOOL);
}

bool cmReadBool(CompositeMessageReader *reader) {
    bool val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_BOOL);
    return val;
}

void cmWriteChar(CompositeMessageWriter *writer, char val) {
    writeValue(writer, &val, sizeof(val), CM_TYPE_CHAR);
}

char cmReadChar(CompositeMessageReader *reader) {
    char val = 0;
    readValue(reader, &val, sizeof(val), CM_TYPE_CHAR);
    return val;
}

void cmWriteTypedArray(CompositeMessageWriter *writer, uint8_t itemType,
                       uint8_t itemSize, const void *data, uint32_t itemCount) {
    if (itemSize > 0x08 || itemSize == 0) {
        writer->firstError = CM_ERROR_INVALID_ARG;
        return;
    }
    if (itemType > 0x14 || itemType < 0x04) {
        writer->firstError = CM_ERROR_INVALID_ARG;
        return;
    }
    uint8_t flag = getTypeFlag(CM_ARRAY | itemType, itemSize);

    // increase number of stored items to store extra null terminator
    if (itemType == CM_TYPE_CHAR) {
        ++itemCount;
    }

    // we need to place flag (1 byte), array size (uint32) and array itself
    if (!ensureSpace(writer, 1 + sizeof(uint32_t) + itemSize * itemCount))
        return;

    writer->buffer[writer->usedSize] = flag;
    writer->usedSize++;
    writeBytes(writer, &itemCount, sizeof(uint32_t));
    if (itemType == CM_TYPE_CHAR) {
        writeBytes(writer, data, (itemCount - 1) * itemSize);
        writer->buffer[writer->usedSize] = 0x00;
        ++writer->usedSize;
    } else {
        writeBytes(writer, data, itemCount * itemSize);
    }
}

uint32_t cmReadTypedArray(CompositeMessageReader *reader, uint8_t itemType,
                          uint8_t itemSize, void *buffer, uint32_t maxItems) {
    if (itemSize > 0x08 || itemSize == 0) {
        reader->firstError = CM_ERROR_INVALID_ARG;
        return 0;
    }
    if (itemType > 0x14 || itemType < 0x04) {
        reader->firstError = CM_ERROR_INVALID_ARG;
        return 0;
    }
    uint8_t flag = getTypeFlag(CM_ARRAY | itemType, itemSize);

    uint32_t arraySize = cmPeekArraySize(reader);

    if (reader->firstError != CM_ERROR_NONE) {
        return 0;
    }

    if (reader->message[reader->readSize] != flag) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return 0;
    }

    if (maxItems < arraySize) {
        reader->firstError = CM_ERROR_NO_SPACE;
        return 0;
    }

    reader->readSize += 1 + sizeof(uint32_t);
    memcpy(buffer, &reader->message[reader->readSize], arraySize * itemSize);
    reader->readSize += arraySize * itemSize;

    return arraySize;
}

uint32_t cmPeekArraySize(CompositeMessageReader *reader) {
    if (reader->firstError != CM_ERROR_NONE)
        return 0;

    if (reader->readSize + 1 + sizeof(uint32_t) > reader->totalSize ||
        !(reader->message[reader->readSize] & CM_ARRAY)) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return 0;
    }

    uint32_t size = *(uint32_t *) &reader->message[reader->readSize + 1];

    return size;
}

uint32_t cmPeekStringLength(CompositeMessageReader *reader) {
    uint32_t len = cmPeekArraySize(reader);
    if (reader->firstError != CM_ERROR_NONE)
        return 0;

    uint8_t typeStored;
    splitTypeFlag(reader->message[reader->readSize], &typeStored, NULL);
    if (typeStored != CM_TYPE_CHAR || len == 0) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return 0;
    }

    return len - 1;
}

void cmWriteVersion(CompositeMessageWriter *writer, uint32_t ver) {
    cmWriteU32(writer, ver);

    if (writer->firstError == CM_ERROR_NONE) {
        writer->buffer[writer->usedSize - sizeof(uint32_t) - 1] = CM_VERSION;
    }
}

uint32_t cmReadVersion(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_VERSION, sizeof(uint32_t)))
        return 0;

    uint32_t i = *(uint32_t *) &reader->message[reader->readSize + 1];

    reader->readSize += 1 + sizeof(uint32_t);
    return i;
}

static bool ensureSpace(CompositeMessageWriter *writer, uint32_t size) {
    if (writer->firstError != CM_ERROR_NONE)
        return false;

    if (writer->bufferSize - writer->usedSize < size) {
        writer->firstError = CM_ERROR_NO_SPACE;
        return false;
    }

    return true;
}

static bool checkValue(CompositeMessageReader *reader,
                       uint8_t type, uint8_t size) {
    if (reader->firstError != CM_ERROR_NONE)
        return false;

    if (reader->readSize + 1 + size > reader->totalSize ||
        reader->message[reader->readSize] != type) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return false;
    }

    return true;
}

static bool writeValue(CompositeMessageWriter *writer, void *val, uint8_t len,
                       uint8_t type) {
    if (writer->firstError != CM_ERROR_NONE)
        return false;
    if (!ensureSpace(writer, 1 + len))
        return false;
    uint8_t flag = getTypeFlag(type, len);
    if (flag == 0) {
        writer->firstError = CM_ERROR_INVALID_ARG;
        return false;
    }

    writer->buffer[writer->usedSize] = flag;
    writer->usedSize++;
    writeBytes(writer, val, len);
    return true;
}

static bool readValue(CompositeMessageReader *reader, void *val, uint8_t len,
                      uint8_t type) {
    if (reader->firstError != CM_ERROR_NONE)
        return false;
    if (reader->totalSize - reader->readSize < len) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return false;
    }
    uint8_t typeStored, lenStored;
    splitTypeFlag(reader->message[reader->readSize], &typeStored, &lenStored);

    if (typeStored != type || len != lenStored) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return false;
    }
    ++reader->readSize;

    memcpy(val, &reader->message[reader->readSize], len);

    reader->readSize += len;
    return true;
}

static uint8_t getTypeFlag(uint8_t type, uint8_t len) {
    uint8_t flag = type;
    if (len == 2) {
        flag |= 0x01u;
    } else if (len == 4) {
        flag |= 0x02u;
    } else if (len == 8) {
        flag |= 0x03u;
    } else if (len != 1) {
        return 0;
    }
    return flag;
}

static void splitTypeFlag(uint8_t flag, uint8_t *type, uint8_t *len) {
    if (len != NULL) {
        *len = 1u << (flag & CM_TYPE_LEN_MASK);
    }
    if (type != NULL) {
        *type = flag & CM_TYPE_MASK;
    }
}

static bool writeBytes(CompositeMessageWriter *writer,
                       const void *data, uint32_t size) {
    if (!ensureSpace(writer, size))
        return false;

    memcpy(&writer->buffer[writer->usedSize], data, size);
    writer->usedSize += size;
    return true;
}

static void inverseByteOrder(void *data, uint8_t size) {
    uint8_t *d = (uint8_t *) data;
    for (uint8_t i = 0; i < size / 2; ++i) {
        uint8_t t = d[size - i - 1];
        d[size - i - 1] = d[i];
        d[i] = t;
    }
}

static bool convertEndianness(void *message, uint32_t size) {
    uint8_t *d = (uint8_t *) message;
    while (size > 0) {
        uint8_t flag = *d;

        uint8_t itemLen = 1u << (flag & CM_TYPE_LEN_MASK);
        uint32_t itemCount = 1;

        --size;
        ++d;

        if (isArray(flag)) {
            // length of each item
            inverseByteOrder(d, sizeof(uint32_t));
            itemCount = *(uint32_t *) d;
            d += sizeof(uint32_t);
            size -= sizeof(uint32_t);
        } else if (isVersion(flag)) {
            itemLen = sizeof(uint32_t);
        } else if (!isSingleValue(flag)) {
            // unknown flag
            return false;
        }

        for (int i = 0; i < itemCount; ++i) {
            inverseByteOrder(d, itemLen);
            d += itemLen;
            size -= itemLen;
        }
    }
    return true;
}

static bool isSingleValue(uint8_t flag) {
    return (flag >> 5u) == 0 && flag > 0;
}

static bool isArray(uint8_t flag) {
    return (flag >> 5u) == 2;
}

static bool isVersion(uint8_t flag) {
    return flag == CM_VERSION;
}
