#pragma once
#include <SKSE/SKSE.h>

namespace DFF {
    namespace Serialization {
        template <typename T>
        inline T Read(SKSE::SerializationInterface* serde) {
            T val;
            serde->ReadRecordData(&val, sizeof(val));
            return val;
        }

        template <>
        inline std::string Read(SKSE::SerializationInterface* serde) {
            std::size_t nameSize;
            serde->ReadRecordData(&nameSize, sizeof(nameSize));

            std::string name;
            name.reserve(nameSize);

            char c;
            for (int i = 0; i < nameSize; i++) {
                serde->ReadRecordData(&c, sizeof(c));
                name += c;
            }
            return name;
        }

        template <typename T>
        inline void Write(SKSE::SerializationInterface* serde, T value) {
            serde->WriteRecordData(&value, sizeof(value));
        }

        template <>
        inline void Write(SKSE::SerializationInterface* serde, std::string name) {
            std::size_t nameSize = name.size();
            serde->WriteRecordData(&nameSize, sizeof(nameSize));

            char c;
            for (int i = 0; i < nameSize; i++) {
                c = name[i];
                serde->WriteRecordData(&c, sizeof(c));
            }
        }

    }
}