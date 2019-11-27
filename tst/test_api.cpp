/*
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

#include "mdclog/mdclog.h"
#include "system_mock.hpp"
#include "private/mdc.h"

using namespace testing;
using namespace mdclogtest;

extern "C" {
const char *__progname;
void mdclog_lib_clean(void);
}

class APITest: public testing::Test
{
public:
    NiceMock<SystemMock> systemMock;
    char *               mdc;
    mdclog_attr_t*       attr;
    const char *         progname;
    void SetUp()
    {
        setSystemMock(&systemMock);
        mdc = NULL;
        progname = __progname;
    }

    void TearDown()
    {
        mdclog_internal_destroy_mdclist();
        if (mdc) free(mdc);
        __progname = progname;
        mdclog_lib_clean();
    }

    void setupWriteExpects(std::vector<const char*> substrs)
    {
        EXPECT_CALL(systemMock, write(STDOUT_FILENO, NotNull(), _))
              .Times(1)
              .WillOnce(Invoke([substrs] (int, const void* buffer, int len)
              {
                  std::string received(static_cast<const char*>(buffer), len);
                  for (auto substr: substrs)
                  {
                      EXPECT_THAT(received, HasSubstr((substr)));
                      EXPECT_THAT(received, MatchesRegex(".*\n$"));
                  }
                  return len;
              }));
    }
};

TEST_F(APITest, BasicLogWriteWithDefaultInitAndNoMDCIsWrittingToStdout)
{
    std::vector<const char*> expected {"hep1", "ERR", __progname};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_ERR, "hep%d", 1);
}

TEST_F(APITest, DefaultIdentityIsEscaped)
{
    __progname = "name\nwith\tnonprint";
    EXPECT_EQ(0, mdclog_init(NULL));
    std::vector<const char*> expected {"hep1", "ERR", "name with nonprint"};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_ERR, "hep%d", 1);
}

TEST_F(APITest, UserDefinedLogIdentityIsAddedToLog)
{
    mdclog_attr_t *attr;
    EXPECT_EQ(0, mdclog_attr_init(&attr));
    EXPECT_EQ(0, mdclog_attr_set_ident(attr, "myname"));
    EXPECT_EQ(0, mdclog_init(attr));
    mdclog_attr_destroy(attr);
    std::vector<const char*> expected {"logentry", "myname"};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_ERR, "logentry");
}

TEST_F(APITest, LogReinitClearsSetAttributes)
{
    EXPECT_EQ(0, mdclog_attr_init(&attr));
    EXPECT_EQ(0, mdclog_attr_set_ident(attr, "myname"));
    EXPECT_EQ(0, mdclog_init(attr));
    mdclog_attr_destroy(attr);
    EXPECT_EQ(0, mdclog_init(NULL));
    std::vector<const char*> expected {"logentry", __progname};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_ERR, "logentry");
}

TEST_F(APITest, NullIsNotValidIdentity)
{
    EXPECT_EQ(0, mdclog_attr_init(&attr));
    EXPECT_EQ(-1, mdclog_attr_set_ident(attr, NULL));
    EXPECT_EQ(errno, EINVAL);
    mdclog_attr_destroy(attr);
}

TEST_F(APITest, IdentityContainingSpecialCharactersIsNotValid)
{
    EXPECT_EQ(0, mdclog_attr_init(&attr));

    EXPECT_EQ(-1, mdclog_attr_set_ident(attr, "\\"));
    EXPECT_EQ(errno, EINVAL);
    EXPECT_EQ(-1, mdclog_attr_set_ident(attr, "\""));
    EXPECT_EQ(errno, EINVAL);
    EXPECT_EQ(-1, mdclog_attr_set_ident(attr, "\b"));
    EXPECT_EQ(errno, EINVAL);
    mdclog_attr_destroy(attr);
}

TEST_F(APITest, NullIsNotValidAttributeInSetIdent)
{
    EXPECT_EQ(-1, mdclog_attr_set_ident(NULL, "foo"));
    EXPECT_EQ(errno, EINVAL);
}

TEST_F(APITest, MaxLengthIdentityIsValid)
{
    std::string identity(IDENTITY_MAX_LENGTH, 'A');
    EXPECT_EQ(0, mdclog_attr_init(&attr));
    EXPECT_EQ(0, mdclog_attr_set_ident(attr, identity.c_str()));
    mdclog_attr_destroy(attr);
}

TEST_F(APITest, TooLongIdentityIsNotValid)
{
    std::string identity(IDENTITY_MAX_LENGTH + 1, 'B');
    EXPECT_EQ(0, mdclog_attr_init(&attr));
    EXPECT_EQ(-1, mdclog_attr_set_ident(attr, identity.c_str()));
    EXPECT_EQ(errno, EINVAL);
    mdclog_attr_destroy(attr);
}

TEST_F(APITest, SetMDCValuesAreIncludedInLog)
{
    mdclog_mdc_add("foo1", "bar");
    mdclog_mdc_add("foo2", "baz");
    std::vector<const char*> expected {"hep1", "ERR", __progname, "foo1", "bar", "foo2", "baz"};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_ERR, "hep%d", 1);
}

TEST_F(APITest, CurrentLoggingLevelPreventsLogWriting)
{
    mdclog_level_set(MDCLOG_WARN);
    std::vector<const char*> expected {"hep1", "ERR", __progname};
    setupWriteExpects(expected);
    mdclog_write(MDCLOG_INFO, "hep%d", 2); // this is not written
    mdclog_write(MDCLOG_ERR, "hep%d", 1);  // this is
}

TEST_F(APITest, UserCanReadCurrentLoggingLevel)
{
    mdclog_level_set(MDCLOG_ERR);
    EXPECT_EQ(mdclog_level_get(), MDCLOG_ERR);
}

TEST_F(APITest, UserCanReadSetMDCValue)
{
    EXPECT_EQ(0, mdclog_mdc_add("foo", "bar"));
    mdc = mdclog_mdc_get("foo");
    EXPECT_THAT(mdc, StrEq("bar"));
}

TEST_F(APITest, GettingNonExistingMDCReturnsNULL)
{
    EXPECT_THAT(mdclog_mdc_get("foo"), IsNull());
}

TEST_F(APITest, GettingMDCWithNULLArgumentReturnsNULL)
{
    EXPECT_THAT(mdclog_mdc_get(NULL), IsNull());
}

TEST_F(APITest, UserCanRemoveMDCKey)
{
    EXPECT_EQ(0, mdclog_mdc_add("foo", "bar"));
    mdc = mdclog_mdc_get("foo");
    EXPECT_THAT(mdc, NotNull());
    mdclog_mdc_remove("foo");
    EXPECT_THAT(mdclog_mdc_get("foo"), IsNull());
}

TEST_F(APITest, RemovingNonExistentMDCIsTolerated)
{
    mdclog_mdc_remove("foo");
}

TEST_F(APITest, RemoveMDCWithNullArgumentDoesNotCauseCrash)
{
    mdclog_mdc_remove(NULL);
}

TEST_F(APITest, NullValuesInMDCKeyOrValueAreNotAccepted)
{
    EXPECT_EQ(-1, mdclog_mdc_add(NULL, "foo"));
    EXPECT_EQ(errno, EINVAL);
    EXPECT_EQ(-1, mdclog_mdc_add("foo", NULL));
    EXPECT_EQ(errno, EINVAL);
}

TEST_F(APITest, SpecialCharactersInMDCKeyAreNotAccepted)
{
    EXPECT_EQ(-1, mdclog_mdc_add("key with an escaped char\\", "foo"));
    EXPECT_EQ(errno, EINVAL);
    EXPECT_EQ(-1, mdclog_mdc_add("key with an escaped char\"", "foo"));
    EXPECT_EQ(errno, EINVAL);
    EXPECT_EQ(-1, mdclog_mdc_add("key with a non-printable char\t", "foo"));
    EXPECT_EQ(errno, EINVAL);
}
