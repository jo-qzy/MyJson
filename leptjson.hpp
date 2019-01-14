/*****************************************
 本文件是看github项目写的练习，之后会在
 json.hpp文件内用c++重新自己尝试练习
 *****************************************/

#ifndef __LEPTJSON_HPP__
#define __LEPTJSON_HPP__

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

enum error_num
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

enum lept_type
{
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_OBJECT,
    LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY
};

struct lept_value
{
    lept_type type;
};

struct lept_context
{
    const char* json;
};

#define EXPECT(c, ch) do{ assert(*c->json == ch); c->json++; }while(0)

static void lept_parse_whitespace(lept_context* c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
    {
        p++;
    }
    c->json = p;
}

static int lept_parse_null(lept_context* c, lept_value* v)
{
    int ret = strncmp(c->json, "null", strlen("null"));
    if (ret != 0)
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v)
{
    int ret = strncmp(c->json, "true", strlen("true"));
    if (ret != 0)
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v)
{
    int ret = strncmp(c->json, "false", strlen("false"));
    if (ret != 0)
    {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += strlen("false");
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v)
{
    switch (*c->json)
    {
    case 'n':
        return lept_parse_null(c, v);
    case 'f':
        return lept_parse_false(c, v);
    case 't':
        return lept_parse_true(c, v);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    default:
        return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    int ret = lept_parse_value(&c, v);
    if (ret == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if (*(c.json) != '\0')
        {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v)
{
    return v->type;
}

#endif
