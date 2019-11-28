/*
 * Tests for Json formatting
 *
 *  Copyright (c) 2019 AT&T Intellectual Property.
 *  Copyright (c) 2018-2019 Nokia.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This source code is part of the near-RT RIC (RAN Intelligent Controller)
 *  platform project (RICP).
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#if HAVE_JSONCPP
#include <json/json.h>
#endif

#include <stdarg.h>

#include "private/json_format.h"

using namespace testing;

extern "C" {
size_t format_timestamp(char* buffer, size_t len, struct timeval* tv);
size_t format_severity(char* buffer, size_t len, mdclog_severity_t severity);
size_t format_identity(char* buffer, size_t len, const char* identity);
size_t format_message(char* buffer, size_t len, const char* msg, va_list arglist);
size_t format_mdc(char* buffer, size_t len, mdc_t* mdc );
int format_log_entry(char* buffer,
                       size_t len,
                       struct timeval* timestamp,
                       const char* identity,
                       mdclog_severity_t severity,
                       mdc_t* mdc,
                       const char* msg,
                       va_list arglist);
}

class FormatTimestampTest: public testing::Test
{
public:
    struct timeval tv = {1550667066, 123456};
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    const char* expected_str = "\"ts\":1550667066123";
};

TEST_F(FormatTimestampTest, TimestampIsFormattedCorrectly)
{
    len = format_timestamp(buffer, strlen(expected_str) + 1, &tv);  // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatTimestampTest, TimestampCannotBeFormattedToTooShortBuffer)
{
    len = format_timestamp(buffer, strlen(expected_str), &tv);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

class FormatSeverityTest: public testing::Test
{
public:
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    const char* expected_err_str = "\"crit\":\"ERROR\"";
    const char* expected_warn_str = "\"crit\":\"WARNING\"";
    const char* expected_info_str = "\"crit\":\"INFO\"";
    const char* expected_debug_str = "\"crit\":\"DEBUG\"";
};

TEST_F(FormatSeverityTest, SeverityIsFormattedCorrectly)
{
    len = format_severity(buffer, strlen(expected_err_str) + 1, MDCLOG_ERR);
    EXPECT_EQ(len, strlen(expected_err_str));
    EXPECT_THAT(buffer, StrEq(expected_err_str));

    len = format_severity(buffer, strlen(expected_warn_str) + 1, MDCLOG_WARN);
    EXPECT_EQ(len, strlen(expected_warn_str));
    EXPECT_THAT(buffer, StrEq(expected_warn_str));

    len = format_severity(buffer, strlen(expected_info_str) + 1, MDCLOG_INFO);
    EXPECT_EQ(len, strlen(expected_info_str));
    EXPECT_THAT(buffer, StrEq(expected_info_str));

    len = format_severity(buffer, strlen(expected_debug_str) + 1, MDCLOG_DEBUG);
    EXPECT_EQ(len, strlen(expected_debug_str));
    EXPECT_THAT(buffer, StrEq(expected_debug_str));
}

TEST_F(FormatSeverityTest, InvalidSeverityValueIsNotFormaated)
{
    len = format_severity(buffer, sizeof(buffer), static_cast<mdclog_severity_t>(5));
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

TEST_F(FormatSeverityTest, SeverityCannotBeFormattedToTooShortBuffer)
{
    len = format_severity(buffer, strlen(expected_err_str), MDCLOG_ERR);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);

    len = format_severity(buffer, strlen(expected_warn_str), MDCLOG_WARN);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);

    len = format_severity(buffer, strlen(expected_info_str), MDCLOG_INFO);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);

    len = format_severity(buffer, strlen(expected_debug_str), MDCLOG_DEBUG);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

class FormatIdentityTest: public testing::Test
{
public:
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    const char* expected_str = "\"id\":\"Donald Duck\"";
};

TEST_F(FormatIdentityTest, identityIsFormattedCorrectly)
{
    len = format_identity(buffer, strlen(expected_str) + 1, "Donald Duck"); // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatIdentityTest, IdentityCannotBeFormattedToTooShortBuffer)
{
    len = format_identity(buffer, strlen(expected_str), "Donald Duck");
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

TEST_F(FormatIdentityTest, TooLongIdentityIsCut)
{
    std::string org_identity(IDENTITY_MAX_LENGTH + 1, 'A');
    std::string cut_identity(IDENTITY_MAX_LENGTH, 'A');
    std::string expected_cut_str = "\"id\":\"" + cut_identity + "\"";

    len = format_identity(buffer, sizeof(buffer), org_identity.c_str());
    EXPECT_EQ(len, expected_cut_str.length());
    EXPECT_THAT(buffer, StrEq(expected_cut_str));
}

TEST_F(FormatIdentityTest, IdetityIsNull)
{
    std::string expected_ident = "\"id\":\"(null)\"";
    len = format_identity(buffer, sizeof(buffer), NULL);
    EXPECT_EQ(len, expected_ident.length());
    EXPECT_THAT(buffer, StrEq(expected_ident));
}

class FormatMessageTest: public testing::Test
{
public:
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    const char* expected_str = "\"msg\":\"Test log with string foobar and int 123\"";
    const char* format_str = "Test log with string %s and int %d";

    size_t test_format_message( char* buffer, size_t len, const char* fmt, ...)
    {
        size_t ret;
        va_list arglist;
        va_start(arglist, fmt);
        ret = format_message(buffer, len, fmt, arglist);
        va_end(arglist);
        return ret;
    }
};

TEST_F(FormatMessageTest, MessageIsFormattedCorrectly)
{
    len = test_format_message(buffer, strlen(expected_str) + 1, format_str, "foobar", 123); // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatMessageTest, MessageIsTruncatedToFitToBuffer)
{
    const char* truncated_str = "\"msg\":\"Test log with string foobar[truncated]\"";

    len = test_format_message(buffer, strlen(expected_str), format_str, "foobar", 123);
    EXPECT_EQ(len, strlen(truncated_str));
    EXPECT_THAT(buffer, StrEq(truncated_str));
}

TEST_F(FormatMessageTest, MessageIsTruncatedToFitToBufferFittingOnlyTruncatedText)
{
    const char* truncated_str = "\"msg\":\"[truncated]\"";

    len = test_format_message(buffer, strlen(truncated_str) + 1, format_str, "foobar", 123); // +1 for the NULL char
    EXPECT_EQ(len, strlen(truncated_str));
    EXPECT_THAT(buffer, StrEq(truncated_str));
}

TEST_F(FormatMessageTest, MessageCannotBeFormattedToTooShortBuffer)
{
    const char* min_truncated_str = "\"msg\":\"[truncated]\"";

    len = test_format_message(buffer, strlen(min_truncated_str), format_str, "foobar", 123);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

TEST_F(FormatMessageTest, BackslashAndDoubleQuoteCharsAreEscaped)
{
    const char* expected_escaped_str = R"~("msg":"Test log with string \\\"foobar\" and int 123")~";
    const char* str_arg = R"~(\"foobar")~";

    len = test_format_message(buffer, strlen(expected_escaped_str) + 1, format_str, str_arg, 123); // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_escaped_str));
    EXPECT_THAT(buffer, StrEq(expected_escaped_str));
}

TEST_F(FormatMessageTest, TooShortBufferButStringIsNotCutBetweenEscapeCharAndEscapedChar)
{
    const char* str = R"~("""""""""")~";
    const char* expected_escaped_str =     R"~("msg":"\"\"\"\"\"\"\"\"\"\"")~";
    const char* expected_truncated_str_1 = R"~("msg":"\"\"\"\"[truncated]")~";
    const char* expected_truncated_str_2 = R"~("msg":"\"\"\"[truncated]")~";

    len = test_format_message(buffer, strlen(expected_escaped_str), str);
    EXPECT_EQ(len, strlen(expected_truncated_str_1));

    len = test_format_message(buffer, strlen(expected_escaped_str) - 1, str);
    EXPECT_EQ(len, strlen(expected_truncated_str_2));

    len = test_format_message(buffer, strlen(expected_escaped_str) - 2, str);
    EXPECT_EQ(len, strlen(expected_truncated_str_2));
}

class FormatMdcTest: public testing::Test
{
public:
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    const char* expected_empty_mdc_str = "\"mdc\":{}";
    const char* expected_one_mdc_str = "\"mdc\":{\"key2\":\"value2\"}";
    const char* expected_str = "\"mdc\":{\"key2\":\"value2\",\"key1\":\"value1\"}";

    void SetUp()
    {
        ASSERT_EQ(0, mdclog_internal_init_mdc());
        ASSERT_EQ(0, mdclog_internal_put_mdc("key1", "value1"));
        ASSERT_EQ(0, mdclog_internal_put_mdc("key2", "value2"));
    }

    void TearDown()
    {
        mdclog_internal_destroy_mdclist();
    }
};

TEST_F(FormatMdcTest, MdcCannotBeFormattedToTooShortBuffer)
{
    len = format_mdc(buffer, strlen(expected_empty_mdc_str) - 1, NULL);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

TEST_F(FormatMdcTest, NoMdcValuesButEmptyMdcCannotBeFormattedToTooShortBuffer)
{
    len = format_mdc(buffer, strlen(expected_empty_mdc_str), NULL);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer), 0U);
}

TEST_F(FormatMdcTest, NoMdcValuesIsFormattedCorrectly)
{
    len = format_mdc(buffer, strlen(expected_empty_mdc_str) + 1, NULL);   // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_empty_mdc_str));
    EXPECT_THAT(buffer, StrEq(expected_empty_mdc_str));
}

TEST_F(FormatMdcTest, MdcIsFormattedCorrectly)
{
    len = format_mdc(buffer, strlen(expected_str) + 1, mdclog_internal_get_first_mdc());  // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatMdcTest, MdcIsTruncatedIfBufferIsTooShortForEvenOneMdc)
{
    len = format_mdc(buffer, strlen(expected_one_mdc_str), mdclog_internal_get_first_mdc());
    EXPECT_EQ(len, strlen(expected_empty_mdc_str));
    EXPECT_THAT(buffer, StrEq(expected_empty_mdc_str));
}

TEST_F(FormatMdcTest, MdcIsFormattedCorrectlyWhenOneMdc)
{
    len = format_mdc(buffer, strlen(expected_one_mdc_str) + 1, mdclog_internal_get_first_mdc());  // +1 for the NULL char
    EXPECT_EQ(len, strlen(expected_one_mdc_str));
    EXPECT_THAT(buffer, StrEq(expected_one_mdc_str));
}

TEST_F(FormatMdcTest, MdcIsTruncatedIfBufferIsTooShortForAllMdcs)
{
    len = format_mdc(buffer, strlen(expected_str), mdclog_internal_get_first_mdc());
    EXPECT_EQ(len, strlen(expected_one_mdc_str));
    EXPECT_THAT(buffer, StrEq(expected_one_mdc_str));
}

class FormatToJsonStrTest: public testing::Test
{
public:
    char buffer[MIN_BUFFER_LENGTH];
    size_t len;
    int ret;
    struct timeval tv = {1550667066, 123456};
    const char* expected_str = "{\"ts\":1550667066123,\"crit\":\"ERROR\",\"id\""
            ":\"Mickey Mouse\",\"mdc\":{\"key1\":\"value1\"},\"msg\":\"Test log 999\"}";

    void SetUp()
    {
        ASSERT_EQ(0, mdclog_internal_init_mdc());
        ASSERT_EQ(0, mdclog_internal_put_mdc("key1", "value1"));
    }

    void TearDown()
    {
        mdclog_internal_destroy_mdclist();
    }

    int test_format_to_json_str(char* buffer,
                            size_t len,
                            struct timeval* timestamp,
                            const char* identity,
                            mdclog_severity_t severity,
                            mdc_t* mdc,
                            const char* fmt,
                            ...)
    {
        int ret;
        va_list arglist;
        va_start(arglist, fmt);
        ret = mdclog_internal_format_to_json_str(buffer, len, timestamp, identity, severity, mdc, fmt, arglist);
        va_end(arglist);
        return ret;
    }
};

TEST_F(FormatToJsonStrTest, LogIsCorrectlyFormattedWhenItFitsToTheBuffer)
{
    ret = test_format_to_json_str(buffer, sizeof(buffer), &tv, "Mickey Mouse", MDCLOG_ERR, mdclog_internal_get_first_mdc(),
            "%s %d", "Test log", 999);
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatToJsonStrTest, FunctionReturnsErrorWhenTheBufferIsTooShort)
{
    ret = test_format_to_json_str(buffer, sizeof(buffer)-1, &tv, "Mickey Mouse", MDCLOG_ERR, mdclog_internal_get_first_mdc(),
            "%s %d", "Test log", 999);
    EXPECT_EQ(ret, -1);
}

#if HAVE_JSONCPP
TEST_F(FormatToJsonStrTest, LogEntryIsValidJSONAndCanBeParsedWithJSONCPP)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errors;
    std::unique_ptr<Json::CharReader> const reader(rbuilder.newCharReader());

    int len = test_format_to_json_str(buffer, sizeof(buffer), &tv, "Mickey Mouse", MDCLOG_ERR, mdclog_internal_get_first_mdc(),
            "%s %d", "Test log", 999);
    EXPECT_TRUE(reader->parse(buffer, buffer + len, &root, &errors)) << errors;
    Json::Value::Members members = root.getMemberNames();
    EXPECT_THAT(members, UnorderedElementsAre(
            StrEq("ts"), StrEq("crit"), StrEq("id"), StrEq("mdc"), StrEq("msg")));
}
#endif

class FormatLogEntryTest: public testing::Test
{
public:
    int ret;
    struct timeval tv = {1550667066, 123456};
    char buffer[MIN_BUFFER_LENGTH];
    const char* expected_with_mdc_str = "{\"ts\":1550667066123,\"crit\":\"INFO\",\"id\":\"Pluto\","
            "\"mdc\":{\"key2\":\"value2\",\"key1\":\"value1\"},\"msg\":\"This is a test log\"}";

    void SetUp()
    {
        ASSERT_EQ(0, mdclog_internal_init_mdc());
        ASSERT_EQ(0, mdclog_internal_put_mdc("key1", "value1"));
        ASSERT_EQ(0, mdclog_internal_put_mdc("key2", "value2"));
    }

    int test_format_log_entry(char* buffer,
                              size_t len,
                              struct timeval* timestamp,
                              const char* identity,
                              mdclog_severity_t severity,
                              mdc_t* mdc,
                              const char* fmt,
                              ...)
    {
        int ret;
        va_list arglist;
        va_start(arglist, fmt);
        ret = format_log_entry(buffer, len, timestamp, identity, severity, mdc, fmt, arglist);
        va_end(arglist);
        return ret;
    }
};

TEST_F(FormatLogEntryTest, LogIsTruncatedIfOnlyMinimumTrucatedMessageFits)
{
    const char* expected_str = "{\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");  // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, LogIsTruncatedIfOnlyTimestampAndMinimumTrucatedMessageFits)
{
    const char* expected_str = "{\"ts\":1550667066123,\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");  // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, LogIsTruncatedIfOnlyTimestampSeverityAndMinimumTrucatedMessageFits)
{
    const char* expected_str = "{\"ts\":1550667066123,\"crit\":\"INFO\",\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");  // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, LogIsTruncatedIfOnlyTimestampSeverityLoggerAndMinimumTrucatedMessageFits)
{
    const char* expected_str = "{\"ts\":1550667066123,\"crit\":\"INFO\",\"id\":\"Pluto\",\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");  // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, MdcIsTruncatedSoThatMinimumTruncatedMessageStillFits)
{
    const char* expected_str = "{\"ts\":1550667066123,\"crit\":\"INFO\",\"id\":\"Pluto\",\"mdc\":{},\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");  // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, MdcIsPartlyTruncatedSoThatMinimumTruncatedMessageStillFits)
{
    const char* expected_str = "{\"ts\":1550667066123,\"crit\":\"INFO\",\"id\":"
            "\"Pluto\",\"mdc\":{\"key2\":\"value2\"},\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str) + 1, &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log"); // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_str));
    EXPECT_THAT(buffer, StrEq(expected_str));
}

TEST_F(FormatLogEntryTest, LogIsCorrectlyFormattedWhenItFitsToTheBuffer)
{
    ret = test_format_log_entry(buffer, strlen(expected_with_mdc_str) + 1, &tv, "Pluto",
            MDCLOG_INFO, mdclog_internal_get_first_mdc(), "This is a test log"); // +1 for the NULL char
    EXPECT_EQ(ret, (int)strlen(expected_with_mdc_str));
    EXPECT_THAT(buffer, StrEq(expected_with_mdc_str));
}

TEST_F(FormatLogEntryTest, FunctionReturnsErrorIfMinimumTruncatedMessageDoesNotFit)
{
    const char* expected_str = "{\"msg\":\"[truncated]\"}";
    ret = test_format_log_entry(buffer, strlen(expected_str), &tv, "Pluto", MDCLOG_INFO,
            mdclog_internal_get_first_mdc(), "This is a test log");
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(strlen(buffer), 0U);
}

class EscapeTest: public testing::Test
{
public:
    char buffer1[MIN_BUFFER_LENGTH];
    char buffer2[MIN_BUFFER_LENGTH];
    int truncated;
    size_t len;
};

TEST_F(EscapeTest, EscapeAndDeescape)
{
    char str[] = {'\\', '"', 'A', '"', '\0'};                            // \"A"
    char expected_escaped_str[] = {'\\', '\\', '\\', '"', 'A', '\\', '"', '\0'};   // \\\"A\"
    len = mdclog_internal_escape(buffer1, strlen(expected_escaped_str) + 1, str, &truncated); // +1 for the NULL char
    EXPECT_EQ(truncated, 0);
    EXPECT_THAT(buffer1, StrEq(expected_escaped_str));
    EXPECT_EQ(len, strlen(expected_escaped_str));

    len = mdclog_internal_deescape(buffer2, strlen(str) + 1, buffer1);
    EXPECT_EQ(len, strlen(str));
    EXPECT_THAT(buffer2, StrEq(str));

    EXPECT_THAT(mdclog_internal_escaped_size(str), strlen(expected_escaped_str) + 1);
}

TEST_F(EscapeTest, EscapeTruncatedCannotCutBetweenTheEscapeCharAndTheEscapedChar)
{
    char str[] = {'A', '\\', '\0'};
    len = mdclog_internal_escape(buffer1, 3, str, &truncated);
    EXPECT_EQ(truncated, 1);
    EXPECT_THAT(buffer1, StrEq("A"));
    EXPECT_EQ(len, strlen("A"));
}

TEST_F(EscapeTest, EscapedStringIsTruncatedCorrectly)
{
    char str[] = {'\\', 'A', '\0'};
    char expected_escaped_str[] = {'\\', '\\', '\0'};
    len = mdclog_internal_escape(buffer1, 3, str, &truncated);
    EXPECT_EQ(truncated, 1);
    EXPECT_THAT(buffer1, StrEq(expected_escaped_str));
    EXPECT_EQ(len, strlen(expected_escaped_str));

    len = mdclog_internal_deescape(buffer2, 2, buffer1);
    EXPECT_EQ(len, strlen("\\"));
    EXPECT_THAT(buffer2, StrEq("\\"));
}

TEST_F(EscapeTest, NothingToEscape)
{
    const char* str = "Donald Duck";
    len = mdclog_internal_escape(buffer1, strlen(str) + 1, str, &truncated); // +1 for the NULL char
    EXPECT_EQ(truncated, 0);
    EXPECT_THAT(buffer1, StrEq(str));
    EXPECT_EQ(len, strlen(str));

    len = mdclog_internal_deescape(buffer2, strlen(str) + 1, buffer1); // +1 for the NULL char
    EXPECT_EQ(len, strlen(str));
    EXPECT_THAT(buffer2, StrEq(str));

    EXPECT_THAT(mdclog_internal_escaped_size(str), strlen(str)+1);
}

TEST_F(EscapeTest, NothingToEscapeTruncated)
{
    const char* str = "Donald Duck";
    len = mdclog_internal_escape(buffer1, strlen(str), str, &truncated);
    EXPECT_EQ(truncated, 1);
    EXPECT_THAT(buffer1, StrEq("Donald Duc"));
    EXPECT_EQ(len, strlen("Donald Duc"));
}

TEST_F(EscapeTest, InvalidStringsToDeescape)
{
    char str1[] = {'\\', '\0'};
    len = mdclog_internal_deescape(buffer2, sizeof(buffer2), str1);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer2), 0U);

    char str2[] = {'\\', 'A', '\0'};
    len = mdclog_internal_deescape(buffer2, sizeof(buffer2), str2);
    EXPECT_EQ(len, 0U);
    EXPECT_EQ(strlen(buffer2), 0U);
}

TEST_F(EscapeTest, NonPrintableCharsAreReplacedWithSpace)
{
    const char* str = "\fThese\tare\tprintable\b\r\nchars\r";
    len = mdclog_internal_escape(buffer1, sizeof(buffer1), str, &truncated);
    EXPECT_EQ(truncated, 0);
    EXPECT_THAT(buffer1, StrEq(" These are printable   chars "));
}

TEST_F(EscapeTest, ContainsSpecialCharacters)
{
    EXPECT_EQ(0, mdclog_internal_contains_special_characters("This is a string without special characters"));
    EXPECT_EQ(1, mdclog_internal_contains_special_characters("This string contains a non-printable character\n"));
    EXPECT_EQ(1, mdclog_internal_contains_special_characters("This string contains an escaped character\""));
    EXPECT_EQ(1, mdclog_internal_contains_special_characters("This string contains an escaped character\\"));
}
