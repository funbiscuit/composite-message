#include "composite_message.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <vector>

SCENARIO("Reader and writer creation", "[create]") {
    GIVEN("Non empty buffer") {
        std::vector<uint8_t> buffer(2);
        uint16_t i = 0x0709;
        auto *e = (uint8_t *) &i;

        WHEN("Reader is created from message without endianness") {
            auto reader = cmGetStaticReader(buffer.data(), buffer.size());

            THEN("Endianness errors") {
                REQUIRE(reader->firstError == CM_ERROR_NO_ENDIAN);
            }
        }

        WHEN("Reader is created from message with same endianness") {
            buffer[0] = e[0];
            buffer[1] = e[1];

            auto reader = cmGetStaticReader(buffer.data(), buffer.size());

            THEN("No errors") {
                REQUIRE(reader->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read size is increased") {
                REQUIRE(reader->readSize == 2);
            }

            AND_THEN("Endianness is read correctly") {
                REQUIRE(reader->endianMatch);
            }
        }

        WHEN("Reader is created from message with inverted endianness") {
            buffer[0] = e[1];
            buffer[1] = e[0];

            auto reader = cmGetStaticReader(buffer.data(), buffer.size());

            THEN("No errors") {
                REQUIRE(reader->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Endianness is read correctly") {
                REQUIRE_FALSE(reader->endianMatch);
            }
        }

        WHEN("Writer is created") {
            auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

            THEN("No errors") {
                REQUIRE(writer->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Endianness is written to first 2 bytes") {
                REQUIRE(writer->usedSize == 2);
                REQUIRE((int) e[0] == (int) buffer[0]);
                REQUIRE((int) e[1] == (int) buffer[1]);
            }
        }
    }
}

SCENARIO("Write message", "[write]") {
    GIVEN("Writer with enough space") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        WHEN("Int8 is written") {
            int8_t i = GENERATE(-128, 5, 127);
            cmWriteI8(writer, i);

            THEN("No errors") {
                REQUIRE(writer->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Used size is increased") {
                REQUIRE(writer->usedSize == 4);
            }

            AND_THEN("Int8 flag is placed after 2nd byte") {
                REQUIRE(buffer[2] == 0x01);
            }

            AND_THEN("Int8 is placed after 3rd byte") {
                REQUIRE(((int8_t *) buffer.data())[3] == i);
            }
        }

        WHEN("UInt8 is written") {
            uint8_t i = GENERATE(0, 127, 255);
            cmWriteU8(writer, i);

            THEN("UInt8 flag is placed after 2nd byte") {
                REQUIRE(buffer[2] == 0x02);
            }

            AND_THEN("UInt8 is placed after 3rd byte") {
                REQUIRE(buffer[3] == i);
            }
        }

        WHEN("Int32 is written") {
            int32_t i = GENERATE(INT32_MIN, 0, INT32_MAX);
            cmWriteI32(writer, i);

            THEN("Int32 flag is placed after 2nd byte") {
                REQUIRE(buffer[2] == 0x05);
            }

            AND_THEN("Int32 is placed after 3rd byte") {
                REQUIRE(*(int32_t *) &buffer[3] == i);
            }
        }
    }

    GIVEN("Writer with not enough space") {
        std::vector<uint8_t> buffer(3);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        WHEN("Int8 is written") {
            cmWriteI8(writer, 15);

            THEN("No space error") {
                REQUIRE(writer->firstError == CM_ERROR_NO_SPACE);
            }
        }
    }
}

SCENARIO("Read message", "[read]") {
    GIVEN("Non-empty message with int8") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        int8_t i = GENERATE(-128, 5, 127);
        cmWriteI8(writer, i);

        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("Int8 is read") {
            auto r = cmReadI8(reader);

            THEN("No errors") {
                REQUIRE(reader->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read size is increased") {
                REQUIRE(reader->readSize == 4);
            }

            AND_THEN("Read int8 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Non-empty message with uint8") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        uint8_t i = GENERATE(0, 127, 255);
        cmWriteU8(writer, i);

        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("UInt8 is read") {
            auto r = cmReadU8(reader);

            THEN("Read uint8 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Non-empty message with int32") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        int32_t i = GENERATE(INT32_MIN, 0, INT32_MAX);
        cmWriteI32(writer, i);

        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("Int32 is read") {
            auto r = cmReadI32(reader);

            THEN("No errors") {
                REQUIRE(reader->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read int32 is correct") {
                REQUIRE(i == r);
            }
        }
    }


    GIVEN("Non-empty message with uint32") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        uint32_t i = GENERATE(0, INT32_MAX, UINT32_MAX);
        cmWriteU32(writer, i);

        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("UInt32 is read") {
            auto r = cmReadU32(reader);

            THEN("Read uint32 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Non-empty message with int32 in inverse endian mode") {
        std::vector<uint8_t> buffer(1024);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());

        int32_t i = GENERATE(INT32_MIN, 0, INT32_MAX);
        cmWriteI32(writer, i);
        std::swap(buffer[0], buffer[1]);
        std::swap(buffer[3], buffer[6]);
        std::swap(buffer[4], buffer[5]);

        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("Int32 is read") {
            auto r = cmReadI32(reader);

            THEN("No errors") {
                REQUIRE(reader->firstError == CM_ERROR_NONE);
            }

            AND_THEN("Read int32 is correct") {
                REQUIRE(i == r);
            }
        }
    }

    GIVEN("Message without data") {
        std::vector<uint8_t> buffer(2);
        auto writer = cmGetStaticWriter(buffer.data(), buffer.size());
        auto reader = cmGetStaticReader(writer->buffer, writer->usedSize);

        WHEN("Int8 is read") {
            cmReadI8(reader);

            THEN("No value error") {
                REQUIRE(reader->firstError == CM_ERROR_NO_VALUE);
            }
        }
    }
}
