#include "default_midas_unpacker_stage.h"
#include <TTree.h>
#include <sstream>
#include <iomanip>
#include <chrono>

using json = nlohmann::json;

DefaultMidasUnpackerStage::DefaultMidasUnpackerStage() = default;
DefaultMidasUnpackerStage::~DefaultMidasUnpackerStage() = default;

void DefaultMidasUnpackerStage::ProcessMidasEvent(const TMEvent& event) {
    json j;
    j["event_id"] = event.event_id;
    j["serial_number"] = event.serial_number;
    j["trigger_mask"] = event.trigger_mask;
    j["timestamp"] = event.time_stamp;
    j["data_size"] = event.data_size;
    j["event_header_size"] = event.event_header_size;
    j["bank_header_flags"] = event.bank_header_flags;

    TMEvent& mutable_event = const_cast<TMEvent&>(event);
    mutable_event.FindAllBanks();

    j["banks"] = json::array();
    for (const auto& bank : mutable_event.banks) {
        json jbank;
        jbank["name"] = bank.name;
        jbank["type"] = bank.type;
        jbank["data_size"] = bank.data_size;
        jbank["data"] = decodeBankData(bank, mutable_event);
        j["banks"].push_back(jbank);
    }

    SafeTreeAccess([&](TTree* tree) {
        static std::string json_str;
        json_str = j.dump();

        static TBranch* b = nullptr;
        if (b == nullptr) {
            b = tree->Branch("event_json", &json_str);
        }
        tree->Fill();
    });
}

json DefaultMidasUnpackerStage::decodeBankData(const TMBank& bank, const TMEvent& event) const {
    const char* bankData = event.GetBankData(&bank);
    if (!bankData || bank.data_size == 0) return nullptr;

    size_t dataSize = bank.data_size;
    json dataArray = json::array();

    switch (bank.type) {
        case TID_UINT8: {
            const uint8_t* arr = reinterpret_cast<const uint8_t*>(bankData);
            for (size_t i = 0; i < dataSize; ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_INT8: {
            const int8_t* arr = reinterpret_cast<const int8_t*>(bankData);
            for (size_t i = 0; i < dataSize; ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_UINT16: {
            const uint16_t* arr = reinterpret_cast<const uint16_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(uint16_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_INT16: {
            const int16_t* arr = reinterpret_cast<const int16_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(int16_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_UINT32: {
            const uint32_t* arr = reinterpret_cast<const uint32_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(uint32_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_INT32: {
            const int32_t* arr = reinterpret_cast<const int32_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(int32_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_UINT64: {
            const uint64_t* arr = reinterpret_cast<const uint64_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(uint64_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_INT64: {
            const int64_t* arr = reinterpret_cast<const int64_t*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(int64_t); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_FLOAT: {
            const float* arr = reinterpret_cast<const float*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(float); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_DOUBLE: {
            const double* arr = reinterpret_cast<const double*>(bankData);
            for (size_t i = 0; i < dataSize / sizeof(double); ++i) dataArray.push_back(arr[i]);
            break;
        }
        case TID_STRING: {
            return std::string(bankData, dataSize);
        }
        default: {
            return toHexString(bankData, dataSize);
        }
    }

    return dataArray;
}

std::string DefaultMidasUnpackerStage::toHexString(const char* data, size_t size) const {
    std::ostringstream oss;
    for (size_t i = 0; i < size; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << (static_cast<uint8_t>(data[i]) & 0xFF);
    }
    return oss.str();
}

std::string DefaultMidasUnpackerStage::Name() const {
    return "DefaultMidasUnpackerStage";
}

