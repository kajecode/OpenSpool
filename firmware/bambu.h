// filament_mappings.h

#pragma once

#include <unordered_map>
#include <string>

#include "esphome/components/nfc/nfc_tag.h"

namespace openspool
{
    struct Tag
    {
        std::string color_hex;
        std::string type;
        std::string brand;
        uint16_t min_temp{0};
        uint16_t max_temp{0};
    };

    inline bool decode(const std::string &payload, Tag &tag)
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            ESP_LOGE("openspool", "Failed to parse input JSON: %s", error.c_str());
            return false;
        }

        if (doc["version"].isNull() || doc["version"].as<std::string>() != "1.0")
        {
            ESP_LOGE("openspool", "Invalid or missing version. Expected version '1.0'");
            return false;
        }

        if (doc["protocol"] != "openspool")
        {
            return false;
        }

        const char *required_fields[] = {"color_hex", "min_temp", "max_temp", "brand", "type"};
        for (const char *field : required_fields)
        {
            if (doc[field].isNull())
            {
                ESP_LOGE("openspool", "Missing required field: %s", field);
                return false;
            }
        }

        tag.color_hex = doc["color_hex"].as<std::string>();
        tag.type = doc["type"].as<std::string>();
        tag.brand = doc["brand"].as<std::string>();
        tag.min_temp = doc["min_temp"].as<uint16_t>();
        tag.max_temp = doc["max_temp"].as<uint16_t>();

        if (tag.color_hex.length() != 6 && tag.color_hex.length() != 8)
        {
            ESP_LOGE("openspool", "Invalid color_hex length (expected 6 or 8 characters)");
            return false;
        }
        return true;
    }

    inline std::string format_display(const std::string &payload)
    {
        if (payload.empty())
        {
            ESP_LOGD("NFC", "Input string is empty");
            return payload;
        }
        ESP_LOGD("NFC", "Input string: %s", payload.c_str());

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            ESP_LOGE("NFC", "JSON parsing failed: %s", error.c_str());
            return {};
        }
        if (!doc.is<JsonObject>())
        {
            ESP_LOGE("NFC", "Invalid JSON: Not an object");
            return {};
        }

        const char *required_fields[] = {"protocol", "color_hex", "type", "min_temp", "max_temp", "brand"};
        for (const char *field : required_fields)
        {
            if (!doc[field].is<const char *>() && !doc[field].is<int>() && !doc[field].is<float>())
            {
                ESP_LOGE("NFC", "Invalid JSON: Missing required field '%s'", field);
                return {};
            }
        }

        std::string output;
        serializeJsonPretty(doc, output);
        if (output.length() > 1024)
        {
            ESP_LOGE("NFC", "Prettified JSON exceeds 1024 bytes");
            return {};
        }
        return output;
    }
}

namespace bambulabs
{
    const std::unordered_map<std::string, std::string> filament_mappings = {
        {"TPU for AMS", "GFU02"},
        {"TPU High Speed", "GFU00"},
        {"TPU", "GFU99"},
        {"PLA", "GFL99"},
        {"PLA High Speed", "GFL95"},
        {"PLA Silk", "GFL96"},
        {"PETG", "GFG99"},
        {"PET-CF", "GFG99"},
        {"ASA", "GFB98"},
        {"ABS", "GFB99"},
        {"PC", "GFC99"},
        {"PA", "GFN99"},
        {"PA-CF", "GFN98"},
        {"PLA-CF", "GFL98"},
        {"PVA", "GFS99"},
        {"BVOH", "GFS97"},
        {"EVA", "GFR99"},
        {"HIPS", "GFS98"},
        {"PC", "GFC99"},
        {"PCTG", "GFG97"},
        {"PE", "GFP99"},
        {"PE-CF", "GFP98"},
        {"PHA", "GFR98"},
        {"PP", "GFP97"},
        {"PP-CF", "GFP96"},
        {"PP-GF", "GFP95"},
        {"PPA-CF", "GFN97"},
        {"PPA-GF", "GFN96"},
        {"Support", "GFS00"}};

    // Special cases for brand-specific codes
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> brand_specific_codes = {
        {"PLA", {{"Bambu", "GFA00"}, {"PolyTerra", "GFL01"}, {"PolyLite", "GFL00"}, {"Sunlu", "GFSNL03"}}},
        {"PLA Aero", {{"Bambu", "GFG01"}}},
        {"TPU", {{"Bambu", "GFU01"}}},
        {"ABS", {{"Bambu", "GFB00"}, {"PolyLite", "GFB60"}}},
        {"ASA", {{"Bambu", "GFB01"}, {"PolyLite", "GFB61"}}},
        {"PC", {{"Bambu", "GFC00"}}},
        {"PA-CF", {{"Bambu", "GFN03"}}},
        {"PET-CF", {{"Bambu", "GFT00"}}},
        {"PETG HF", {{"Bambu", "GFG02"}}},
        {"PETG Translucent", {{"Bambu", "GFG01"}}},
        {"PETG", {{"Bambu", "GFG00"}, {"PolyLite", "GFG60"}, {"Sunlu", "GFSNL08"}}}};

    // Function with two parameters
    inline std::string get_bambu_code(const std::string &type, const std::string &brand = "")
    {
        if (!brand.empty())
        {
            auto brand_it = brand_specific_codes.find(type);
            if (brand_it != brand_specific_codes.end())
            {
                auto code_it = brand_it->second.find(brand);
                if (code_it != brand_it->second.end())
                {
                    return code_it->second;
                }
            }
        }

        auto it = filament_mappings.find(type);
        if (it != filament_mappings.end())
        {
            return it->second;
        }
        return ""; // Unknown type
    }

    // Function with three parameters (for Bambu PLA subtypes)
    inline std::string get_bambu_code(const std::string &type, const std::string &brand, const std::string &subtype)
    {
        if (type == "PLA" && brand == "Bambu")
        {
            if (subtype == "Matte")
                return "GFA01";
            if (subtype == "Metal")
                return "GFA02";
            if (subtype == "Impact")
                return "GFA03";
            return "GFA00"; // Default to Basic for unknown subtypes
        }
        return get_bambu_code(type, brand);
    }

    inline std::string generate_mqtt_payload(const openspool::Tag &tag, uint16_t ams_id, uint16_t ams_tray)
    {
        JsonDocument doc_out;
        JsonObject print = doc_out["print"].to<JsonObject>();
        print["sequence_id"] = "0";
        print["command"] = "ams_filament_setting";
        print["ams_id"] = ams_id;
        print["tray_id"] = ams_tray;
        if (tag.color_hex.length() == 6)
        {
            print["tray_color"] = tag.color_hex + "FF";
        }
        else
        {
            print["tray_color"] = tag.color_hex;
        }
        print["nozzle_temp_min"] = tag.min_temp;
        print["nozzle_temp_max"] = tag.max_temp;
        print["tray_type"] = tag.type;
        print["setting_id"] = "";
        print["tray_info_idx"] = get_bambu_code(tag.type, tag.brand);
        // print["tray_sub_brands"] = doc_in["sub_brand"]; //TODO: support sub brands if needed

        std::string result;
        serializeJson(doc_out, result);

        if (result.empty())
        {
            ESP_LOGE("bambu", "Failed to build JSON");
            return {};
        }

        ESP_LOGI("mqtt", "Publishing %s", result.c_str());
        return result;
    }

    inline std::string generate_mqtt_payload(std::string openspool_tag_json, uint16_t ams_id, uint16_t ams_tray)
    {
        openspool::Tag tag;
        if (!openspool::decode(openspool_tag_json, tag))
        {
            return {};
        }
        return generate_mqtt_payload(tag, ams_id, ams_tray);
    }

}

namespace rfid
{
    struct TagResult
    {
        bool is_valid_openspool{false};
        std::string payload;
    };

    inline TagResult read_openspool_tag(esphome::nfc::NfcTag &tag)
    {
        TagResult result;

        if (!tag.has_ndef_message())
        {
            ESP_LOGI("NFC", "Tag found without NDEF message");
            return result;
        }

        const auto &records = tag.get_ndef_message()->get_records();
        bool found_json = false;
        for (const auto &record : records)
        {
            if (record->get_type() != "application/json")
            {
                continue;
            }

            if (found_json)
            {
                ESP_LOGW("NFC", "Multiple JSON records found, using first one");
                break;
            }

            result.payload = record->get_payload();
            ESP_LOGD("NFC", "Payload: %s", result.payload.c_str());

            openspool::Tag openspool_tag;
            result.is_valid_openspool = openspool::decode(result.payload, openspool_tag);

            found_json = true;
        }

        if (!found_json)
        {
            ESP_LOGW("NFC", "No application/json record found");
        }
        return result;
    }
}