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

#ifndef null
  #define null "null"
#endif

#include <iostream>

class StringUtil
{
public:
  static std::string trim2(std::string value, std::string trimStrings )
  {
    bool bFound = false;
    for(int i=0, trimSize = trimStrings.size(); i<trimSize; i++){
      std::string trimString = trimStrings.substr( i, 1 );
      do
      {
        bFound = false;
        if( value.starts_with( trimString ) ){
          value = value.substr( 1, value.size() -1 );
          bFound = true;
        }
        if( value.ends_with( trimString ) ){
          value = value.substr( 0, value.size() -1 );
          bFound = true;
        }
      } while (bFound);
    }
    // workaround for unable to find "\"" in the above loop....
    do
    {
      bFound = false;
      if( value.starts_with( "\"" ) ){
        value = value.substr( 1, value.size() -1 );
        bFound = true;
      }
      if( value.ends_with( "\"" ) ){
        value = value.substr( 0, value.size() -1 );
        bFound = true;
      }
    } while (bFound);

    return value;
  }

  static std::string trim(std::string value )
  {
    static const std::string trimString = " \r\n\t";

    int nPos = value.find_last_not_of( trimString );
    if( nPos != std::string::npos ){
      value = value.substr( 0, nPos+1 );
    }

    nPos = value.find_first_not_of( trimString );
    if( nPos != std::string::npos ){
      value = value.substr( nPos );
    }

    return value;
  }
};

#define JSON_SHARED_PTR 1

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

public:
  static std::shared_ptr<JSON> parse(std::string jsonString, std::shared_ptr<JSON> pJson = nullptr);
  virtual std::shared_ptr<JSON> getSharedPtr(void){
    return shared_from_this();
  }

public:
  JSON(std::string jsonString = ""):mType(JSON_TYPE::TYPE_HASH), mValue(""){
    JSON::parse( jsonString );
  };
  virtual ~JSON(){};

  virtual int getCount(void){
    switch( mType ){
      case JSON_TYPE::TYPE_ARRAY:
        return mArrayData.size();
      case JSON_TYPE::TYPE_HASH:
        return mHashData.size();
      default:
        return mValue.empty() ? 1 : 0;
    }
  };

  std::shared_ptr<JSON> operator[](std::string key){
    if( !mHashData.contains( key ) ){
      std::shared_ptr<JSON> json = std::make_shared<JSON>();
      mHashData[key] = json;
    }
    return mHashData[key];
  };

  std::shared_ptr<JSON> operator[](int nIndex){
    std::shared_ptr<JSON> jsonValue;
    switch( mType ){
      case JSON_TYPE::TYPE_ARRAY:
        jsonValue = mArrayData[nIndex];
        break;
      case JSON_TYPE::TYPE_HASH:
      default:
        jsonValue = mHashData[ std::to_string(nIndex) ];
        break;
    }
    return jsonValue ? jsonValue : std::make_shared<JSON>();
  };

  std::string getString(void){
    switch( mType ){
      case JSON_TYPE::TYPE_ARRAY:
        return "TYPE_ARRAY";
        break;
      case JSON_TYPE::TYPE_HASH:
        return "TYPE_HASH";
      case JSON_TYPE::TYPE_VALUE:
      default:
        return mValue;
    }
  };

  bool operator==(std::string value){ return mValue == value; };

  void setValue(std::string value){
    mValue = value;
    mType = JSON_TYPE::TYPE_VALUE;
  }
  void setValue(int value){ setValue( std::to_string( value ) ); };
  void setValue(float value){
    std::string strVal = std::to_string( value );
    while( strVal.ends_with("0") ){
      strVal = strVal.substr(0, strVal.size()-1);
    }
    setValue( strVal );
  };
  void setValue(bool value){  setValue( std::string( value ? "true" : "false" ) ); };

  std::shared_ptr<JSON> operator=(std::string value){
    setValue(value);
    return getSharedPtr();
  }

  std::shared_ptr<JSON> operator=(int value){
    setValue(value);
    return getSharedPtr();
  }
  std::shared_ptr<JSON> operator=(float value){
    setValue(value);
    return getSharedPtr();
  }

  int getInt(void){ return std::stoi( getString() ); };
  float getFloat(void){ return std::stof( getString() ); };
  bool getBoolean(void){ return getString() == "true"; };
  bool isNull(void){
    std::string val = getString();
    return mType == JSON_TYPE::TYPE_VALUE && ( val.empty() || val == "null" );
  }


  virtual void push_back(std::shared_ptr<JSON> jsonValue){
    mType = JSON_TYPE::TYPE_ARRAY;
    mArrayData.push_back( jsonValue );
  }

  virtual void push_back(std::string value){
    std::shared_ptr<JSON> jsonValue = std::make_shared<JSON>();
    *jsonValue = value;
    push_back( jsonValue );
  }
};

#else /* JSON_SHARED_PTR */
#endif /* JSON_SHARED_PTR */

class JSONArray : public JSON
{
public:
  JSONArray(){ mType = JSON_TYPE::TYPE_ARRAY; };
  virtual ~JSONArray(){};
};

#endif /* __JSON_HPP__ */