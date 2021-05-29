#include "composite_message.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <vector>
#include <cfloat>

SCENARIO("Reader and writer creation", "[create]") {
    GIVEN("Non empty buffer") {
        std::vector<uint8_t> buffer(2);
        uint16_t i = 0x0709;
        auto *e = (uint8_t *) &i;

        WHEN("Reader is created from message without endianness") {
            auto reader = cmGetReader(buffer.data(), buffer.size());

            THEN("Endianness errors") {
                REQUIRE(reader.firstError == CM_ERROR_NO_ENDIAN);
            }
        }

        WHEN("Reader is created") {
            buffer[0] = e[0];
            buffer[1] = e[1];

            auto reader = cmGetReader(buffer.data(), buffer.size());

            THEN("No errors") {
                REQUIRE(reader.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read size is increased") {
                REQUIRE(reader.readSize == 2);
            }
        }

        WHEN("Writer is created") {
            auto writer = cmGetWriter(buffer.data(), buffer.size());

            THEN("No errors") {
                REQUIRE(writer.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Endianness is written to first 2 bytes") {
                REQUIRE(writer.usedSize == 2);
                REQUIRE((int) e[0] == (int) buffer[0]);
                REQUIRE((int) e[1] == (int) buffer[1]);
            }
        }
    }
}

SCENARIO("Write message", "[write]") {
    GIVEN("Writer with enough space") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetWriter(buffer.data(), buffer.size());

        WHEN("Int8 is written") {
            int8_t i = GENERATE(-128, 5, 127);
            cmWriteI8(&writer, i);

            THEN("No errors") {
                REQUIRE(writer.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Used size is increased") {
                REQUIRE(writer.usedSize == 4);
            }

            AND_THEN("Int8 is placed after 3rd byte") {
                REQUIRE(((int8_t *) buffer.data())[3] == i);
            }
        }

        WHEN("UInt8 is written") {
            uint8_t i = GENERATE(0, 127, 255);
            cmWriteU8(&writer, i);

            THEN("UInt8 is placed after 3rd byte") {
                REQUIRE(buffer[3] == i);
            }
        }

        WHEN("Int32 is written") {
            int32_t i = GENERATE(INT32_MIN, 0, INT32_MAX);
            cmWriteI32(&writer, i);

            THEN("Int32 is placed after 3rd byte") {
                REQUIRE(*(int32_t *) &buffer[3] == i);
            }
        }
    }

    GIVEN("Writer with not enough space") {
        std::vector<uint8_t> buffer(3);
        auto writer = cmGetWriter(buffer.data(), buffer.size());

        WHEN("Int8 is written") {
            cmWriteI8(&writer, 15);

            THEN("No space error") {
                REQUIRE(writer.firstError == CM_ERROR_NO_SPACE);
            }
        }
    }
}

SCENARIO("Read message", "[read]") {
    std::vector<uint8_t> buffer(1024);
    auto writer = cmGetWriter(buffer.data(), buffer.size());

    GIVEN("Non-empty message with int8") {
        int8_t i = GENERATE(-128, 5, 127);
        cmWriteI8(&writer, i);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Int8 is read") {
            auto r = cmReadI8(&reader);

            THEN("No errors") {
                REQUIRE(reader.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read size is increased") {
                REQUIRE(reader.readSize == 4);
            }

            AND_THEN("Read int8 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Non-empty message with different integers") {
        int8_t i1 = GENERATE(INT8_MIN, INT8_MAX);
        uint8_t i2 = UINT8_MAX;
        int16_t i3 = GENERATE(INT16_MIN, INT16_MAX);
        uint16_t i4 = UINT16_MAX;
        int32_t i5 = GENERATE(INT32_MIN, INT32_MAX);
        uint32_t i6 = UINT16_MAX;
        int64_t i7 = GENERATE(INT64_MIN, INT64_MAX);
        uint64_t i8 = UINT64_MAX;
        cmWriteI8(&writer, i1);
        cmWriteU8(&writer, i2);
        cmWriteI16(&writer, i3);
        cmWriteU16(&writer, i4);
        cmWriteI32(&writer, i5);
        cmWriteU32(&writer, i6);
        cmWriteI64(&writer, i7);
        cmWriteU64(&writer, i8);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Values are read") {
            auto r1 = cmReadI8(&reader);
            auto r2 = cmReadU8(&reader);
            auto r3 = cmReadI16(&reader);
            auto r4 = cmReadU16(&reader);
            auto r5 = cmReadI32(&reader);
            auto r6 = cmReadU32(&reader);
            auto r7 = cmReadI64(&reader);
            auto r8 = cmReadU64(&reader);

            THEN("No errors") {
                REQUIRE(reader.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read values are correct") {
                REQUIRE(i1 == r1);
                REQUIRE(i2 == r2);
                REQUIRE(i3 == r3);
                REQUIRE(i4 == r4);
                REQUIRE(i5 == r5);
                REQUIRE(i6 == r6);
                REQUIRE(i7 == r7);
                REQUIRE(i8 == r8);
            }
        }
    }

    GIVEN("Non-empty message with float/double value") {
        float f = GENERATE(FLT_MIN, 1.f, FLT_MAX);
        double d = GENERATE(DBL_MIN, 1.0, DBL_MAX);
        cmWriteF(&writer, f);
        cmWriteD(&writer, d);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Float and double values are read") {
            auto r1 = cmReadF(&reader);
            auto r2 = cmReadD(&reader);

            THEN("Read values are correct") {
                REQUIRE(f == r1);
                REQUIRE(d == r2);
            }
        }
    }

    GIVEN("Message with arrays") {
        std::vector<uint8_t> dataU8{0, 123, 17, 255};
        std::vector<uint32_t> dataU32{0, 123, 17, UINT32_MAX, 234};
        std::vector<uint64_t> dataU64{0, 11, 31, UINT64_MAX, 234, 57};

        cmWriteArray(&writer, dataU8.data(), dataU8.size(), sizeof(uint8_t));
        cmWriteArray(&writer, dataU32.data(), dataU32.size(), sizeof(uint32_t));
        cmWriteArray(&writer, dataU64.data(), dataU64.size(), sizeof(uint64_t));

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Array size is peeked") {
            auto size = cmPeekArraySize(&reader);

            THEN("Size is correct") {
                REQUIRE(size == dataU8.size());
            }

            AND_THEN("Size can be peeked again") {
                size = cmPeekArraySize(&reader);
                REQUIRE(size == dataU8.size());
            }
        }

        WHEN("Arrays are read") {
            std::vector<uint8_t> readU8(32);
            std::vector<uint32_t> readU32(32);
            std::vector<uint64_t> readU64(32);

            auto size = cmReadArray(&reader, readU8.data(), readU8.size(), sizeof(uint8_t));
            readU8.resize(size);
            size = cmReadArray(&reader, readU32.data(), readU32.size(), sizeof(uint32_t));
            readU32.resize(size);
            size = cmReadArray(&reader, readU64.data(), readU64.size(), sizeof(uint64_t));
            readU64.resize(size);

            THEN("No errors") {
                REQUIRE(reader.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read arrays are correct") {
                REQUIRE_THAT(readU8, Catch::Matchers::Equals(dataU8));
                REQUIRE_THAT(readU32, Catch::Matchers::Equals(dataU32));
                REQUIRE_THAT(readU64, Catch::Matchers::Equals(readU64));
            }
        }
    }

    GIVEN("Message with array and inverse endianness") {
        std::vector<uint32_t> dataU32{0, 123, 17, 76234, 2349843723};
        std::vector<uint32_t> data2U32{78, 547, 879, 789674232};
        std::vector<uint32_t> dataInverseU32 = dataU32;
        std::vector<uint32_t> data2InverseU32 = data2U32;
        // change endianness of data
        for (auto &i : dataInverseU32) {
            auto *d = (uint8_t *) &i;
            std::swap(d[0], d[3]);
            std::swap(d[1], d[2]);
        }
        for (auto &i : data2InverseU32) {
            auto *d = (uint8_t *) &i;
            std::swap(d[0], d[3]);
            std::swap(d[1], d[2]);
        }

        cmWriteArray(&writer, dataInverseU32.data(), dataInverseU32.size(), sizeof(uint32_t));
        cmWriteArray(&writer, data2InverseU32.data(), data2InverseU32.size(), sizeof(uint32_t));
        cmWriteArray(&writer, dataInverseU32.data(), dataInverseU32.size(), sizeof(uint32_t));

        std::swap(writer.buffer[0], writer.buffer[1]);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Arrays is read") {
            std::vector<uint32_t> readU32(32);
            std::vector<uint32_t> read2U32(32);
            std::vector<uint32_t> read3U32(32);

            auto size = cmReadArray(&reader, readU32.data(), readU32.size(), sizeof(uint32_t));
            readU32.resize(size);
            size = cmReadArray(&reader, read2U32.data(), read2U32.size(), sizeof(uint32_t));
            read2U32.resize(size);
            size = cmReadArray(&reader, read3U32.data(), read3U32.size(), sizeof(uint32_t));
            read3U32.resize(size);

            THEN("Read arrays are correct") {
                REQUIRE_THAT(readU32, Catch::Matchers::Equals(dataU32));
                REQUIRE_THAT(read2U32, Catch::Matchers::Equals(data2U32));
                REQUIRE_THAT(read3U32, Catch::Matchers::Equals(dataU32));
            }
        }
    }


    GIVEN("Non-empty message with int32 in inverse endian mode") {
        int32_t i = GENERATE(INT32_MIN, 0, INT32_MAX);
        cmWriteI32(&writer, i);
        std::swap(buffer[0], buffer[1]);
        std::swap(buffer[3], buffer[6]);
        std::swap(buffer[4], buffer[5]);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Int32 is read") {
            auto r = cmReadI32(&reader);

            THEN("No errors") {
                REQUIRE(reader.firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read int32 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Message without data") {
        buffer.resize(2);
        writer = cmGetWriter(buffer.data(), buffer.size());
        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Int8 is read") {
            cmReadI8(&reader);

            THEN("No value error") {
                REQUIRE(reader.firstError == CM_ERROR_NO_VALUE);
            }
        }
    }
}

SCENARIO("Read message with extras", "[read]") {

    GIVEN("Non-empty message with version") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetWriter(buffer.data(), buffer.size());

        uint32_t ver = GENERATE(157, 157157, UINT32_MAX);
        cmWriteVersion(&writer, ver);

        auto reader = cmGetReader(writer.buffer, writer.usedSize);

        WHEN("Version is read") {
            auto r = cmReadVersion(&reader);

            THEN("Read value is correct") {
                REQUIRE(r == ver);
            }
        }
    }
}
