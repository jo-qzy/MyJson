#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>

using namespace std;

// error number
enum error_number {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_OVERFLOW,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_STRING_CHAR,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_MISS_COMMA_OR_SQUARE_BRAKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

enum json_type {
    JSON_NULL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

class Writer;

class Value {
public:
    Value() :type(JSON_NULL) {}

    Value(double value) :type(JSON_NUMBER), number(value) {}

    Value(const char* value) :type(JSON_STRING), str(value) {}

    Value(const string& value) :type(JSON_STRING), str(value) {}

    Value(const string key, const Value value) :type(JSON_OBJECT) {
        object.insert(make_pair(key, value));
    }

    Value(const char *beginValue, const char *endValue) :type(JSON_STRING), str(beginValue, endValue) {}

    ~Value() {
    }

    Value& operator[](const string key) {
        assert(type == JSON_OBJECT);
        return object[key];
    }

//  const Value operator[](const string key) const {
//      return object[key];
//  }

    Value& operator[](const size_t index) {
        assert(type == JSON_ARRAY && index < array.size());
        return array[index];
    }

    const Value& operator[](const size_t index) const {
        assert(type == JSON_ARRAY && index < array.size());
        return array[index];
    }

    json_type get_type() {
        return type;
    }

    double asDouble() const {
        assert(type == JSON_NUMBER);
        return number;
    }

    string asString() const {
        switch (type) {
        case JSON_NULL: return string("null");
        case JSON_TRUE: return string("true");
        case JSON_FALSE: return string("false");
        case JSON_STRING: return str;
//      case JSON_ARRAY: return string();
//      case JSON_OBJECT: return string();
        }
    }

    bool empty() {
        switch (type) {
        case JSON_ARRAY: return array.empty();
        case JSON_OBJECT: return object.empty();
        default: return true;
        }
    }

    size_t size() {
        assert(type == JSON_ARRAY || type == JSON_OBJECT);
        switch (type) {
        case JSON_ARRAY: return array.size();
        case JSON_OBJECT: return object.size();
        }
    }

    bool isNull() {
        return type == JSON_NULL;
    }

    bool isBool() const {
        return type == JSON_TRUE || type == JSON_FALSE;
    }

    bool isDouble() const {
        return type == JSON_NUMBER;
    }

    bool isString() const {
        return type == JSON_STRING;
    }

    bool isArray() const {
        return type == JSON_ARRAY;
    }

    bool isObject() const {
        return type == JSON_OBJECT;
    }

    bool isValidIndex(const size_t index) const {
        return index < array.size();
    }

    bool isMember(string key) const {
        return object.find(key) != object.end();
    }

    Value get(const string key, const Value default_value) const {
        return Value(key, default_value);
    }

    vector<string> getMemberNames() const {
        assert(type == JSON_OBJECT);
        vector<string> names;
        for (auto e : object) {
            names.push_back(e.first);
        }
        return names;
    }

    Value removeMember(const string key) {
        map<string, Value>::iterator mt = object.find(key);
        if (mt != object.end())
            object.erase(mt);
        return *this;
    }

    void setComment(const string _comment) {
        comment = _comment;
    }

    bool hasComment() {
        return comment != "";
    }

    string getComment() {
        return comment;
    }

    vector<Value> get_array() {
        assert(type == JSON_ARRAY);
        return array;
    }

    map<string, Value> get_object() {
        assert(type == JSON_OBJECT);
        return object;
    }

    Value get_object(string str) {
        assert(type == JSON_OBJECT);
        return object[str];
    }

    void set_literal(json_type dst_type) {
        type = dst_type;
    }

    void set_number(double dst_number) {
        type = JSON_NUMBER;
        number = dst_number;
    }

    void set_string(string dst_str) {
        type = JSON_STRING;
        str = dst_str;
    }

    void set_array(vector<Value>& dst_array) {
        type = JSON_ARRAY;
        array = dst_array;
    }

    void set_object(map<string, Value>& dst_object) {
        type = JSON_OBJECT;
        object = dst_object;
    }

//  string toStyledString() {
//      w->toStyledString(*this);
//  }
private:
    json_type type;
    string comment;
    double number;
    string str;
    vector<Value> array;
    map<string, Value> object;
    Writer* w;
};

class value_parse {
public:
    void set_json_source(const string& source, Value* v) {
        json_source = source;
        it = json_source.begin();
        value = v;
    }

    int parse()
    {
        skip_blank();
        int ret = 0;
        if ((ret = parse_value()) == PARSE_OK) {
            skip_blank();
            if (it != json_source.end()) {
                value->set_literal(JSON_NULL);
                return PARSE_ROOT_NOT_SINGULAR;
            }
        }
        return ret;
    }
private:
    int parse_value(Value* element = nullptr) {
        if (it == json_source.end())
            return PARSE_EXPECT_VALUE;
        switch (*it) {
        case 'n': return parse_literal("null", JSON_NULL, element);
        case 't': return parse_literal("true", JSON_TRUE, element);
        case 'f': return parse_literal("false", JSON_FALSE, element);
        case '[': return parse_array(element);
        case '{': return parse_object(element);
        case '\"': return parse_string(element);
        case '\0': return PARSE_EXPECT_VALUE;
        default:
            if (*it == '-' || (*it >= '0' && *it <= '9')) {
                return parse_number(element);
            }
            return PARSE_INVALID_VALUE;
        }
    }

    void skip_blank() {
        while (it != json_source.end() && *it == ' ')
            it++;
    }

    int parse_literal(const char* dst, json_type type, Value* element = nullptr) {
        int len = strlen(dst);
        if (strncmp(&(*it), dst, len) == 0) {
            it += len;
            if (element != nullptr)
                element->set_literal(type);
            else
                value->set_literal(type);
            return PARSE_OK;
        }
        else
            return PARSE_INVALID_VALUE;
    }

#define ISDIGIT(num) ((num >= '0') && (num <= '9'))
#define ISDIGIT1TO9(num) ((num >= '1') && (num <= '9'))

    int parse_number(Value* element = nullptr) {
        string::const_iterator tmp_it = it;
        if (*tmp_it == '-')
            tmp_it++;
        if (*tmp_it == '0')
            tmp_it++;
        else {
            if (!ISDIGIT1TO9(*tmp_it))
                return PARSE_INVALID_VALUE;
            for (tmp_it++; ISDIGIT(*tmp_it); tmp_it++);
        }
        if (*tmp_it == '.') {
            tmp_it++;
            if (!ISDIGIT(*tmp_it))
                return PARSE_INVALID_VALUE;
            for (tmp_it++; ISDIGIT(*tmp_it); tmp_it++);
        }
        if (*tmp_it == 'e' || *tmp_it == 'E') {
            tmp_it++;
            if (*tmp_it == '-' || *tmp_it == '+')
                tmp_it++;
            if (!ISDIGIT(*tmp_it))
                return PARSE_INVALID_VALUE;
            for (tmp_it++; ISDIGIT(*tmp_it); tmp_it++);
        }
        // if (*tmp_it != ' ' && *tmp_it != '\0')
        //    return PARSE_INVALID_VALUE;
        double dst_number = strtod(&(*it), NULL);
        if (errno == ERANGE && (dst_number == HUGE_VAL || dst_number == -HUGE_VAL))
            return PARSE_NUMBER_OVERFLOW;
        it = tmp_it;
        if (element != nullptr)
            element->set_number(dst_number);
        else
            value->set_number(dst_number);
        return PARSE_OK;
    }

#define CHECK_ITERATOR(it) do { if (it == json_source.end()) return PARSE_MISS_QUOTATION_MARK; } while(0)

    int parse_string(Value* element = nullptr) {
        string tmp_str;
        string::const_iterator tmp_it = it;
        int ret = parse_string(tmp_str, tmp_it);
        if (ret == PARSE_OK) {
            it = tmp_it;
            if (element != nullptr)
                element->set_string(tmp_str);
            else
                value->set_string(tmp_str);
        }
        return ret;
    }

    int parse_string(string& tmp_str, string::const_iterator& tmp_it) {
        char ch = 0;
        tmp_it++;
        while (tmp_it != json_source.end()) {
            ch = *tmp_it++;
            switch (ch) {
            case '\"':
                return PARSE_OK;
            case '\\':
                if (tmp_it != json_source.end()) {
                    switch (*tmp_it++) {
                    case '\"': tmp_str += '\"'; break;
                    case 'n': tmp_str += '\n'; break;
                    case 'r': tmp_str += '\r'; break;
                    case 't': tmp_str += '\t'; break;
                    case 'f': tmp_str += '\f'; break;
                    case 'b': tmp_str += '\b'; break;
                    case '/': tmp_str += '/'; break;
                    case '\\': tmp_str += '\\'; break;
                    case 'u': { 
                        unsigned u = 0;
                        int ret = 0;
                        if ((ret = parse_hex4(tmp_it, u)) != PARSE_OK)
                            return ret;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            CHECK_ITERATOR(tmp_it);
                            if (*tmp_it++ != '\\')
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            CHECK_ITERATOR(tmp_it);
                            if (*tmp_it++ != 'u')
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            unsigned tmp_u;
                            if ((ret = parse_hex4(tmp_it, tmp_u)) != PARSE_OK)
                                return ret;
                            if (tmp_u <0xDC00 || tmp_u > 0xDFFF)
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            u = 0x10000 + (u - 0xD800) * 0x400 + (tmp_u - 0xDC00);
                        }
                        if (u > 0x10FFFF)
                            return PARSE_INVALID_UNICODE_HEX;
                        encode_utf8(u, tmp_str);
                        break;
                    }
                    default: return PARSE_INVALID_STRING_ESCAPE;
                    }
                }
                else
                    return PARSE_MISS_QUOTATION_MARK;
                break;
            case '\0':
                return PARSE_MISS_QUOTATION_MARK;
            default:
                if (ch < 0x20)
                    return PARSE_INVALID_STRING_CHAR;
                tmp_str += ch;
                break;
            }
        }
        return PARSE_MISS_QUOTATION_MARK;
    }

    int parse_array(Value* element = nullptr) {
        it++;
        vector<Value> tmp_array;
        int ret = 0;
        skip_blank();
        if (*it == ']') {
            it++;
            if (element != nullptr)
                element->set_array(tmp_array);
            else
                value->set_array(tmp_array);
            return PARSE_OK;
        }
        while (it != json_source.end()) {
            Value tmp_element;
            skip_blank();
            if ((ret = parse_value(&tmp_element)) != PARSE_OK)
                return ret;
            skip_blank();
            // insert element to array
            tmp_array.push_back(tmp_element);
            if (*it == ',') {
                CHECK_ITERATOR(it);
                it++;
            }
            else if (*it == ']') {
                CHECK_ITERATOR(it);
                it++;
                break;
            }
            else
                return PARSE_MISS_COMMA_OR_SQUARE_BRAKET;
        }
        if (element != nullptr)
            element->set_array(tmp_array);
        else
            value->set_array(tmp_array);
        return PARSE_OK;
    }

    int parse_object(Value* element = nullptr) {
        it++;
        skip_blank();
        int ret = 0;
        map<string, Value> tmp_object;
        if (*it == '}') {
            if (element != nullptr)
                element->set_object(tmp_object);
            else
                value->set_object(tmp_object);
            return PARSE_OK;
        }
        for (;;) {
            skip_blank();
            CHECK_ITERATOR(it);
            if (*it != '\"')
                return PARSE_MISS_KEY;
            string key_str;
            string::const_iterator tmp_it = it;
            if ((ret = parse_string(key_str, tmp_it)) != PARSE_OK)
                return PARSE_MISS_KEY;
            it = tmp_it;
            skip_blank();
            CHECK_ITERATOR(it);
            if (*it != ':')
                return PARSE_MISS_COLON;
            it++;
            skip_blank();
            CHECK_ITERATOR(it);
            Value tmp_value;
            if ((ret = parse_value(&tmp_value)) != PARSE_OK)
                return ret;
            tmp_object.insert(make_pair(key_str, tmp_value));
            skip_blank();
            CHECK_ITERATOR(it);
            if (*it == '}') {
                it++;
                if (element != nullptr)
                    element->set_object(tmp_object);
                else
                    value->set_object(tmp_object);
                return PARSE_OK;
            }
            else if (*it == ',')
                it++;
            else {
                return PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            }
        }
    }

    int parse_hex4(string::const_iterator& tmp_it, unsigned& u) {
        u = 0;
        char ch = 0;
        for (int i = 0; i < 4; i++) {
            u <<= 4;
            CHECK_ITERATOR(tmp_it);
            ch = *tmp_it++;
            if (ch >= '0' && ch <= '9')
                u |= ch - '0';
            else if (ch >= 'a' && ch <= 'f')
                u |= ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')
                u |= ch - 'A' + 10;
            else return PARSE_INVALID_UNICODE_HEX;
        }
        return PARSE_OK;
    }

    void encode_utf8(unsigned& u, string& tmp_str) {
        if (u <= 0x7F)
            tmp_str += (u & 0xFF);
        else if (u <= 0x7FF) {
            tmp_str += (0xC0 | (0xFF & (u >> 6)));
            tmp_str += (0x80 | (0x3F & u));
        }
        else if (u <= 0xffff) {
            tmp_str += (0xE0 | (0xFF & (u >> 12)));
            tmp_str += (0x80 | (0x3F & (u >> 6)));
            tmp_str += (0x80 | (0x3F & u));
        }
        else {
            tmp_str += (0xF0 | (0xFF & (u >> 18)));
            tmp_str += (0x80 | (0x3F & (u >> 12)));
            tmp_str += (0x80 | (0x3F & (u >> 6)));
            tmp_str += (0x80 | (0x3F & u));
        }    
    }
private:
    string json_source;
    string::const_iterator it;
    Value* value;
};

class json_parser : public value_parse {
private:
    json_parser() {}
public:
    static json_parser* get_parser(const string& source, Value* v){
        static json_parser singleton_parser;
        singleton_parser.set_json_source(source, v);
        // use singleton create parser
        return &singleton_parser;
    }

    json_parser(const json_parser& ban_parser) = delete;

    json_parser& operator=(const json_parser& ban_parser) = delete;
};

class Reader {
public:
    static int parse(const string document, Value& value) {
        json_parser* parser = json_parser::get_parser(document, &value);
        int ret = parser->parse();
        parser = nullptr;
        return ret;
    }
};

class Writer {
public:
    string toStyledString(const Value& value);
};

class FastWriter : public Writer {
public:
    string toStyledString(Value& value) {
        return convert_value(value);
    }
protected:
    string convert_value(Value value) {
        switch (value.get_type()) {
            case JSON_NULL: return convert_literal(value.get_type()); break;
            case JSON_TRUE: return convert_literal(value.get_type()); break;
            case JSON_FALSE: return convert_literal(value.get_type()); break;
            case JSON_NUMBER: return convert_number(value.asDouble()); break;
            case JSON_STRING: return convert_string(value.asString()); break;
            case JSON_ARRAY: return convert_array(value.get_array()); break;
            case JSON_OBJECT: return convert_object(value.get_object()); break;
        }
    }
private:
    string convert_literal(json_type type) {
        switch (type) {
            case JSON_NULL: return string("null");
            case JSON_TRUE: return string("true");
            case JSON_FALSE: return string("false");
        }
    }

    string convert_number(const double number) {
        char buf[50];
        sprintf(buf, "%.17g", number);
        return string(buf);
    }

    string convert_string(const string str) {
        string tmp_str = "\"";
        for (auto e : str) {
            switch (e) {
                case '\\': tmp_str += "\\\\"; break;
                case '\"': tmp_str += "\\\""; break;
                case '\b': tmp_str += "\\b"; break;
                case '\f': tmp_str += "\\f"; break;
                case '\n': tmp_str += "\\n"; break;
                case '\r': tmp_str += "\\r"; break;
                case '\t': tmp_str += "\\t"; break;
                default:
                if (e < 0x20) {
                    char buf[7];
                    sprintf(buf, "\\u%04x", e);
                    tmp_str += buf;
                }
                else
                tmp_str += e;
                break;
            }
        }
        return tmp_str;
    }

    string convert_array(vector<Value> array) {
        string tmp_str("[ ");
        vector<Value>::const_iterator vt = array.begin();
        for (vt; vt != array.end(); vt++) {
            tmp_str += convert_value(*vt);
            tmp_str +=" , ";
        }
        tmp_str.pop_back();
        tmp_str.pop_back();
        tmp_str += "]";
        return tmp_str;
    }

    string convert_object(const map<string, Value> object) {
        string tmp_str("{ ");
        for (auto mt : object) {
            tmp_str += mt.first;
            tmp_str += " : ";
            tmp_str += convert_value(mt.second);
            tmp_str += " , ";
        }
        tmp_str.pop_back();
        tmp_str.pop_back();
        tmp_str += "}";
        return tmp_str;
    }
};

class StyleWriter : public Writer {
public:

};

#endif
