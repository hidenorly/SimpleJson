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

std::string getHierachyKey(std::vector<std::string> keys)
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

std::shared_ptr<JSON> JSON::parse(std::string jsonString, std::shared_ptr<JSON> pJson)
{
  if( pJson ){
    pJson = std::make_shared<JSON>();
  }
  JSON::JSON_TYPE type = JSON::JSON_TYPE::TYPE_HASH;
  JSON::JSON_TYPE prevType = JSON::JSON_TYPE::TYPE_VALUE;
  std::vector<JSON::JSON_TYPE> queue;
  int nKeyStart = 0, nKeyEnd = 0, nValueStart = 0, nValueEnd = 0;
  bool bFound = false;
  std::vector<std::string> keyQueue;
  std::vector<std::string> flatData;
  std::string key;

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
          keyQueue.pop_back();
          queue.pop_back();
          type = queue.back();
          nValueEnd = i - 1;
          bFound = true;
        }
        break;
      case '[':
        {
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
          keyQueue.pop_back();
          queue.pop_back();
          type = queue.back();
          nValueEnd = i - 1;
          bFound = true;
        }
        break;
      case ':':
        {
          nKeyEnd = i - 1;
          nValueStart = i + 1;
          bFound = true;
        }
        break;
      case ',':
        {
          nKeyStart = nValueEnd = i - 1;
          bFound = true;
        }
        break;
      default:;
    }

    if( bFound ){
      std::string value;
      if( nKeyEnd >= nKeyStart ){
        key = StringUtil::trim2( jsonString.substr( nKeyStart, nKeyEnd-nKeyStart + 1 ), " \"[],{}:" );
        if( !key.empty() ){
          nKeyStart = nKeyEnd + 2;
          nValueStart = nKeyStart;
        }
      }

      if( nValueEnd >= nValueStart){
        value = StringUtil::trim2( jsonString.substr( nValueStart, nValueEnd-nValueStart + 1 ), " \"[],{}:" );
        if( !value.empty() ){
          nValueStart = nKeyStart = i + 1;
          std::string theKey = getHierachyKey( keyQueue );
          if( theKey.empty() ){
            theKey = key;
          } else {
            if( !key.empty() ){
              theKey = theKey + "." + key;
            }
          }
          key = "";
          std::string data = theKey + "=" + value;
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
          }
          break;
        case JSON::JSON_TYPE::TYPE_VALUE:
          {
          }
          break;
      }
    }

    prevType = type;
    bFound = false;
  }

  // debug dump
  for( auto& aData : flatData ){
    std::cout << aData << std::endl;
  }

  return pJson;
}
