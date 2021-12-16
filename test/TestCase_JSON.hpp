/*
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __TESTCASE_JSON_HPP__
#define __TESTCASE_JSON_HPP__

#include <gtest/gtest.h>

#include "JSON.hpp"

class TestCase_JSON : public ::testing::Test
{
protected:
  TestCase_JSON();
  virtual ~TestCase_JSON();
  virtual void SetUp();
  virtual void TearDown();

#if JSON_SHARED_PTR
  void testGetterCommon(std::shared_ptr<JSON> json);
#else
  void testGetterCommon(JSON json);
#endif /* JSON_SHARED_PTR */
  void testFromString(void);
  void testSetter(void);
};

#endif /* __TESTCASE_JSON_HPP__ */
