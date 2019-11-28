/*
 * Tests for MDC
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
#include <thread>
#include <semaphore.h>

#include "private/mdc.h"

using namespace testing;

class MDCTest: public testing::Test
{
public:
    void SetUp()
    {
        ASSERT_EQ(0, mdclog_internal_init_mdc());
    }

    void TearDown()
    {
        mdclog_internal_destroy_mdclist();
    }

    void addAndCheck(const char *key, const char *value)
    {
        EXPECT_EQ(0, mdclog_internal_put_mdc(key, value));
        findAndCheck(key, value);
    }

    void addAndCheck(const char *key, const char *value, const char *escaped_value)
    {
        EXPECT_EQ(0, mdclog_internal_put_mdc(key, value));
        findAndCheck(key, escaped_value);
    }

    void findAndCheck(const char *key, const char *value)
    {
        auto mdc(mdclog_internal_search_mdc(key));
        ASSERT_THAT(mdc, NotNull());
        EXPECT_THAT(mdclog_internal_get_mdc_key(mdc), StrEq(key));
        EXPECT_THAT(mdclog_internal_get_mdc_val(mdc), StrEq(value));
    }
};

TEST_F(MDCTest, ItIsPossibleToAddDMC)
{
    addAndCheck("foo", "bar");
}

TEST_F(MDCTest, ItIsPossibleToRemoveMDC)
{
    addAndCheck("foo", "bar");
    mdclog_internal_rm_mdc("foo");
    EXPECT_THAT(mdclog_internal_search_mdc("foo"), IsNull());
}

TEST_F(MDCTest, RemoveNonExistingMDCDoesNotHaveAnyEffect)
{
    addAndCheck("foo", "bar");
    mdclog_internal_rm_mdc("foo2");
    EXPECT_THAT(mdclog_internal_search_mdc("foo"), NotNull());
}

TEST_F(MDCTest, AddingMDCWithExistingKeyOverwritesOldEntry)
{
    addAndCheck("foo", "bar");
    addAndCheck("foo", "baz");
    mdclog_internal_rm_mdc("foo");
    EXPECT_THAT(mdclog_internal_search_mdc("foo"), IsNull());
}

TEST_F(MDCTest, ItIsPossibleToAddMultipleMDCKeys)
{
    addAndCheck("first", "1");
    addAndCheck("second", "2");
    addAndCheck("third", "3");
}

TEST_F(MDCTest, ItIsPossibleToRemoveFirstlyAddedElement)
{
    addAndCheck("first", "1");
    addAndCheck("second", "2");
    addAndCheck("third", "3");
    mdclog_internal_rm_mdc("first");
    EXPECT_THAT(mdclog_internal_search_mdc("first"), IsNull());
    findAndCheck("second", "2");
    findAndCheck("third", "3");
}

TEST_F(MDCTest, ItIsPossibleToRemoveSecondAddedElement)
{
    addAndCheck("first", "1");
    addAndCheck("second", "2");
    addAndCheck("third", "3");
    mdclog_internal_rm_mdc("second");
    EXPECT_THAT(mdclog_internal_search_mdc("second"), IsNull());
    findAndCheck("first", "1");
    findAndCheck("third", "3");
}

TEST_F(MDCTest, ItIsPossibleToRemoveLatestlyAddedElement)
{
    addAndCheck("first", "1");
    addAndCheck("second", "2");
    addAndCheck("third", "3");
    mdclog_internal_rm_mdc("third");
    EXPECT_THAT(mdclog_internal_search_mdc("third"), IsNull());
    findAndCheck("first", "1");
    findAndCheck("second", "2");
}

TEST_F(MDCTest, ItIsPossibleToClearList)
{
    addAndCheck("foo", "1");
    mdclog_internal_clean_mdclist();
    EXPECT_THAT(mdclog_internal_get_first_mdc(), IsNull());
}

TEST_F(MDCTest, MDCListIterationWorks)
{
    addAndCheck("first", "1");
    addAndCheck("second", "2");
    addAndCheck("third", "3");
    auto mdc(mdclog_internal_get_first_mdc());
    ASSERT_THAT(mdc, NotNull());
    std::map<const char*, const char*> list_items;
    while (mdc)
    {
        list_items[mdclog_internal_get_mdc_key(mdc)] = mdclog_internal_get_mdc_val(mdc);
        mdc = mdclog_internal_get_next_mdc(mdc);

    }
    EXPECT_THAT(list_items, UnorderedElementsAre(
            Pair(StrEq("first"), StrEq("1")),
            Pair(StrEq("second"), StrEq("2")),
            Pair(StrEq("third"), StrEq("3"))));
}

TEST_F(MDCTest, GetMDCKeyToleratesNullArgument)
{
    EXPECT_THAT(mdclog_internal_get_mdc_key(NULL), IsNull());
}

TEST_F(MDCTest, GetMDCValToleratesNullArgument)
{
    EXPECT_THAT(mdclog_internal_get_mdc_val(NULL), IsNull());
}

TEST_F(MDCTest, GetNextMDCToleratesNullArgument)
{
    EXPECT_THAT(mdclog_internal_get_next_mdc(NULL), IsNull());
}

TEST_F(MDCTest, SpecialCharactersInValueAreEscaped)
{
    addAndCheck("foo", "\\", "\\\\");
    addAndCheck("foo", "\"", "\\\"");
    addAndCheck("foo", "\r\n", "  ");
}


class MDCTestWithThreads: public MDCTest
{
public:
    std::thread thread1, thread2;
    sem_t sem1, sem2;

    void SetUp()
    {
        sem_init(&sem1, 0, 0);
        sem_init(&sem2, 0, 0);
    }
    void TearDown()
    {
        thread1.join();
        thread2.join();
    }
    void createThreadAndSetMDC(const char* k, const char* v)
    {
        thread1 = std::thread([this, k, v]()
                    {
                        addAndCheck(k, v);
                        sem_post(&sem1);
                        sem_wait(&sem2);
                    });
    }

};

TEST_F(MDCTestWithThreads, MDCsAreVisibleOnlyToThreadThatSetsThem)
{
    createThreadAndSetMDC("foo", "bar");
    thread2 = std::thread([this]()
            {
                sem_wait(&sem1);
                EXPECT_THAT(mdclog_internal_search_mdc("foo"), IsNull());
                sem_post(&sem2);
            });
}
