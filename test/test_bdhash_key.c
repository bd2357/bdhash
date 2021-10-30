#include "unity.h"

#include "bdhash_key.h"
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bdhash_key_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement bdhash_key");
}

void test_make_bdhash_str(void)
{
    char *str = "Hello There.";
    TEST_ASSERT_EQUAL(49, make_bdhash(str, strlen(str)));
    str = "This is a longer string than Hello There.";
    TEST_ASSERT_EQUAL(51, make_bdhash(str, strlen(str)));
    str = "ss"; // short string
    TEST_ASSERT_EQUAL(44, make_bdhash(str, strlen(str)));
    char ns = 0;
    str = &ns; // null string should be zero
    TEST_ASSERT_EQUAL(0, make_bdhash(str, strlen(str)));
}

void test_make_bdhash_int(void)
{
    int val = 42;
    TEST_ASSERT_EQUAL(34, make_bdhash(&val, sizeof val) );
}

void test_make_bdhash_uint(void)
{
    uint32_t val = 42;
    TEST_ASSERT_EQUAL(34, make_bdhash(&val, sizeof val) );
}

void test_make_bdhash_int16(void)
{
    int16_t val = 42;
    TEST_ASSERT_EQUAL(90, make_bdhash(&val, sizeof val) );
}

void test_package_bdkey_str(void)
{
    char *str = "Hello There.";
    bdkey_t package = package_bdkey(str, strlen(str));
    TEST_ASSERT_EQUAL(49, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    TEST_ASSERT_EQUAL_PTR(str, (char*)package.key);
}

void test_package_bdkey_long_str(void)
{
    char *str = "This is a longer string than Hello There.";
    bdkey_t package = package_bdkey(str, strlen(str));
    TEST_ASSERT_EQUAL(51, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    TEST_ASSERT_EQUAL_PTR(str, (char*)package.key);
}

void test_package_bdkey_short_str(void)
{
    char *str = "ss"; // short string
    bdkey_t package = package_bdkey(str, strlen(str));
    TEST_ASSERT_EQUAL(44, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    // str was copied
    TEST_ASSERT_EQUAL_MEMORY(str, &package.key, strlen(str));
}

void test_package_bdkey_none(void)
{
    char *str = ""; // null str
    bdkey_t package = package_bdkey(str, strlen(str));
    TEST_ASSERT_EQUAL(0, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    // str was copied
    TEST_ASSERT_EQUAL(0, package.key); // should be 0
}

void test_test_bdkey(void)
{
    char *str = "This is a longer string than Hello There.";
    bdkey_t str_key = package_bdkey(str, strlen(str));
    bdkey_t null_key = package_bdkey("", strlen(""));
    bdkey_t val_key = package_bdkey(&(int){42}, sizeof(int));
    TEST_ASSERT_TRUE(test_bdkey(&null_key, ""));
    TEST_ASSERT_TRUE(test_bdkey(&str_key, str));
    TEST_ASSERT_FALSE(test_bdkey(&str_key, "differnt string"));
    TEST_ASSERT_TRUE(test_bdkey(&val_key, &(int){42}));
    TEST_ASSERT_FALSE(test_bdkey(&val_key, &(int){1234}));
}

void test_compare_bdkey(void)
{
    char *str = "This is a longer string than Hello There.";
    bdkey_t str1_key = package_bdkey(str, strlen(str));
    bdkey_t str2_key = package_bdkey(str, strlen(str));
    bdkey_t str3_key = package_bdkey("short string", strlen("short string"));
    bdkey_t null_key = package_bdkey("", strlen(""));
    bdkey_t null2_key = package_bdkey(&(int){85}, 0);
    bdkey_t val1_key = package_bdkey(&(uint8_t){42}, sizeof(uint8_t));
    bdkey_t val2_key = package_bdkey(&(uint8_t){42}, sizeof(uint8_t));
    bdkey_t val3_key = package_bdkey(&(uint8_t){143}, sizeof(uint8_t));

    TEST_ASSERT_TRUE(compare_bdkey(&str1_key, &str2_key));
    TEST_ASSERT_FALSE(compare_bdkey(&str1_key, &str3_key));
    TEST_ASSERT_TRUE(compare_bdkey(&null_key, &null2_key));
    TEST_ASSERT_TRUE(compare_bdkey(&val1_key, &val2_key));
    TEST_ASSERT_FALSE(compare_bdkey(&val1_key, &val3_key));


}
