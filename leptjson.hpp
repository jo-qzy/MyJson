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
#include <cmath>

enum error_num
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_OVERFLOW,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE
};

enum lept_type
{
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_OBJECT,
    LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY
};

struct lept_value
{
    union
    {
        struct
        {
            char* str = NULL;
            size_t len;
        };
        double num;
    };
    lept_type type;
};

struct lept_context
{
    const char* json;
    char* s;
    size_t top, size;
};

#define EXPECT(c, ch) do{ assert(*c->json == ch); c->json++; }while(0)

static void lept_parse_whitespace(lept_context* c)
{
    const char* p = c->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type)
{
    int ret = strncmp(c->json, literal, strlen(literal));
    if (ret != 0)
        return LEPT_PARSE_INVALID_VALUE;
    c->json += strlen(literal);
    v->type = type;
    return LEPT_PARSE_OK;
}

#define ISDIGIT(num) ((num >= '0') && (num <= '9'))
#define ISDIGIT1TO9(num) ((num >= '1') && (num <= '9'))

static int lept_parse_num(lept_context* c, lept_value* v)
{
    assert((c->json != NULL) || (*c->json <= '0' && *c->json >= '9'));
    const char* p = c->json;
    if (*p == '-')
        p++;
    if (*p == '0')
        p++;
    else
    {
        if (!ISDIGIT1TO9(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p =='-')
            p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p != '\0' && *p != ' ')
        return LEPT_PARSE_INVALID_VALUE;
    errno = 0;
    char* end = NULL;
    v->num = strtod(c->json, &end);
    if (errno == ERANGE && (v->num == HUGE_VAL || v->num == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_OVERFLOW;
    v->type = LEPT_NUMBER;
    c->json = end;
    if (*c->json != '\0')
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    return LEPT_PARSE_OK;
}

static void* lept_context_push(lept_context* c, size_t size)
{
    void* ret = NULL;
    ret = c->s + c->top;
    c->top += size;
    return ret;
}

static void* lept_context_pop(lept_context* c, size_t size)
{
    return c->s + (c->top -= size);
}

void lept_set_string(lept_value* v, const char* s, size_t len);

static const char* lept_parse_hex4(const char* p, unsigned* u)
{
    *u = 0;
    for (int i = 0; i < 4; i++)
    {
        *u <<= 4;
        char ch = *p++;
        if (ch >= '0' && ch <= '9')
            *u |= ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            *u |= ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            *u |= ch - 'A' + 10;
        else
            return NULL;
    }
    return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u)
{
    if (u <= 0x7f)
        *(char*)lept_context_push(c, 1) = (u & 0xff);
    else if (u <= 0x7ff)
    {
        *(char*)lept_context_push(c, 1) = (0xc0 | (0xff & (u >> 6)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & u));
    }
    else if (u <= 0xffff)
    {
        *(char*)lept_context_push(c, 1) = (0xe0 | (0xff & (u >> 12)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & (u >> 6)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & u));
    }
    else
    {
        assert(u <= 0x10ffff);
        *(char*)lept_context_push(c, 1) = (0xf0 | (0xff & (u >> 18)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & (u >> 12)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & (u >> 6)));
        *(char*)lept_context_push(c, 1) = (0x80 | (0x3f & u));
    }
}

static int lept_parse_string(lept_context* c, lept_value* v)
{
    size_t head = c->top, len = 0;
    const char* p = c->json + 1;
    for (;;)
    {
        char ch = *p++;
        switch (ch)
        {
        case '\"':
            len = c->top - head;
            lept_set_string(v, (const char*)lept_context_pop(c, len), len);
            c->json = p;
            return LEPT_PARSE_OK;
        case '\\':
            switch (*p++)
            {
            case '\"': *(char*)lept_context_push(c, 1) = '\"'; break;
            case '\\': *(char*)lept_context_push(c, 1) = '\\'; break;
            case '/': *(char*)lept_context_push(c, 1) = '/'; break;
            case 'b': *(char*)lept_context_push(c, 1) = '\b'; break;
            case 'n': *(char*)lept_context_push(c, 1) = '\n'; break;
            case 'f': *(char*)lept_context_push(c, 1) = '\f'; break;
            case 'r': *(char*)lept_context_push(c, 1) = '\r'; break;
            case 't': *(char*)lept_context_push(c, 1) = '\t'; break;
            case 'u':
                unsigned u;
                unsigned u_tmp;
                if (!(p = lept_parse_hex4(p, &u)))
                {
                    c->top = head;
                    return LEPT_PARSE_INVALID_UNICODE_HEX;
                }
                if (u >= 0xd800 && u <= 0xdbff)
                {
                    if (*p++ != '\\' || *p++ != 'u')
                    {
                        c->top = head;
                        return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                    }
                    if (!(p = lept_parse_hex4(p, &u_tmp)))
                    {
                        c->top = head;
                        return LEPT_PARSE_INVALID_UNICODE_HEX;
                    }
                    if (u_tmp < 0xdc00 || u_tmp > 0xdfff)
                    {
                        c->top = head;
                        return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                    }
                    u = 0x10000 + (u - 0xd800) * 0x400 + (u_tmp - 0xdc00);
                }
                lept_encode_utf8(c, u);
                break;
            default: c->top = head; return LEPT_PARSE_INVALID_STRING_ESCAPE;
            }
            break;
        case '\0':
            c->top = head;
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        default:
            if (ch < 0x20)
            {
                c->top = head;
                return LEPT_PARSE_INVALID_STRING_CHAR;
            }
            *(char*)lept_context_push(c, 1) = ch;
        }
    }
}

static int lept_parse_value(lept_context* c, lept_value* v)
{
    switch (*c->json)
    {
    case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
    case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case '\"': return lept_parse_string(c, v);
    case '\0': return LEPT_PARSE_EXPECT_VALUE;
    default:
        if ((*c->json >= '0' && *c->json <= '9') || *c->json == '-')
            return lept_parse_num(c, v);
        return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json)
{
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    c.s = (char*)malloc(100);
    c.size = 100;
    c.top = 0;

    lept_parse_whitespace(&c);
    int ret = lept_parse_value(&c, v);
    if (ret == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if (*(c.json) != '\0')
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    free(c.s);
    return ret;
}

lept_type lept_get_type(const lept_value* v)
{
    return v->type;
}

double lept_get_num(const lept_value* v)
{
    return v->num;
}

void lept_set_num(lept_value* v, double num)
{
    if (v->str)
        free(v->str);
    v->num = num;
    v->type = LEPT_NUMBER;
}

int lept_get_boolean(const lept_value* v)
{
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, bool b)
{
    if (v->str)
        free(v->str);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

char* lept_get_string(lept_value* v)
{
    return v->str;
}

void lept_set_string(lept_value* v, const char* s, size_t len)
{
    if (v->str)
        free(v->str);
    v->str = (char*)malloc(len + 1);
    memcpy(v->str, s, len);
    (v->str)[len] = '\0';
    v->len = len;
    v->type = LEPT_STRING;
}

int lept_get_string_length(lept_value* v)
{
    return v->len;
}

#endif
