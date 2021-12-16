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

#include "TestCase_Common.hpp"
#include "TestCase_JSON.hpp"

TestCase_JSON::TestCase_JSON()
{
}

TestCase_JSON::~TestCase_JSON()
{
}

void TestCase_JSON::SetUp()
{
}

void TestCase_JSON::TearDown()
{
}

#if JSON_SHARED_PTR
void TestCase_JSON::testGetterCommon(std::shared_ptr<JSON> json)
{
  EXPECT_TRUE( (*json)["key1"]->getString() == "value1" );
  EXPECT_TRUE( *((*json)["key1"]) == "value1" );

  std::shared_ptr<JSON> tmp = (*json)["key2"];
  EXPECT_TRUE( tmp->getCount() == 2 );
  EXPECT_TRUE( *((*tmp)[0]) == "value2-1" );
  EXPECT_TRUE( *((*tmp)[1]) == "value2-2" );

  tmp = (*json)["key3"];
  EXPECT_TRUE( tmp->getCount() == 2 );
  tmp = (*tmp)[0];
  EXPECT_TRUE( *((*tmp)["key4"]) == "true" );
  EXPECT_TRUE( (*tmp)["key4"]->getBoolean() == true );
  EXPECT_TRUE( *((*tmp)["key5"]) == "false" );
  EXPECT_TRUE( (*tmp)["key5"]->getBoolean() == false );

  tmp = (*json)["key3"];
  tmp = (*tmp)[1];
  EXPECT_TRUE( *((*tmp)["key4"]) == "0" );
  EXPECT_TRUE( *((*tmp)["key5"]) == "null" );
  EXPECT_TRUE( (*tmp)["key5"]->isNull() );

  EXPECT_TRUE( *((*json)["key6"]) == "3.14159" );
  EXPECT_TRUE( (*json)["key6"]->getFloat() == 3.14159f );
  EXPECT_TRUE( (*json)["key6"]->getInt() == 3 );
}
#else /* JSON_SHARED_PTR */
void TestCase_JSON::testGetterCommon(JSON json)
{
  EXPECT_TRUE( json.getCount() == 4 );

  EXPECT_TRUE( json["key1"].getString() == "value1" );
  EXPECT_TRUE( json["key1"] == "value1" );

  EXPECT_TRUE( json["key2"].getCount() == 2 );
  EXPECT_TRUE( json["key2"][0] == "value2-1" );
  EXPECT_TRUE( json["key2"][1] == "value2-2" );

  EXPECT_TRUE( json["key3"].getCount() == 2 );
  EXPECT_TRUE( json["key3"][0]["key4"] == "true" );
  EXPECT_TRUE( json["key3"][0]["key4"].getBoolean() == true );
  EXPECT_TRUE( json["key3"][0]["key5"] == "false" );
  EXPECT_TRUE( json["key3"][0]["key5"].getBoolean() == false );
  EXPECT_TRUE( json["key3"][1]["key4"] == "0" );
  EXPECT_TRUE( json["key3"][1]["key4"].getInt() == 0 );
  EXPECT_TRUE( json["key3"][1]["key5"] == "null" );
  EXPECT_TRUE( json["key3"][1]["key5"].isNull() == true );

  EXPECT_TRUE( json["key6"] == "3.14159" );
  EXPECT_TRUE( json["key6"].getFloat() == 3.14159f );
  EXPECT_TRUE( json["key6"].getInt() == 3 );
}
#endif /* JSON_SHARED_PTR */

TEST_F(TestCase_JSON, testFromString)
{
  std::string jsonString = FileUtil::readFileToString("test.json");
#if JSON_SHARED_PTR
  std::shared_ptr<JSON> json = std::make_shared<JSON>( jsonString );
//   testGetterCommon( json );
#else
   JSON json( jsonString );
   testGetterCommon( json );
#endif
}

TEST_F(TestCase_JSON, testSetter)
{
  std::shared_ptr<JSON> json = std::make_shared<JSON>();

  *((*json)["key1"]) = "value1";

  EXPECT_TRUE( (*json)["key1"]->getString() == "value1" );
  EXPECT_TRUE( *((*json)["key1"]) == "value1" );

  *((*json)["key2"]) = JSONArray();
  (*json)["key2"]->push_back( "value2-1" );
  (*json)["key2"]->push_back( "value2-2" );

  EXPECT_TRUE( (*json)["key2"]->getCount() == 2 );
  std::shared_ptr<JSON> tmp = (*json)["key2"];
  EXPECT_TRUE( *((*tmp)[0]) == "value2-1" );
  EXPECT_TRUE( *((*tmp)[1]) == "value2-2" );

  std::shared_ptr<JSON> json2 = std::make_shared<JSON>();
  *((*json2)["key4"]) = "true";
  (*json2)["key5"]->setValue( false );

  (*json)["key3"]->push_back( json2 );

  EXPECT_TRUE( (*json)["key3"]->getCount() == 1 );
  tmp = (*json)["key3"];
  tmp = (*tmp)[0];
  EXPECT_TRUE( *((*tmp)["key4"]) == "true" );
  EXPECT_TRUE( (*tmp)["key4"]->getBoolean() == true );
  EXPECT_TRUE( *((*tmp)["key5"]) == "false" );
  EXPECT_TRUE( (*tmp)["key5"]->getBoolean() == false );

  std::shared_ptr<JSON> json3 = std::make_shared<JSON>();
  *((*json3)["key4"]) = 0;
  *((*json3)["key5"]) = null;

  (*json)["key3"]->push_back( json3 );

  EXPECT_TRUE( (*json)["key3"]->getCount() == 2 );
  tmp = (*json)["key3"];
  tmp = (*tmp)[1];
  EXPECT_TRUE( *((*tmp)["key4"]) == "0" );
  EXPECT_TRUE( *((*tmp)["key5"]) == "null" );
  EXPECT_TRUE( (*tmp)["key5"]->isNull() );

  *((*json)["key6"]) = 3.14159f;

  EXPECT_TRUE( *((*json)["key6"]) == "3.14159" );
  EXPECT_TRUE( (*json)["key6"]->getFloat() == 3.14159f );
  EXPECT_TRUE( (*json)["key6"]->getInt() == 3 );

  testGetterCommon( json );
}