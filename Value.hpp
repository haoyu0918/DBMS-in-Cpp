//
//  Value.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Value_h
#define Value_h

#include <variant>
#include <optional>
#include <string>
#include <map>
#include <cstdint>
#include "Errors.hpp"
#include "StorageBlock.hpp"
#include <unordered_map>

namespace ECE141 {


  struct Storable {
     virtual StatusResult  encode(std::ostream &aWriter)=0;
     virtual StatusResult  decode(std::istream &aReader)=0;
      virtual BlockType     getType() const = 0;
      virtual uint32_t getHeaderId() const{
          return 0;
      };
      virtual void setHeaderId(uint32_t anum){
      };
  };
    enum class DataType {
        no_type='N', bool_type='B', datetime_type='D', float_type='F', int_type='I', varchar_type='V',
    };

  using vartype = std::variant<uint32_t, float, bool, std::string>;
    struct Visitor{
        std::ostream& anOutput;
        Visitor(std::ostream& anOut): anOutput(anOut){}
        void operator()(uint32_t aValue){
            anOutput << aValue;
        }
        void operator()(bool aValue){
            anOutput << (aValue? "true": "false");
        }
        void operator()(std::string& aValue){
            anOutput << aValue;
        }
        void operator()(float avalue){
            anOutput << avalue;
        }
    };
    struct ValueWriter{
        std::ostream& awriter;
        ValueWriter(std::ostream& anOutput):awriter(anOutput){}
        template<typename T>
        StatusResult BinaryWrite(const T& value, char type){
            awriter.write((char*)&type, 1);
            awriter.write((const char*)&value, sizeof(T));
            return StatusResult{};
        }
        StatusResult operator()(const uint32_t anum){
            //std::clog << "encoding uint32 here" << std::endl;
            return BinaryWrite<uint32_t >(anum, 'I');
        }
        StatusResult operator()(const float anum){
            //std::clog << "encoding float here" << std::endl;
            return BinaryWrite<float>(anum, 'F');
        }
        StatusResult operator()(const std::string astr){
            //std::clog << "encoding string here" << std::endl;
            char c = 'V';
            awriter.write(&c, sizeof(char));
            int len = astr.size();
            awriter.write(reinterpret_cast<char*>(&len), sizeof(int));
            awriter.write(astr.c_str(), astr.size());
            return StatusResult{};
        }
        StatusResult operator()(const bool thevalue){
            //std::clog << "encoding bool here" << std::endl;
            return BinaryWrite<bool>(thevalue, 'B');
        }

    };

  struct ValueType: public Storable{
      vartype data;
      ValueType(const std::string& aName):data(aName){}
      ValueType(uint32_t anum):data(anum){}
      ValueType(float aNum): data(aNum){}
      ValueType(bool v): data(v){}
      ValueType&  operator=(const ValueType& ano){
          data = ano.data;
          return *this;
      }
      ValueType& operator = (const std::string& astring){
          data = astring;
          return *this;
      }
      ValueType& operator = (uint32_t anum){
          data = anum;
          return *this;
      }
      ValueType& operator = (float anum){
          data = anum;
          return *this;
      }
      ValueType& operator = (bool aValue){
          data = aValue;
          return *this;
      }
      ValueType(){}
      StatusResult decode(std::istream& aReader){
          char c = 'N';
          aReader.read(&c, sizeof(char));
          if(c == 'I'){
              uint32_t  thenum;
              aReader.read((char*)&thenum, sizeof(uint32_t));
              //std::clog << thenum << std::endl;
              data = thenum;
          }
          else if(c == 'F'){
              float thenum;
              aReader.read((char*)&thenum,sizeof(float));
              //std::clog << thenum << std::endl;
              data = thenum;
          }
          else if( c == 'B'){
              bool thevalue;
              aReader.read((char*)&thevalue, sizeof(bool));
             // std::clog << thevalue << std::endl;
              data = thevalue;
          }
          else if(c == 'V'){
              int len;
              char buffer[200];
              memset(buffer, 0, 200);
              aReader.read((char*)&len, sizeof(int));
              aReader.read(buffer, len);
              //std::clog << buffer << std::endl;
              data = std::string(buffer);
          }
          return StatusResult{};
      }
      StatusResult encode(std::ostream& aWriter) {
          return std::visit(ValueWriter{aWriter}, data);
      }
      BlockType getType() const{
          return BlockType::unknown_block;
      }
  };
  using KeyValues = std::unordered_map<std::string, ValueType>;

}

#endif /* Value_h */
