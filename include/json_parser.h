#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>

namespace ai_glasses {

class JsonValue;

using JsonNull = std::nullptr_t;
using JsonBool = bool;
using JsonNumber = double;
using JsonString = std::string;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::unordered_map<std::string, JsonValue>;

class JsonValue {
public:
    using Value = std::variant<JsonNull, JsonBool, JsonNumber, JsonString, 
                               std::shared_ptr<JsonArray>, std::shared_ptr<JsonObject>>;
    
    JsonValue();
    JsonValue(std::nullptr_t);
    JsonValue(bool value);
    JsonValue(double value);
    JsonValue(int value);
    JsonValue(const std::string& value);
    JsonValue(const char* value);
    JsonValue(const JsonArray& value);
    JsonValue(const JsonObject& value);
    
    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    
    bool asBool() const;
    double asNumber() const;
    int asInt() const;
    const std::string& asString() const;
    const JsonArray& asArray() const;
    const JsonObject& asObject() const;
    
    bool has(const std::string& key) const;
    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](size_t index) const;
    size_t size() const;

private:
    Value value_;
    static JsonValue null_value_;
};

class JsonParser {
public:
    static JsonValue parse(const std::string& json);
    static std::string stringify(const JsonValue& value, bool pretty = false);
    
private:
    static JsonValue parseValue(const std::string& json, size_t& pos);
    static JsonValue parseObject(const std::string& json, size_t& pos);
    static JsonValue parseArray(const std::string& json, size_t& pos);
    static JsonString parseString(const std::string& json, size_t& pos);
    static JsonNumber parseNumber(const std::string& json, size_t& pos);
    static JsonBool parseBool(const std::string& json, size_t& pos);
    static JsonNull parseNull(const std::string& json, size_t& pos);
    
    static void skipWhitespace(const std::string& json, size_t& pos);
    static std::string escapeString(const std::string& str);
    static std::string unescapeString(const std::string& str);
};

}

#endif
