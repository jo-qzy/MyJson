#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <iostrem>
#include <string>
#include <vector>

using namespace std;

#define PARSE_OK 0
#define PARSE_EXPECT_VALUE 1
#define PARSE_INVALID_VALUE 2
#define PARSE_ROT_NOT_SINGLUAR 3

enum json_type
{
    JSON_NULL, JSON_TRUE, JSON_FALSE,
    JSON_NUMBER, JSON_STRING, JSON_ARRAY,
    JSON_OBJECT
};

struct json_value
{
    json_type type;
};

struct context
{
    const string json;
    const string::iterator it;
};

class json
{
public:
    int parse_json(json_value* v, const string &json)
    {
        c->json = json;
        it = _c->json.begin();
        json->type = JSON_NULL;
        parse_blank();
        return parse_value(v);
    }
private:
    void parse_blank()
    {
        c.it = 
    }
private:
    context _c;
};

#endif
