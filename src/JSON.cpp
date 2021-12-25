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

#include "JSON.hpp"
#include <vector>
#include <iostream>

class StringUtil
{
public:
  static std::string trim(std::string value, std::string trimStrings )
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
};


class StringTokenizer
{
protected:
  std::string mBuf;
  std::string mToken;
  int mPos;
  int mBufLength;
  int mTokenLength;

public:
  StringTokenizer(std::string sourceString, std::string token) : mBuf(sourceString), mToken(token), mPos(0)
  {
    mBufLength = sourceString.length();
    mTokenLength = token.length();
  }
  virtual ~StringTokenizer(){};

  std::string getNext(){
    std::string result = "";

    int pos = mBuf.find( mToken, mPos );
    if( pos != std::string::npos ){
      result = mBuf.substr( mPos, pos - mPos );
      mPos = pos + mTokenLength;
    } else {
      result = mBuf.substr( mPos );
      mPos = mBuf.length();
    }

    return result;
  }
  bool hasNext(void){
    return ( mPos < mBufLength ) ? true : false;
  }
};


std::shared_ptr<JSON> JSON::getSharedPtr(void)
{
  return shared_from_this();
}

JSON::JSON(std::shared_ptr<JSON> pJsonUplink):mType(JSON_TYPE::TYPE_HASH), mValue(""), mUplinkRef(pJsonUplink)
{
}

JSON::~JSON()
{

}

int JSON::getCount(void)
{
  switch( mType ){
    case JSON_TYPE::TYPE_ARRAY:
      return mArrayData.size();
    case JSON_TYPE::TYPE_HASH:
      return mHashData.size();
    default:
      return mValue.empty() ? 1 : 0;
  }
}

std::shared_ptr<JSON> JSON::operator[](std::string key){
  std::shared_ptr<JSON> result = std::make_shared<JSON>();
  *result = JSON_NULL;

  if( isArray() ){
    int nIndex = getIndex( key );
    if( nIndex>=0 && nIndex<mArrayData.size() ){
      result = mArrayData[ nIndex ];
    }
  } else {
    if( mHashData.contains( key ) ){
      result = mHashData[key];
    } else {
      insert_or_assign( key, result );
    }
  }
  return result;
}


std::shared_ptr<JSON> JSON::operator[](int nIndex)
{
  std::shared_ptr<JSON> jsonValue;
  switch( mType ){
    case JSON_TYPE::TYPE_ARRAY:
      if( mArrayData.size() < ( nIndex + 1 ) ){
        mArrayData.resize( nIndex + 1 );
      }
      jsonValue = mArrayData[nIndex];
      break;
    case JSON_TYPE::TYPE_HASH:
    default:
      jsonValue = mHashData[ std::to_string(nIndex) ];
      break;
  }
  return jsonValue ? jsonValue : std::make_shared<JSON>();
}

std::string JSON::getString(void)
{
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
}

bool JSON::operator==(std::string value)
{
  return mValue == value;
}

void JSON::setValue(const char* value)
{
  setValue( std::string(value) );
}

void JSON::setValue(std::string value)
{
  mValue = value;
  mType = JSON_TYPE::TYPE_VALUE;
  mHashData.clear();
  mArrayData.clear();

  std::shared_ptr<JSON> pUplinkJson = mUplinkRef.lock();
  if( pUplinkJson ){
    pUplinkJson->update( getSharedPtr() );
  }
}

void JSON::setValue(int value)
{
  setValue( std::to_string( value ) );
}

void JSON::setValue(float value)
{
  std::string strVal = std::to_string( value );
  while( strVal.ends_with("0") ){
    strVal = strVal.substr(0, strVal.size()-1);
  }
  setValue( strVal );
}

void JSON::setValue(bool value)
{
  setValue( std::string( value ? "true" : "false" ) );
}

std::shared_ptr<JSON> JSON::operator=(const char* value)
{
  setValue(value);
  return getSharedPtr();
}

std::shared_ptr<JSON> JSON::operator=(std::string value)
{
  setValue(value);
  return getSharedPtr();
}

std::shared_ptr<JSON> JSON::operator=(int value)
{
  setValue(value);
  return getSharedPtr();
}

std::shared_ptr<JSON> JSON::operator=(float value)
{
  setValue(value);
  return getSharedPtr();
}

std::shared_ptr<JSON> JSON::operator=(std::shared_ptr<JSON> pJson)
{
  std::shared_ptr<JSON> pUplink = mUplinkRef.lock();
  if( pUplink ){
    std::string key = pUplink->getKeyFromJsonObject( getSharedPtr() );
    pUplink->insert_or_assign( key, pJson );
  }

  return getSharedPtr();
}

std::shared_ptr<JSON> JSON::insert_or_assign(std::string key, std::shared_ptr<JSON> pJson)
{
  mType = JSON_TYPE::TYPE_HASH;
  mArrayData.clear();
  mValue.clear();
  if( pJson ){
    pJson->setUplink( getSharedPtr() );
  }
  mHashData.insert_or_assign( key, pJson );
  return getSharedPtr();
}

std::string JSON::getKeyFromJsonObject(std::shared_ptr<JSON> pJson)
{
  std::string result;

  for( auto& [key, aJson] : mHashData ){
    if( aJson == pJson ){
      result = key;
      break;
    }
  }

  return result;
}


std::shared_ptr<JSON> JSON::update(std::shared_ptr<JSON> pJson)
{
  std::shared_ptr<JSON> result;

  for( auto& [key, aJson] : mHashData ){
    if( aJson == pJson ){
      insert_or_assign( key, pJson );
      result = pJson;
      break;
    }
  }

  return result;
}

void JSON::setUplink(std::shared_ptr<JSON> pUplinkJson)
{
  mUplinkRef = pUplinkJson;
}

int JSON::getInt(void)
{
  int result = 0;
  try {
    result = std::stoi( getString() );
  } catch (...) {
    result = 0;
  }
  return result;
}

float JSON::getFloat(void)
{
  float result = 0.0f;
  try {
    result = std::stof( getString() );
  } catch (...) {
    result = 0.0f;
  }
  return result;
}

bool JSON::getBoolean(void)
{
  return getString() == "true";
}

bool JSON::isNull(void)
{
  std::string val = getString();
  return mType == JSON_TYPE::TYPE_VALUE && ( val.empty() || val == JSON_NULL );
}

bool JSON::isHash(void)
{
  return mType == JSON_TYPE::TYPE_HASH;
}

bool JSON::isArray(void)
{
  return mType == JSON_TYPE::TYPE_ARRAY;
}

bool JSON::isValue(void)
{
  return mType == JSON_TYPE::TYPE_VALUE;
}

bool JSON::hasOwnProperty(std::string key)
{
  if( isHash() ){
    return mHashData.contains( key );
  } else if( isArray() ){
    int nIndex = getIndex( key );
    return nIndex!=-1 && nIndex < mArrayData.size();
  }
  return false;
}

void JSON::push_back(std::shared_ptr<JSON> jsonValue)
{
  mType = JSON_TYPE::TYPE_ARRAY;
  mHashData.clear();
  mValue.clear();
  if( jsonValue ){
    jsonValue->setUplink( getSharedPtr() );
  }
  mArrayData.push_back( jsonValue );
}

void JSON::push_back(std::string value)
{
  std::shared_ptr<JSON> jsonValue = std::make_shared<JSON>();
  *jsonValue = value;
  push_back( jsonValue );
}

std::string JSON::getHierachyKey(std::vector<std::string> keys)
{
  std::string hierachyKey;

  for( auto& aKey : keys ){
    if( !aKey.empty() ){
      hierachyKey = hierachyKey + "." + aKey;
    }
  }
  if( hierachyKey.starts_with(".") ){
    hierachyKey = hierachyKey.substr(1, hierachyKey.size() - 1 );
  }

  return hierachyKey;
}

int JSON::getIndex(std::string key)
{
  int result = -1;
  try {
    result = std::stoi(key);
  } catch (...) {
    result = -1;
  }
  return result;
}


std::shared_ptr<JSON> JSON::getObjectRelativePath(std::string key, bool bForceEnsure)
{
  std::shared_ptr<JSON> pRef = getSharedPtr();

  StringTokenizer hierachyKeys( key, "." );

  while( hierachyKeys.hasNext() ){
    std::string theKey = hierachyKeys.getNext();
    bool isArray = false;
    if( theKey.starts_with("[") && theKey.ends_with("]")){
      theKey = theKey.substr( 1, theKey.size() - 2 );
      isArray = true;
    }
    int nIndex = getIndex( theKey );

    if( pRef && !theKey.empty() ){
      if( pRef->hasOwnProperty( theKey ) ){
        if( pRef->isHash() ){
          pRef = (*pRef)[ theKey ];
        } else if( pRef->isArray() ){
          if( nIndex>=0 ){
            pRef = (*pRef)[ nIndex ];
          } else {
            std::shared_ptr<JSON> tmp = std::make_shared<JSON>();
            pRef->push_back( tmp );
            pRef = tmp;
          }
        }
      } else {
        // not found
        std::shared_ptr<JSON> tmp = std::make_shared<JSON>();
        if( !isArray ){
          pRef->insert_or_assign( theKey, tmp );
        } else {
          pRef->push_back( tmp );
        }
        pRef = tmp;
        if( !bForceEnsure ) break;
      }
    }
  }

  if( !pRef ){
    pRef = std::make_shared<JSON>();
  }

  return pRef;
}

std::string JSON::getStringRelativePath(std::string key)
{
  std::shared_ptr<JSON> pRef = getObjectRelativePath( key );
  return pRef ? pRef->getString() : "";
}


void JSON::setObjectRelativePath(std::string key, std::shared_ptr<JSON> pValue)
{
  std::shared_ptr<JSON> pRef = getObjectRelativePath( key, true );

  if( pRef ){
    if( pValue->isValue() ){
      *pRef = pValue->getString();
    } else {
      *pRef = pValue;
    }
  }
}

void JSON::setValueRelativePath(std::string key, const char* value)
{
  setValueRelativePath( key, std::string( value ) );
}

void JSON::setValueRelativePath(std::string key, std::string value)
{
  std::shared_ptr<JSON> pValue = std::make_shared<JSON>();
  pValue->setValue ( value );
  setObjectRelativePath( key, pValue );
}

void JSON::setValueRelativePath(std::string key, int value)
{
  setValueRelativePath( key, std::to_string( value ) );
}

void JSON::setValueRelativePath(std::string key, float value)
{
  setValueRelativePath( key, std::to_string( value ) );
}

void JSON::setValueRelativePath(std::string key, bool value)
{
  setValueRelativePath( key, value ? "true" : "false" );
}



std::map<std::string, std::shared_ptr<JSON>>::iterator JSON::begin()
{
  return mHashData.begin();
}

std::map<std::string, std::shared_ptr<JSON>>::iterator JSON::end()
{
  return mHashData.end();
}

std::vector<std::shared_ptr<JSON>>::iterator JSON::getArrayIteratorBegin()
{
  return mArrayData.begin();
}

std::vector<std::shared_ptr<JSON>>::iterator JSON::getArrayIteratorEnd()
{
  return mArrayData.end();
}


std::shared_ptr<JSON> JSON::parse(std::string jsonString, std::shared_ptr<JSON> pJson)
{
  if( !pJson ){
    pJson = std::make_shared<JSON>();
  }
  JSON::JSON_TYPE type = JSON::JSON_TYPE::TYPE_HASH;
  JSON::JSON_TYPE prevType = JSON::JSON_TYPE::TYPE_VALUE;
  std::vector<JSON::JSON_TYPE> queue;
  int nKeyStart = 0, nKeyEnd = 0, nValueStart = 0, nValueEnd = 0;
  bool bFoundValue = false;
  bool bFoundBlacket = false;
  bool bFoundEnd = false;
  std::vector<std::string> keyQueue;
  std::map<std::string, int> arrayQueue;
  std::vector<std::string> flatData;
  std::string key;
  std::string theHierachyKey;

  for(int i=0, nSize = jsonString.size(); i<nSize; i++){
    std::string c = jsonString.substr( i, 1 );

    // detect brackets
    switch( c.c_str()[0] )
    {
      case '{':
        {
          type = JSON::JSON_TYPE::TYPE_HASH;
          queue.push_back( type );
          keyQueue.push_back( key );
          key = "";
          nKeyStart = i + 1;
        }
        break;
      case '}':
        {
          bFoundEnd = true;
          queue.pop_back();
          type = queue.back();
          nValueEnd = i - 1;
          bFoundValue = true;
        }
        break;
      case '[':
        {
          bFoundBlacket = true;
          type = JSON::JSON_TYPE::TYPE_ARRAY;
          queue.push_back( type );
          keyQueue.push_back( key );
          key = "";
          nKeyStart = i + 1;
          nValueStart = i + 1;
        }
        break;
      case ']':
        {
          bFoundEnd = true;
          queue.pop_back();
          type = queue.back();
          nValueEnd = i - 1;
          bFoundValue = true;
        }
        break;
      case ':':
        {
          nKeyEnd = i - 1;
          nValueStart = i + 1;
          bFoundValue = true;
        }
        break;
      case ',':
        {
          nKeyStart = nValueEnd = i - 1;
          bFoundValue = true;
        }
        break;
      default:;
    }

    if( bFoundValue ){
      std::string value;
      if( nKeyEnd >= nKeyStart ){
        key = StringUtil::trim( jsonString.substr( nKeyStart, nKeyEnd-nKeyStart + 1 ), " \"[],{}:" );
        if( !key.empty() ){
          nKeyStart = nKeyEnd + 2;
          nValueStart = nKeyStart;
        }
        theHierachyKey = JSON::getHierachyKey( keyQueue );
        if( theHierachyKey.empty() ){
          theHierachyKey = key;
        } else {
          if( !key.empty() ){
            theHierachyKey = theHierachyKey + "." + key;
          }
        }
      }

      if( nValueEnd >= nValueStart){
        value = StringUtil::trim( jsonString.substr( nValueStart, nValueEnd-nValueStart + 1 ), " \"[],{}:" );
        if( !value.empty() ){
          nValueStart = nKeyStart = i + 1;
          key = "";
          std::string data;
          if( arrayQueue.contains( theHierachyKey ) ){
            data = theHierachyKey + ".[" + std::to_string( arrayQueue[theHierachyKey] ) + "]=" + value;
            arrayQueue[theHierachyKey] = arrayQueue[theHierachyKey] + 1;
          } else {
             data = theHierachyKey + "=" + value;
          }
//          std::cout << data << std::endl;
          flatData.push_back( data );
        }
      }
    }

    // handler of each JSON object mode
    if( type != prevType ){
      switch( type )
      {
        case JSON::JSON_TYPE::TYPE_HASH:
          {
          }
          break;
        case JSON::JSON_TYPE::TYPE_ARRAY:
          {
            if( bFoundBlacket && !arrayQueue.contains( theHierachyKey ) ){
              arrayQueue.insert_or_assign( theHierachyKey, 0 );
              std::string data = theHierachyKey + "=[]";
              flatData.push_back( data );
            }
          }
          break;
        case JSON::JSON_TYPE::TYPE_VALUE:
          {
          }
          break;
      }
    }

    prevType = type;
    bFoundValue = false;
    bFoundBlacket = false;
    if( bFoundEnd ){
      keyQueue.pop_back();
      bFoundEnd = false;
    }
  }

  for( auto& aData : flatData ){
    StringTokenizer tok( aData, "=" );
    if( tok.hasNext() ){
      std::string key = tok.getNext();
      std::shared_ptr<JSON> pValue = std::make_shared<JSON>();
      *pValue = tok.getNext();
      std::cout << "--- " << key << "=" << pValue->getString() << std::endl;
      if( pValue->getString() == "[]" ){
        pValue = std::make_shared<JSONArray>();
      }
      pJson->setObjectRelativePath( key, pValue );
    }
  }

  // debug dump
  std::cout << "debug dump" << std::endl;
  dump( pJson );

  return pJson;
}

void JSON::dump(JSON_REF_TYPE json, std::string rootKey)
{
  rootKey = rootKey.empty() ? rootKey : rootKey+".";
#if JSON_SHARED_PTR
  if( json ){
    if( json->isHash() ){
      for( auto& [key, pJson] : *json ){
        std::string theKey = rootKey+key;
        if( pJson->isValue() ){
          std::cout << "[" << theKey << "]=" << pJson->getString() << std::endl;
        } else {
          dump( pJson, theKey );
        }
      }
    } else if( json->isArray() ){
      int i=0;
      for( auto&& it = json->getArrayIteratorBegin(); it!= json->getArrayIteratorEnd(); it++){
        std::string theKey = rootKey + "[" + std::to_string( i++ ) +"]";
        if( (*it)->isValue() ){
          std::cout << "[" << theKey << "]=" << (*it)->getString() << std::endl;
        } else {
          dump( *it, theKey );
        }
      }
    }
  }
#else /* JSON_SHARED_PTR */
  if( json.isHash() ){
    for( auto& [key, aJson] : json ){
      std::string theKey = rootKey + key;
      if( pJson->isValue() ){
        std::cout << "[" << theKey << "]=" << aJson.getString() << std::endl;
      } else {
        dump( aJson, theKey );
      }
    }
  } else if( json.isArray() ){
    int i=0;
    for( auto&& it = json.getArrayIteratorBegin(); it!= json.getArrayIteratorEnd(); it++){
        std::string theKey = rootKey + std::to_string( i++ );
      if( (*it).isValue() ){
        std::cout << "[" << theKey << "]=" << (*it).getString() << std::endl;
      } else {
        dump( *it, theKey );
      }
    }
  }
#endif /* JSON_SHARED_PTR */
}

