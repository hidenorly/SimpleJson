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

#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <memory>
#include <map>
#include <vector>
#include <string>

#ifndef JSON_NULL
  #define JSON_NULL "null"
#endif /* JSON_NULL */


#define JSON_SHARED_PTR 1

#if JSON_SHARED_PTR
#define JSON_REF_TYPE std::shared_ptr<JSON>
#else
#define JSON_REF_TYPE JSON&
#endif


#if JSON_SHARED_PTR
class JSON : public std::enable_shared_from_this<JSON>
{
protected:
  enum JSON_TYPE
  {
    TYPE_VALUE,
    TYPE_HASH,
    TYPE_ARRAY
  };
  JSON_TYPE mType;

  std::map<std::string, std::shared_ptr<JSON>> mHashData;
  std::vector<std::shared_ptr<JSON>> mArrayData;
  std::string mValue;
  std::weak_ptr<JSON> mUplinkRef;

protected:
  static std::string getHierachyKey(std::vector<std::string> keys);
  static int getIndex(std::string key);

public:
  JSON(std::shared_ptr<JSON> pJsonUplink = nullptr);
  virtual ~JSON();

  static std::shared_ptr<JSON> parse(std::string jsonString, std::shared_ptr<JSON> pJson = nullptr);
  virtual std::shared_ptr<JSON> getSharedPtr(void);

  int getCount(void);
  bool operator==(std::string value);

  bool isNull(void);
  bool isHash(void);
  bool isArray(void);
  bool isValue(void);

  bool hasOwnProperty(std::string key);

  std::string getString(void);

  int getInt(void);
  float getFloat(void);
  bool getBoolean(void);

  std::shared_ptr<JSON> operator[](std::string key);
  std::shared_ptr<JSON> operator[](int nIndex);

  std::shared_ptr<JSON> getObjectRelativePath(std::string key, bool bForceEnsure = false);

  void setObjectRelativePath(std::string key, std::shared_ptr<JSON> value);

  void setValue(std::string value);
  void setValue(const char* value);
  void setValue(int value);
  void setValue(float value);
  void setValue(bool value);

  std::shared_ptr<JSON> operator=(const char* value);
  std::shared_ptr<JSON> operator=(std::string value);
  std::shared_ptr<JSON> operator=(int value);
  std::shared_ptr<JSON> operator=(float value);
  std::shared_ptr<JSON> operator=(std::shared_ptr<JSON> pJson);

  std::shared_ptr<JSON> insert_or_assign(std::string key, std::shared_ptr<JSON> pJson);
  std::shared_ptr<JSON> update(std::shared_ptr<JSON> pJson);
  std::string getKeyFromJsonObject(std::shared_ptr<JSON> pJson);
  void setUplink(std::shared_ptr<JSON> pUplinkJson);

  void push_back(std::shared_ptr<JSON> jsonValue);
  void push_back(std::string value);

  std::map<std::string, std::shared_ptr<JSON>>::iterator begin();
  std::map<std::string, std::shared_ptr<JSON>>::iterator end();

  std::vector<std::shared_ptr<JSON>>::iterator getArrayIteratorBegin();
  std::vector<std::shared_ptr<JSON>>::iterator getArrayIteratorEnd();

  static void dump(JSON_REF_TYPE json, std::string rootKey = "");
};

#else /* JSON_SHARED_PTR */
#endif /* JSON_SHARED_PTR */

class JSONArray : public JSON
{
public:
  JSONArray(std::shared_ptr<JSON> pJsonUplink = nullptr):JSON(pJsonUplink){ mType = JSON_TYPE::TYPE_ARRAY; };
  virtual ~JSONArray(){};
};


#endif /* __JSON_HPP__ */