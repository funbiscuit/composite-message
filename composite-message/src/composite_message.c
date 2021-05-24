#include "composite_message.h"

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

#define ENDIAN_MARK     0x0709
#define ENDIAN_INV_MARK 0x0907

static CompositeMessageWriter staticWriter;
static CompositeMessageReader staticReader;

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
    if (writer->firstError != CM_ERROR_NONE)
        return;

    if (writer->bufferSize - writer->usedSize < 2) {
        writer->firstError = CM_ERROR_NO_SPACE;
        return;
    }

    writer->buffer[writer->usedSize] = CM_INT8;
    writer->buffer[writer->usedSize + 1] = i;

    writer->usedSize += 2;
}

int8_t cmReadI8(CompositeMessageReader *reader) {
    if (reader->firstError != CM_ERROR_NONE)
        return 0;

    if (reader->readSize + 2 > reader->totalSize ||
        reader->message[reader->readSize] != CM_INT8) {
        reader->firstError = CM_ERROR_NO_VALUE;
        return 0;
    }

    uint8_t i = reader->message[reader->readSize + 1];

    reader->readSize += 2;
    return i;
}
