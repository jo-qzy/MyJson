#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <iostrem>
#include <string>
#include <vector>

using namespace std;

// error number
enum error_number {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGLUAR
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
    json_type type = JSON_NULL;
    double number;
    string str;
    list<json_value> list;
    map<string, json_value>* object = nullptr;
public:
    ~json_value() {
        if (type == JSON_OBJECT)
            delete object;
    }
};

class json_value_parse {
public:
    void set_json_source(const string& source, json_value* v) {
        json_source = str;
        it = json_source.begin();
        value = v;
    }

    int parse()
    {
        skip_whitespace();
        int ret = 0;
        if ((ret = parse_value()) == PARSE_OK) {
            skip_whitespace();
            if (it != json_end())
                return PARSE_ROOT_NOT_SINGLUAR;
        }
        return ret;
    }
private:
    int parse_value() {
        if (it == json_source.end())
            return PARSE_EXPECT_VALUE;
        switch (*it) {
        case 'n': return parse_literal("null", JSON_NULL);
        case 't': return parse_literal("true", JSON_TRUE);
        case 'f': return parse_literal("false", JSON_FALSE);
        case '[': return parse_array();
        case '{': return parse_object();
        case '\"': return parse_string();
        case '\0': return PARSE_EXPECT_VALUE;
        default:
            if (*it == '-' || (*it >= '0' && *it <= '9')) {
                return parse_number();
            }
            return PARSE_INVALID_VALUE;
        }
    }

    void skip_whitespace() {
        while (it != json_source.end() && *it == ' ')
            it++;
    }

    int parse_literal(const char* dst, json_type type) {
        int len = strlen(dst);
        if (strncmp(&(*it), dst, len) == 0) {
            it += len;
            value->type = type;
            return PARSE_OK;
        }
        else
            return PARSE_INVALID_VALUE;
    }

#define ISDIGIT(num) ((num >= '0') && (num <= '9'))
#define ISDIGIT1TO9(num) ((num >= '1') && (num <= '9'))

    int parse_number() {
        const string::iterator tmp_it = it;
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
            if (!ISDIGIT(*tmp_it))
                return PARSE_INVALID_VALUE;
            for (tmp_it++; ISDIGIT(*tmp_it); tmp_it++);
        }
        if (*tmp_it != ' ' && *tmp_it != '\0')
            return PARSE_INVALID_VALUE;
        value->number = strtod(&(*it), NULL);
        it = tmp_it;
        value->type = JSON_NUMBER;
        return PARSE_OK;
    }

#define CHECK_ITERATOR(it) do { if (it == json_source.end()) return PARSE_MISS_QUOTATION_MARK; } while(0)

    int parse_string() {
        string::iterator tmp_it = it + 1;
        string tmp_str;
        char ch = 0;
        while (tmp_it != json_source.end()) {
            ch = *tmp_it++;
            switch (ch) {
            case '\"':
                set_string(tmp_str);
                it = tmp_it + 1;
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
                    case 'u': 
                        unsigned u = 0;
                        int ret = 0;
                        if ((ret = parse_hex4(tmp_it, u)) != PARSE_OK)
                            return ret;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            CHECK_ITERATOR(tmp_it);
                            if (*tmp_it++ != '\\')
                                return PARSE_INVALID_UNICODE_HEX;
                            CHECK_ITERATOR(tmp_it);
                            if (*tmp_it++ != 'u')
                                return PARSE_INVALID_UNICODE_HEX;
                            unsigned tmp_u;
                            if ((ret == parse_hex4(tmp_it, tmp_u)) != PARSE_OK)
                                return ret;
                            u = 0x10000 + (u - 0xD800) * 0x400 + (tmp_u - 0xDC00);
                        }
                        if (u > 0x10FFFF)
                            return PARSE_INVALID_UNICODE_HEX;
                        encode_utf8(u, tmp_str);
                        break;
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

    int parse_array()
    {}

    int parse_object() {
        return PARSE_OK;
    }

    void set_string(const string& str) {
        value->type = JSON_STRING;
        value->str = str;
    }

    int parse_hex4(string::iterator& tmp_it, unsigned& u) {
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
            tmp_str += (0xe0 | (0xff & (u >> 12)));
            tmp_str += (0x80 | (0x3f & (u >> 6)));
            tmp_str = (0x80 | (0x3f & u));
        }
        else {
            tmp_str = (0xf0 | (0xff & (u >> 18)));
            tmp_str = (0x80 | (0x3f & (u >> 12)));
            tmp_str = (0x80 | (0x3f & (u >> 6)));
            tmp_str = (0x80 | (0x3f & u));
        }    
    }
private:
    string json_source;
    const string::iterator it;
    json_value* value;
};

class josn_parser : public json_value_parse {
private:
    json_parser() {}
public:
    static json_parser* get_parser(const string& source, json_value* v){
        static json_object_parse singleton_parser;
        set_json_source(source, v);
        // use singleton create parser
        return &singleton_parser;
    }

    json_parser(const json_parser&) = delete;

    json_parser& operator=(const json_parse&) = delete;
}

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
        if (json_source == "")
        {
            fprint(2, "please input json text to parse");
            return PARSE_INVALID_VALUE;
        }
        parser = json_parser::get_parser(json_source, &value);
        return parser->parse();
    }

    int parse(const string& str) {
        parser = json_parser::get_parser(str, &value);
        return parser->parse();
    }
private:
    json_value value;
    string json_source;
    json_parser* parser;
};

#endif
