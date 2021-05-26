#include "composite_message.h"

#include <string.h>

#define CM_INT8     0x1
#define CM_UINT8    0x2
#define CM_INT16    0x3
#define CM_UINT16   0x4
#define CM_INT32    0x5
#define CM_UINT32   0x6
#define CM_INT64    0x7
#define CM_UINT64   0x8
#define CM_BOOL     0x9
#define CM_CHAR     0xA
#define CM_FLOAT    0xB
#define CM_DOUBLE   0xC

#define CM_VERSION   0x83

#define ENDIAN_MARK     0x0709
#define ENDIAN_INV_MARK 0x0907

static CompositeMessageWriter staticWriter;
static CompositeMessageReader staticReader;


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

CompositeMessageWriter *cmGetStaticWriter(void *buffer, uint32_t size) {
    staticWriter.buffer = (uint8_t *) buffer;
    staticWriter.bufferSize = size;
    staticWriter.usedSize = 0;
    staticWriter.firstError = CM_ERROR_NONE;
    if (size < 2) {
        staticWriter.firstError = CM_ERROR_NO_SPACE;
    } else {
        *(uint16_t *) staticWriter.buffer = ENDIAN_MARK;
        staticWriter.usedSize = 2;
    }
    return &staticWriter;
}

CompositeMessageReader *cmGetStaticReader(const void *message, uint32_t size) {
    const uint8_t *m = (const uint8_t *) message;
    staticReader.message = m;
    staticReader.totalSize = size;
    staticReader.readSize = 0;
    staticReader.firstError = CM_ERROR_NONE;
    staticReader.endianMatch = true;
    uint16_t e = *(uint16_t *) m;
    if (size < 2 || (e != ENDIAN_MARK && e != ENDIAN_INV_MARK)) {
        staticReader.firstError = CM_ERROR_NO_ENDIAN;
    } else {
        staticReader.endianMatch = e == ENDIAN_MARK;
        staticReader.readSize = 2;
    }
    return &staticReader;
}

void cmWriteI8(CompositeMessageWriter *writer, int8_t i) {
    if (!ensureSpace(writer, 2))
        return;

    writer->buffer[writer->usedSize] = CM_INT8;
    *(int8_t *) &writer->buffer[writer->usedSize + 1] = i;

    writer->usedSize += 2;
}

int8_t cmReadI8(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_INT8, 1))
        return 0;

    int8_t i = *(int8_t *) &reader->message[reader->readSize + 1];

    reader->readSize += 2;
    return i;
}

void cmWriteU8(CompositeMessageWriter *writer, uint8_t i) {
    if (!ensureSpace(writer, 2))
        return;

    writer->buffer[writer->usedSize] = CM_UINT8;
    writer->buffer[writer->usedSize + 1] = i;

    writer->usedSize += 2;
}

uint8_t cmReadU8(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_UINT8, 1))
        return 0;

    uint8_t i = reader->message[reader->readSize + 1];

    reader->readSize += 2;
    return i;
}

void cmWriteI16(CompositeMessageWriter *writer, int16_t i) {
    if (!ensureSpace(writer, 1 + sizeof(int16_t)))
        return;

    writer->buffer[writer->usedSize] = CM_INT16;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(int16_t));
}

int16_t cmReadI16(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_INT16, sizeof(int16_t)))
        return 0;

    int16_t i = *(int16_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(int16_t));

    reader->readSize += 1 + sizeof(int16_t);
    return i;
}

void cmWriteU16(CompositeMessageWriter *writer, uint16_t i) {
    if (!ensureSpace(writer, 1 + sizeof(uint16_t)))
        return;

    writer->buffer[writer->usedSize] = CM_UINT16;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(uint16_t));
}

uint16_t cmReadU16(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_UINT16, sizeof(uint16_t)))
        return 0;

    uint16_t i = *(uint16_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(uint16_t));

    reader->readSize += 1 + sizeof(uint16_t);
    return i;
}

void cmWriteI32(CompositeMessageWriter *writer, int32_t i) {
    if (!ensureSpace(writer, 1 + sizeof(int32_t)))
        return;

    writer->buffer[writer->usedSize] = CM_INT32;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(int32_t));
}

int32_t cmReadI32(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_INT32, sizeof(int32_t)))
        return 0;

    int32_t i = *(int32_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(int32_t));

    reader->readSize += 1 + sizeof(int32_t);
    return i;
}

void cmWriteU32(CompositeMessageWriter *writer, uint32_t i) {
    if (!ensureSpace(writer, 1 + sizeof(uint32_t)))
        return;

    writer->buffer[writer->usedSize] = CM_UINT32;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(uint32_t));
}

uint32_t cmReadU32(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_UINT32, sizeof(uint32_t)))
        return 0;

    uint32_t i = *(uint32_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(uint32_t));

    reader->readSize += 1 + sizeof(uint32_t);
    return i;
}

void cmWriteI64(CompositeMessageWriter *writer, int64_t i) {
    if (!ensureSpace(writer, 1 + sizeof(int64_t)))
        return;

    writer->buffer[writer->usedSize] = CM_INT64;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(int64_t));
}

int64_t cmReadI64(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_INT64, sizeof(int64_t)))
        return 0;

    int64_t i = *(int64_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(int64_t));

    reader->readSize += 1 + sizeof(int64_t);
    return i;
}


void cmWriteU64(CompositeMessageWriter *writer, uint64_t i) {
    if (!ensureSpace(writer, 1 + sizeof(uint64_t)))
        return;

    writer->buffer[writer->usedSize] = CM_UINT64;
    writer->usedSize++;
    writeBytes(writer, &i, sizeof(uint64_t));
}

uint64_t cmReadU64(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_UINT64, sizeof(uint64_t)))
        return 0;

    uint64_t i = *(uint64_t *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(uint64_t));

    reader->readSize += 1 + sizeof(uint64_t);
    return i;
}


void cmWriteF(CompositeMessageWriter *writer, float f) {
    if (!ensureSpace(writer, 1 + sizeof(float)))
        return;

    writer->buffer[writer->usedSize] = CM_FLOAT;
    writer->usedSize++;
    writeBytes(writer, &f, sizeof(float));
}

float cmReadF(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_FLOAT, sizeof(float)))
        return 0.f;

    float f = *(float *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&f, sizeof(float));

    reader->readSize += 1 + sizeof(float);
    return f;
}

void cmWriteD(CompositeMessageWriter *writer, double d) {
    if (!ensureSpace(writer, 1 + sizeof(double)))
        return;

    writer->buffer[writer->usedSize] = CM_DOUBLE;
    writer->usedSize++;
    writeBytes(writer, &d, sizeof(double));
}

double cmReadD(CompositeMessageReader *reader) {
    if (!checkValue(reader, CM_DOUBLE, sizeof(double)))
        return 0.0;

    double d = *(double *) &reader->message[reader->readSize + 1];

    if (!reader->endianMatch)
        inverseByteOrder(&d, sizeof(double));

    reader->readSize += 1 + sizeof(double);
    return d;
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

    if (!reader->endianMatch)
        inverseByteOrder(&i, sizeof(uint32_t));

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