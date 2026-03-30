#include "json_parser.h"
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <functional>

namespace ai_glasses {

JsonValue JsonValue::null_value_ = JsonValue(nullptr);

JsonValue::JsonValue() : type_(Type::Null) {}
JsonValue::JsonValue(std::nullptr_t) : type_(Type::Null) {}
JsonValue::JsonValue(bool value) : type_(Type::Bool), bool_value_(value) {}
JsonValue::JsonValue(double value) : type_(Type::Number), number_value_(value) {}
JsonValue::JsonValue(int value) : type_(Type::Number), number_value_(static_cast<double>(value)) {}
JsonValue::JsonValue(const std::string& value) : type_(Type::String), string_value_(value) {}
JsonValue::JsonValue(const char* value) : type_(Type::String), string_value_(std::string(value)) {}
JsonValue::JsonValue(const JsonArray& value) : type_(Type::Array), array_value_(std::make_shared<JsonArray>(value)) {}
JsonValue::JsonValue(const JsonObject& value) : type_(Type::Object), object_value_(std::make_shared<JsonObject>(value)) {}

bool JsonValue::isNull() const { return type_ == Type::Null; }
bool JsonValue::isBool() const { return type_ == Type::Bool; }
bool JsonValue::isNumber() const { return type_ == Type::Number; }
bool JsonValue::isString() const { return type_ == Type::String; }
bool JsonValue::isArray() const { return type_ == Type::Array; }
bool JsonValue::isObject() const { return type_ == Type::Object; }

bool JsonValue::asBool() const { 
    return bool_value_; 
}
double JsonValue::asNumber() const { 
    return number_value_; 
}
int JsonValue::asInt() const { 
    return static_cast<int>(number_value_); 
}
const std::string& JsonValue::asString() const { 
    return string_value_; 
}
const JsonArray& JsonValue::asArray() const { 
    return *array_value_; 
}
const JsonObject& JsonValue::asObject() const { 
    return *object_value_; 
}

bool JsonValue::has(const std::string& key) const {
    if (!isObject()) return false;
    const auto& obj = asObject();
    return obj.find(key) != obj.end();
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (!isObject()) return null_value_;
    const auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end()) return null_value_;
    return it->second;
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (!isArray()) return null_value_;
    const auto& arr = asArray();
    if (index >= arr.size()) return null_value_;
    return arr[index];
}

size_t JsonValue::size() const {
    if (isArray()) return asArray().size();
    if (isObject()) return asObject().size();
    return 0;
}

void JsonParser::skipWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(json[pos])) {
        pos++;
    }
}

std::string JsonParser::unescapeString(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            i++;
            switch (str[i]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': 
                    if (i + 4 < str.size()) {
                        std::string hex = str.substr(i + 1, 4);
                        int codepoint = std::stoi(hex, nullptr, 16);
                        if (codepoint < 0x80) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint < 0x800) {
                            result += static_cast<char>(0xC0 | (codepoint >> 6));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (codepoint >> 12));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                        i += 4;
                    }
                    break;
                default:
                    result += str[i];
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

JsonString JsonParser::parseString(const std::string& json, size_t& pos) {
    if (json[pos] != '"') {
        throw std::runtime_error("Expected '\"' at position " + std::to_string(pos));
    }
    pos++;
    
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            pos++;
            switch (json[pos]) {
                case 'u':
                    result += json.substr(pos - 1, 6);
                    break;
                default:
                    result += json[pos];
            }
        } else {
            result += json[pos];
        }
        pos++;
    }
    
    if (pos >= json.size()) {
        throw std::runtime_error("Unterminated string");
    }
    
    pos++;
    return unescapeString(result);
}

JsonNumber JsonParser::parseNumber(const std::string& json, size_t& pos) {
    size_t start = pos;
    
    if (json[pos] == '-') pos++;
    
    while (pos < json.size() && std::isdigit(json[pos])) pos++;
    
    if (pos < json.size() && json[pos] == '.') {
        pos++;
        while (pos < json.size() && std::isdigit(json[pos])) pos++;
    }
    
    if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
        pos++;
        if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) pos++;
        while (pos < json.size() && std::isdigit(json[pos])) pos++;
    }
    
    return std::stod(json.substr(start, pos - start));
}

JsonBool JsonParser::parseBool(const std::string& json, size_t& pos) {
    if (json.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    } else if (json.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    }
    throw std::runtime_error("Invalid boolean at position " + std::to_string(pos));
}

JsonNull JsonParser::parseNull(const std::string& json, size_t& pos) {
    if (json.substr(pos, 4) == "null") {
        pos += 4;
        return nullptr;
    }
    throw std::runtime_error("Invalid null at position " + std::to_string(pos));
}

JsonValue JsonParser::parseValue(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);
    
    if (pos >= json.size()) {
        throw std::runtime_error("Unexpected end of input");
    }
    
    char c = json[pos];
    
    if (c == '{') {
        return parseObject(json, pos);
    } else if (c == '[') {
        return parseArray(json, pos);
    } else if (c == '"') {
        return JsonValue(parseString(json, pos));
    } else if (c == 't' || c == 'f') {
        return JsonValue(parseBool(json, pos));
    } else if (c == 'n') {
        return JsonValue(parseNull(json, pos));
    } else if (c == '-' || std::isdigit(c)) {
        return JsonValue(parseNumber(json, pos));
    }
    
    throw std::runtime_error("Unexpected character at position " + std::to_string(pos));
}

JsonValue JsonParser::parseObject(const std::string& json, size_t& pos) {
    if (json[pos] != '{') {
        throw std::runtime_error("Expected '{' at position " + std::to_string(pos));
    }
    pos++;
    
    JsonObject obj;
    skipWhitespace(json, pos);
    
    if (pos < json.size() && json[pos] == '}') {
        pos++;
        return JsonValue(obj);
    }
    
    while (true) {
        skipWhitespace(json, pos);
        
        if (json[pos] != '"') {
            throw std::runtime_error("Expected string key at position " + std::to_string(pos));
        }
        
        std::string key = parseString(json, pos);
        
        skipWhitespace(json, pos);
        
        if (pos >= json.size() || json[pos] != ':') {
            throw std::runtime_error("Expected ':' at position " + std::to_string(pos));
        }
        pos++;
        
        JsonValue value = parseValue(json, pos);
        obj[key] = value;
        
        skipWhitespace(json, pos);
        
        if (pos >= json.size()) {
            throw std::runtime_error("Unexpected end of object");
        }
        
        if (json[pos] == '}') {
            pos++;
            break;
        } else if (json[pos] == ',') {
            pos++;
        } else {
            throw std::runtime_error("Expected ',' or '}' at position " + std::to_string(pos));
        }
    }
    
    return JsonValue(obj);
}

JsonValue JsonParser::parseArray(const std::string& json, size_t& pos) {
    if (json[pos] != '[') {
        throw std::runtime_error("Expected '[' at position " + std::to_string(pos));
    }
    pos++;
    
    JsonArray arr;
    skipWhitespace(json, pos);
    
    if (pos < json.size() && json[pos] == ']') {
        pos++;
        return JsonValue(arr);
    }
    
    while (true) {
        JsonValue value = parseValue(json, pos);
        arr.push_back(value);
        
        skipWhitespace(json, pos);
        
        if (pos >= json.size()) {
            throw std::runtime_error("Unexpected end of array");
        }
        
        if (json[pos] == ']') {
            pos++;
            break;
        } else if (json[pos] == ',') {
            pos++;
        } else {
            throw std::runtime_error("Expected ',' or ']' at position " + std::to_string(pos));
        }
    }
    
    return JsonValue(arr);
}

JsonValue JsonParser::parse(const std::string& json) {
    size_t pos = 0;
    JsonValue result = parseValue(json, pos);
    skipWhitespace(json, pos);
    return result;
}

std::string JsonParser::stringify(const JsonValue& value, bool pretty) {
    std::ostringstream oss;
    
    // C++14 兼容的递归 lambda
    std::function<void(const JsonValue&)> writeValue;
    writeValue = [&oss, &writeValue, pretty](const JsonValue& v) {
        if (v.isNull()) {
            oss << "null";
        } else if (v.isBool()) {
            oss << (v.asBool() ? "true" : "false");
        } else if (v.isNumber()) {
            oss << v.asNumber();
        } else if (v.isString()) {
            oss << "\"" << escapeString(v.asString()) << "\"";
        } else if (v.isArray()) {
            oss << "[";
            const auto& arr = v.asArray();
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) oss << ",";
                if (pretty) oss << "\n";
                writeValue(arr[i]);
            }
            if (pretty && !arr.empty()) oss << "\n";
            oss << "]";
        } else if (v.isObject()) {
            oss << "{";
            const auto& obj = v.asObject();
            bool first = true;
            for (const auto& pair : obj) {
                if (!first) oss << ",";
                first = false;
                if (pretty) oss << "\n";
                oss << "\"" << pair.first << "\":";
                if (pretty) oss << " ";
                writeValue(pair.second);
            }
            if (pretty && !obj.empty()) oss << "\n";
            oss << "}";
        }
    };
    
    writeValue(value);
    return oss.str();
}

std::string JsonParser::escapeString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
        }
    }
    return result;
}

}
