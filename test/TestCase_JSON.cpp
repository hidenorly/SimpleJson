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

void TestCase_JSON::testGetterCommon(JSON_REF_TYPE json)
#if JSON_SHARED_PTR
{
  EXPECT_TRUE( (*json)["key1"]->getString() == "value1" );
  EXPECT_TRUE( *((*json)["key1"]) == "value1" );

  std::shared_ptr<JSON> tmp = (*json)["key2"];
  EXPECT_TRUE( tmp->getCount() == 2 );
  EXPECT_TRUE( *((*tmp)[0]) == "value2-1" );
  EXPECT_TRUE( *((*tmp)[1]) == "value2-2" );
  EXPECT_TRUE( tmp->isArray() );

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
  EXPECT_TRUE( *((*tmp)["key5"]) == JSON_NULL );
  EXPECT_TRUE( (*tmp)["key5"]->isNull() );

  EXPECT_TRUE( *((*json)["key6"]) == "3.14159" );
  EXPECT_TRUE( (*json)["key6"]->getFloat() == 3.14159f );
  EXPECT_TRUE( (*json)["key6"]->getInt() == 3 );

  EXPECT_TRUE( (*json)["key6"]->isValue() );
  EXPECT_TRUE( json->getObjectRelativePath("key6")->getString() == "3.14159" );
  EXPECT_TRUE( *(json->getObjectRelativePath("key2.0")) == "value2-1" );
}
#else /* JSON_SHARED_PTR */
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
  EXPECT_TRUE( json["key3"][1]["key5"] == JSON_NULL );
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
  std::shared_ptr<JSON> json = JSON::parse( jsonString );
//   testGetterCommon( json );
#else /* JSON_SHARED_PTR */
   JSON json( jsonString );
   testGetterCommon( json );
#endif /* JSON_SHARED_PTR */
}

TEST_F(TestCase_JSON, testSetter)
{
#if JSON_SHARED_PTR
  std::shared_ptr<JSON> json = std::make_shared<JSON>();

  *((*json)["key1"]) = "value1";

  EXPECT_TRUE( (*json)["key1"]->getString() == "value1" );
  EXPECT_TRUE( *((*json)["key1"]) == "value1" );

  std::shared_ptr<JSON> tmp = std::make_shared<JSONArray>();
  tmp->push_back( "value2-1" );
  tmp->push_back( "value2-2" );
//  json->insert_or_assign( "key2", tmp );
  *((*json)["key2"]) = tmp;

  EXPECT_TRUE( (*json)["key2"]->getCount() == 2 );
  EXPECT_TRUE( (*json)["key2"]->isArray() );
  tmp = (*json)["key2"];
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
  *((*json3)["key5"]) = JSON_NULL;

  (*json)["key3"]->push_back( json3 );

  EXPECT_TRUE( (*json)["key3"]->getCount() == 2 );
  tmp = (*json)["key3"];
  tmp = (*tmp)[1];
  EXPECT_TRUE( *((*tmp)["key4"]) == "0" );
  EXPECT_TRUE( *((*tmp)["key5"]) == JSON_NULL );
  EXPECT_TRUE( (*tmp)["key5"]->isNull() );

  *((*json)["key6"]) = 3.14159f;

  EXPECT_TRUE( *((*json)["key6"]) == "3.14159" );
  EXPECT_TRUE( (*json)["key6"]->getFloat() == 3.14159f );
  EXPECT_TRUE( (*json)["key6"]->getInt() == 3 );

  std::shared_ptr<JSON> p = (*json)["key7"];
  p->setValue("hoge7");
  std::shared_ptr<JSON> v = std::make_shared<JSON>();
  v->setValue( "hoge8" );
  p->insert_or_assign("key8", v );
  p = v;
  v = std::make_shared<JSON>();
  v->setValue( "hoge9" );
  p->insert_or_assign("key9", v );

  EXPECT_TRUE( *((*((*((*json)["key7"]))["key8"]))["key9"]) == "hoge9" );

  p = json->getObjectRelativePath("key7.key8.key9");
  EXPECT_TRUE( p->getString() == "hoge9" );
#else /* JSON_SHARED_PTR */
#endif /* JSON_SHARED_PTR */

  testGetterCommon( json );
  JSON::dump( json );
}

TEST_F(TestCase_JSON, testSetObjectRelativePath)
{
  std::shared_ptr<JSON> json = std::make_shared<JSON>();

  json->setObjectRelativePath( "key3", std::make_shared<JSONArray>());

  std::shared_ptr<JSON> pValue = std::make_shared<JSON>();
  pValue->setValue(true);
  json->setObjectRelativePath( "key3.[0].key4", pValue);

  pValue = std::make_shared<JSON>();
  pValue->setValue(false);
  json->setObjectRelativePath( "key3.[0].key5", pValue);

  pValue = std::make_shared<JSON>();
  pValue->setValue( 0 );
  json->setObjectRelativePath( "key3.[1].key4", pValue);

  pValue = std::make_shared<JSON>();
  *pValue = JSON_NULL;
  json->setObjectRelativePath( "key3.[1].key5", pValue);

  JSON::dump( json );

  EXPECT_TRUE( (*json)["key3"]->getCount() == 2 );
  std::shared_ptr<JSON> tmp = (*json)["key3"];

  tmp = (*tmp)[0];
  EXPECT_TRUE( *((*tmp)["key4"]) == "true" );
  EXPECT_TRUE( (*tmp)["key4"]->getBoolean() == true );
  EXPECT_TRUE( *((*tmp)["key5"]) == "false" );
  EXPECT_TRUE( (*tmp)["key5"]->getBoolean() == false );

  tmp = (*tmp)[1];
  EXPECT_TRUE( (*tmp)["key4"]->getInt() == 0 );
  EXPECT_TRUE( *((*tmp)["key5"]) == JSON_NULL );
  EXPECT_TRUE( (*tmp)["key5"]->isNull() );

  tmp = json->getObjectRelativePath("key3.[0].key4");
  EXPECT_TRUE( tmp->getBoolean() == true );

  tmp = json->getObjectRelativePath("key3.[0].key5");
  EXPECT_TRUE( tmp->getBoolean() == false );

  tmp = json->getObjectRelativePath("key3.[1].key4");
  EXPECT_TRUE( tmp->getInt() == 0 );

  tmp = json->getObjectRelativePath("key3.[1].key5");
  EXPECT_TRUE( tmp->isNull() );
}


TEST_F(TestCase_JSON, testIterator)
{
#if JSON_SHARED_PTR
  std::shared_ptr<JSON> json = std::make_shared<JSON>();

  *((*json)["key1"]) = "value1";

  std::shared_ptr<JSON> tmp = std::make_shared<JSONArray>();
  tmp->push_back( "value2-1" );
  tmp->push_back( "value2-2" );
  *((*json)["key2"]) = tmp;

  std::shared_ptr<JSON> json2 = std::make_shared<JSON>();
  *((*json2)["key4"]) = "true";
  (*json2)["key5"]->setValue( false );
  (*json)["key3"]->push_back( json2 );

  std::shared_ptr<JSON> json3 = std::make_shared<JSON>();
  *((*json3)["key4"]) = 0;
  *((*json3)["key5"]) = JSON_NULL;

  (*json)["key3"]->push_back( json3 );

  tmp = (*json)["key3"];
  tmp = (*tmp)[1];

  *((*json)["key6"]) = 3.14159f;
#else /* JSON_SHARED_PTR */
#endif /* JSON_SHARED_PTR */

  JSON::dump( json );
}


TEST_F(TestCase_JSON, testSetValueRelativePath)
{
#if JSON_SHARED_PTR
  std::shared_ptr<JSON> json = std::make_shared<JSON>();
  json->setValueRelativePath("key1", "value1");
  JSON::dump(json);
  EXPECT_TRUE( *((*json)["key1"]) == "value1" );

  json->setValueRelativePath("key7.key8.key9", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( *((*((*((*json)["key7"]))["key8"]))["key9"]) == "hoge9" );
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("key1", 1);
  JSON::dump(json);
  EXPECT_TRUE( (*json)["key1"]->getInt() == 1 );

  json->setValueRelativePath("key1", 3.14159f);
  JSON::dump(json);
  EXPECT_TRUE( (*json)["key1"]->getFloat() == 3.14159f );

  json->setValueRelativePath("key1", true);
  JSON::dump(json);
  EXPECT_TRUE( (*json)["key1"]->getBoolean() );

  json->setValueRelativePath("\"key7\".\"key8\".\"key9\"", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("key7.\"key8\".key9", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("\"key7\".key8.key9", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("key7.key8.\"key9\"", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("\"key7\".key8.\"key9\"", "hoge9");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.key8.key9") == "hoge9" );

  json->setValueRelativePath("\"key7.1\".key8.\"key9.1\"", "hoge9.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "\"key7.1\".key8.\"key9.1\"") == "hoge9.1" );

  json->setValueRelativePath("\"key7.1\".\"key8.1\".\"key9.1\"", "hoge8.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "\"key7.1\".\"key8.1\".\"key9.1\"") == "hoge8.1" );

  json->setValueRelativePath("\"key7.1\".\"key8.1\".key9", "hoge8.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "\"key7.1\".\"key8.1\".key9") == "hoge8.1" );

  json->setValueRelativePath("key7.\"key8.1\".key9", "hoge8.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.\"key8.1\".key9") == "hoge8.1" );

  json->setValueRelativePath("key7.\"key8.1\".\"key9.1\"", "hoge8.1&9.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key7.\"key8.1\".\"key9.1\"") == "hoge8.1&9.1" );

  json->setValueRelativePath("\"key8.1\"", "hoge8.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "\"key8.1\"" ) == "hoge8.1" );

  json->setValueRelativePath("key8.\"key8.1\"", "hoge8.1");
  JSON::dump(json);
  EXPECT_TRUE( json->getStringRelativePath( "key8.\"key8.1\"" ) == "hoge8.1" );
#else /* JSON_SHARED_PTR */
#endif /* JSON_SHARED_PTR */
}