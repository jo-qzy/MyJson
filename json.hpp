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

struct json_value {
public:
    ~json_value() {
    }

    int get_type() {
        return type;
    }

    double get_number() {
        assert(type == JSON_NUMBER);
        return number;
    }

    string get_string() {
        assert(type == JSON_STRING);
        return str;
    }

    vector<json_value> get_array() {
        assert(type == JSON_ARRAY);
        return array;
    }

    map<string, json_value> get_object() {
        assert(type == JSON_OBJECT);
        return object;
    }

    void set_literal(json_type dst_type) {
        type = dst_type;
    }

    void set_number(double dst_number) {
        type = JSON_NUMBER;
        number = dst_number;
    }

    void set_string(string& dst_str) {
        type = JSON_STRING;
        str = dst_str;
    }

    void set_array(vector<json_value>& dst_array) {
        type = JSON_ARRAY;
        array = dst_array;
    }

    void set_object(map<string, json_value>& dst_object) {
        type = JSON_OBJECT;
        object = dst_object;
    }
private:
    json_type type = JSON_NULL;
    double number;
    string str;
    vector<json_value> array;
    map<string, json_value> object;
};

class json_value_parse {
public:
    void set_json_source(const string& source, json_value* v) {
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
    int parse_value(json_value* element = nullptr) {
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

    int parse_literal(const char* dst, json_type type, json_value* element = nullptr) {
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

    int parse_number(json_value* element = nullptr) {
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

    int parse_string(json_value* element = nullptr) {
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

    int parse_array(json_value* element = nullptr) {
        it++;
        vector<json_value> tmp_array;
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
            json_value tmp_element;
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

    int parse_object(json_value* element = nullptr) {
        it++;
        skip_blank();
        int ret = 0;
        map<string, json_value> tmp_object;
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
            json_value tmp_value;
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
    json_value* value;
};

class json_parser : public json_value_parse {
private:
    json_parser() {}
public:
    static json_parser* get_parser(const string& source, json_value* v){
        static json_parser singleton_parser;
        singleton_parser.set_json_source(source, v);
        // use singleton create parser
        return &singleton_parser;
    }

    json_parser(const json_parser& ban_parser) = delete;

    json_parser& operator=(const json_parser& ban_parser) = delete;
};

class json {
public:
    json() {}

    json(const char* str) {
        json_source += str;
    }

    json(const string& str) {
        json_source = str;
    }

    int parse() {
        if (json_source == "") {
            cout << "please input json text to parse" << endl;
            return PARSE_INVALID_VALUE;
        }
        parser = json_parser::get_parser(json_source, &value);
        int ret = parser->parse();
        parser = nullptr;
        return ret;
    }

    int parse(const string& str) {
        parser = json_parser::get_parser(str, &value);
        int ret = parser->parse();
        parser = nullptr;
        return ret;
    }

    json_value& get_value() {
        return value;
    }

    int get_type() {
        return value.get_type();
    }

    double get_number() {
        return value.get_number();
    }

    string get_string() {
        return value.get_string();
    }

    void set_string(string str) {
        value.set_string(str);
    }

    json_value get_array_element(size_t index) {
        return value.get_array()[index];
    }

    size_t get_array_size() {
        return value.get_array().size();
    }

    json_value& get_object(string key) {
        return value.get_object()[key];
    }
private:
    json_value value;
    string json_source;
    json_parser* parser = nullptr;
};

#endif
