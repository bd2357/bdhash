#include "unity.h"

#include "hash_reg.h"
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_hash_reg_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement hash_reg");
}

void test_make_jthash_str(void)
{
    char *str = "Hello There.";
    TEST_ASSERT_EQUAL(49, make_jthash(str, strlen(str)));
    str = "This is a longer string than Hello There.";
    TEST_ASSERT_EQUAL(51, make_jthash(str, strlen(str)));
    str = "ss"; // short string
    TEST_ASSERT_EQUAL(44, make_jthash(str, strlen(str)));
    char ns = 0;
    str = &ns; // null string should be zero
    TEST_ASSERT_EQUAL(0, make_jthash(str, strlen(str)));
}

void test_make_jthash_int(void)
{
    int val = 42;
    TEST_ASSERT_EQUAL(34, make_jthash(&val, sizeof val) );
}

void test_make_jthash_uint(void)
{
    uint32_t val = 42;
    TEST_ASSERT_EQUAL(34, make_jthash(&val, sizeof val) );
}

void test_make_jthash_int16(void)
{
    int16_t val = 42;
    TEST_ASSERT_EQUAL(90, make_jthash(&val, sizeof val) );
}

void test_package_jtkey_str(void)
{
    char *str = "Hello There.";
    jtkey_t package = package_jtkey(str, strlen(str));
    TEST_ASSERT_EQUAL(49, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    TEST_ASSERT_EQUAL_PTR(str, (char*)package.key);
}

void test_package_jtkey_long_str(void)
{
    char *str = "This is a longer string than Hello There.";
    jtkey_t package = package_jtkey(str, strlen(str));
    TEST_ASSERT_EQUAL(51, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    TEST_ASSERT_EQUAL_PTR(str, (char*)package.key);
}

void test_package_jtkey_short_str(void)
{
    char *str = "ss"; // short string
    jtkey_t package = package_jtkey(str, strlen(str));
    TEST_ASSERT_EQUAL(44, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    // str was copied
    TEST_ASSERT_EQUAL_MEMORY(str, &package.key, strlen(str));
}

void test_package_jtkey_none(void)
{
    char *str = ""; // null str
    jtkey_t package = package_jtkey(str, strlen(str));
    TEST_ASSERT_EQUAL(0, package.hash);
    TEST_ASSERT_EQUAL(strlen(str), package.len);
    // str was copied
    TEST_ASSERT_EQUAL(0, package.key); // should be 0
}

void test_test_jtkey(void)
{
    char *str = "This is a longer string than Hello There.";
    jtkey_t str_key = package_jtkey(str, strlen(str));
    jtkey_t null_key = package_jtkey("", strlen(""));
    jtkey_t val_key = package_jtkey(&(int){42}, sizeof(int));
    TEST_ASSERT_TRUE(test_jtkey(&null_key, ""));
    TEST_ASSERT_TRUE(test_jtkey(&str_key, str));
    TEST_ASSERT_FALSE(test_jtkey(&str_key, "differnt string"));
    TEST_ASSERT_TRUE(test_jtkey(&val_key, &(int){42}));
    TEST_ASSERT_FALSE(test_jtkey(&val_key, &(int){1234}));
}

void test_compare_jtkey(void)
{
    char *str = "This is a longer string than Hello There.";
    jtkey_t str1_key = package_jtkey(str, strlen(str));
    jtkey_t str2_key = package_jtkey(str, strlen(str));
    jtkey_t str3_key = package_jtkey("short string", strlen("short string"));
    jtkey_t null_key = package_jtkey("", strlen(""));
    jtkey_t null2_key = package_jtkey(&(int){85}, 0);
    jtkey_t val1_key = package_jtkey(&(uint8_t){42}, sizeof(uint8_t));
    jtkey_t val2_key = package_jtkey(&(uint8_t){42}, sizeof(uint8_t));
    jtkey_t val3_key = package_jtkey(&(uint8_t){143}, sizeof(uint8_t));

    TEST_ASSERT_TRUE(compare_jtkey(&str1_key, &str2_key));
    TEST_ASSERT_FALSE(compare_jtkey(&str1_key, &str3_key));
    TEST_ASSERT_TRUE(compare_jtkey(&null_key, &null2_key));
    TEST_ASSERT_TRUE(compare_jtkey(&val1_key, &val2_key));
    TEST_ASSERT_FALSE(compare_jtkey(&val1_key, &val3_key));


}
