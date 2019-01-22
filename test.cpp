#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "json.hpp"

using namespace std;

int main_ret = 0;
int test_count = 0;
int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual) \
    do{\
        test_count++;\
        if (equality) test_pass++;\
        else{\
            cout << __FILE__ << ":" << __LINE__ << ": expect:" << expect << " actual: " << actual << endl;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), expect, actual)
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), expect, actual)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual)

static void test_parse_null() {
    Reader reader;
    Value value;
    EXPECT_EQ_INT(PARSE_OK, reader.parse("null", value));    
    EXPECT_EQ_INT(JSON_NULL, value.get_type());
}

static void test_parse_true() {
    Reader reader;
    Value value;
    EXPECT_EQ_INT(PARSE_OK, reader.parse("true", value));
    EXPECT_EQ_INT(JSON_TRUE, value.get_type());
}

static void test_parse_false() {
    Reader reader;
    Value value;
    EXPECT_EQ_INT(PARSE_OK, reader.parse("false", value));
    EXPECT_EQ_INT(JSON_FALSE, value.get_type());
}

#define TEST_NUMBER(expect, json_source) \
    do{\
        Reader reader;\
        Value value;\
        EXPECT_EQ_INT(PARSE_OK, reader.parse(json_source, value));\
        EXPECT_EQ_INT(JSON_NUMBER, value.get_type());\
        EXPECT_EQ_DOUBLE(expect, value.asDouble());\
    } while (0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(1.0, "1.0");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(1.234e-10, "1.234E-10");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324" ); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308" );  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308" );  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308" );  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json_source) \
    do {\
        Reader reader;\
        Value value;\
        string dst = expect;\
        EXPECT_EQ_INT(PARSE_OK, reader.parse(json_source, value));\
        EXPECT_EQ_INT(JSON_STRING, value.get_type());\
        EXPECT_EQ_STRING(dst, value.asString());\
    } while (0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    // this example parse right, but give "Hello\0World" to class string tmp,
    // tmp will become "Hello", result in different result....
    // TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_access_string() {
    Reader reader;
    Value value;
    value.set_string("");
    EXPECT_EQ_STRING("", value.asString());
    value.set_string("Hello");
    EXPECT_EQ_STRING("Hello", value.asString());
}

static void test_parse_array() {
    Reader reader;
    Value value;
    EXPECT_EQ_INT(PARSE_OK, reader.parse("[   ]", value));
    EXPECT_EQ_INT(JSON_ARRAY, value.get_type());
    EXPECT_EQ_SIZE_T(0, value.get_array().size());

    EXPECT_EQ_INT(PARSE_OK, reader.parse("[  null , false , true , 123 , \"abc\"  ]", value));
    EXPECT_EQ_INT(JSON_ARRAY, value.get_type());
    EXPECT_EQ_SIZE_T(5, value.get_array().size());
    EXPECT_EQ_INT(JSON_NULL, value[0].get_type());
    EXPECT_EQ_INT(JSON_FALSE, value[1].get_type());
    EXPECT_EQ_INT(JSON_TRUE, value[2].get_type());
    EXPECT_EQ_INT(JSON_NUMBER, value[3].get_type());
    EXPECT_EQ_INT(JSON_STRING, value[4].get_type());
    EXPECT_EQ_DOUBLE(123.0, value[3].asDouble());
    EXPECT_EQ_STRING("abc", value[4].asString());

    EXPECT_EQ_INT(PARSE_OK, reader.parse("[ [  ] , [ 0  ] , [ 0 , 1  ] , [ 0 , 1 , 2  ]  ]", value));
    EXPECT_EQ_INT(JSON_ARRAY, value.get_type());
    EXPECT_EQ_SIZE_T(4, value.size());
    EXPECT_EQ_INT(JSON_ARRAY, value[0].get_type());
}

void test_parse_object() {
    Reader reader;
    Value value;
    EXPECT_EQ_INT(PARSE_OK, reader.parse("{"
                                         "\"n\" : null , "
                                         "\"f\" : false , "
                                         "\"t\" : true , "
                                         "\"i\" : 123 , "
                                         "\"s\" : \"abc\", "
                                         "\"a\" : [ 1, 2, 3  ],"
                                         "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3  }"
                                         " } "
                                         , value));
    EXPECT_EQ_INT(JSON_OBJECT, value.get_type());
    EXPECT_EQ_INT(JSON_NULL, value["n"].get_type());
    EXPECT_EQ_INT(JSON_FALSE, value["f"].get_type());
    EXPECT_EQ_INT(JSON_TRUE, value["t"].get_type());

    EXPECT_EQ_INT(JSON_NUMBER, value["i"].get_type());
    EXPECT_EQ_DOUBLE(123, value["i"].asDouble());

    EXPECT_EQ_INT(JSON_STRING, value["s"].get_type());
    EXPECT_EQ_STRING("abc", value["s"].asString());

    EXPECT_EQ_INT(JSON_ARRAY, value["a"].get_type());
    EXPECT_EQ_INT(JSON_OBJECT, value["o"].get_type());
}

#define TEST_ERROR(error, json_source) \
    do{\
        Reader reader;\
        Value value;\
        EXPECT_EQ_INT(error, reader.parse(json_source, value));\
        EXPECT_EQ_INT(JSON_NULL, value.get_type());\
    } while(0)

static void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_OVERFLOW, "1e309");
    TEST_ERROR(PARSE_NUMBER_OVERFLOW, "-1e309");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
}

static void test_convert() {
    Reader reader;
    Value value;
    string json_source("{  "
                        "\"n\" : null , "
                        "\"f\" : false , "
                        "\"t\" : true , "
                        "\"i\" : 123 , "
                        "\"s\" : \"abc\", "
                        "\"a\" : [ 1, 2, 3  ],"
                        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3  }"
                      " } "
                      );
    EXPECT_EQ_INT(PARSE_OK, reader.parse(json_source, value));
    EXPECT_EQ_INT(JSON_OBJECT, value.get_type());
    FastWriter fw;
    string str = fw.toStyledString(value);
    cout << str << endl;
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    test_convert();
    return main_ret;
}
