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
    json j;
    EXPECT_EQ_INT(PARSE_OK, j.parse("null"));
    EXPECT_EQ_INT(JSON_NULL, j.get_type());
}

static void test_parse_true() {
    json j;
    EXPECT_EQ_INT(PARSE_OK, j.parse("true"));
    EXPECT_EQ_INT(JSON_TRUE, j.get_type());
}

static void test_parse_false() {
    json j;
    EXPECT_EQ_INT(PARSE_OK, j.parse("false"));
    EXPECT_EQ_INT(JSON_FALSE, j.get_type());
}

#define TEST_NUMBER(expect, json_source) \
    do{\
        json j;\
        EXPECT_EQ_INT(PARSE_OK, j.parse(json_source));\
        EXPECT_EQ_INT(JSON_NUMBER, j.get_type());\
        EXPECT_EQ_DOUBLE(expect, j.get_number());\
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
        json j;\
        string dst = expect;\
        EXPECT_EQ_INT(PARSE_OK, j.parse(json_source));\
        EXPECT_EQ_INT(JSON_STRING, j.get_type());\
        EXPECT_EQ_STRING(dst, j.get_string());\
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
    json j;
    j.set_string("");
    EXPECT_EQ_STRING("", j.get_string());
    j.set_string("Hello");
    EXPECT_EQ_STRING("Hello", j.get_string());
}

static void test_parse_array() {
    json j;
    EXPECT_EQ_INT(PARSE_OK, j.parse("[   ]"));
    EXPECT_EQ_INT(JSON_ARRAY, j.get_type());
    EXPECT_EQ_SIZE_T(0, j.get_array_size());

    EXPECT_EQ_INT(PARSE_OK, j.parse("[  null , false , true , 123 , \"abc\"  ]"));
    EXPECT_EQ_INT(JSON_ARRAY, j.get_type());
    EXPECT_EQ_SIZE_T(5, j.get_array_size());
    EXPECT_EQ_INT(JSON_NULL, j.get_array_element(0).get_type());
    EXPECT_EQ_INT(JSON_FALSE, j.get_array_element(1).get_type());
    EXPECT_EQ_INT(JSON_TRUE, j.get_array_element(2).get_type());
    EXPECT_EQ_INT(JSON_NUMBER, j.get_array_element(3).get_type());
    EXPECT_EQ_INT(JSON_STRING, j.get_array_element(4).get_type());
    EXPECT_EQ_DOUBLE(123.0, j.get_array_element(3).get_number());
    EXPECT_EQ_STRING("abc", j.get_array_element(4).get_string());

    EXPECT_EQ_INT(PARSE_OK, j.parse("[ [  ] , [ 0  ] , [ 0 , 1  ] , [ 0 , 1 , 2  ]  ]"));
    EXPECT_EQ_INT(JSON_ARRAY, j.get_type());
    EXPECT_EQ_SIZE_T(4, j.get_array_size());
    EXPECT_EQ_INT(JSON_ARRAY, j.get_array_element(0).get_type());
}

void test_parse_object() {
    json j("{  "
           "\"n\" : null , "
           "\"f\" : false , "
           "\"t\" : true , "
           "\"i\" : 123 , "
           "\"s\" : \"abc\", "
           "\"a\" : [ 1, 2, 3  ],"
           "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3  }"
           " } "
          );
    EXPECT_EQ_INT(PARSE_OK, j.parse());
    EXPECT_EQ_INT(JSON_OBJECT, j.get_type());
    EXPECT_EQ_INT(JSON_NULL, j.get_object("n").get_type());
    EXPECT_EQ_INT(JSON_FALSE, j.get_object("f").get_type());
    EXPECT_EQ_INT(JSON_TRUE, j.get_object("t").get_type());
    json_value v = j.get_object("i");
    EXPECT_EQ_INT(JSON_NUMBER, v.get_type());
    EXPECT_EQ_DOUBLE(123, v.get_number());
    v = j.get_object("s");
    EXPECT_EQ_INT(JSON_STRING, v.get_type());
    EXPECT_EQ_STRING("abc", v.get_string());
    v = j.get_object("a");
    EXPECT_EQ_INT(JSON_ARRAY, v.get_type());
    EXPECT_EQ_INT(JSON_OBJECT, j.get_object("o").get_type());
}

#define TEST_ERROR(error, json_source) \
    do{\
        json j;\
        EXPECT_EQ_INT(error, j.parse(json_source));\
        EXPECT_EQ_INT(JSON_NULL, j.get_type());\
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

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
