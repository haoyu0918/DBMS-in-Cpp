//
//  Attribute.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <variant>
#include <unordered_map>
#include "Value.hpp"
using DefaultType = std::variant<int, float,bool, std::string>;
namespace ECE141 {
      
  /*enum class DataType {
    no_type='N', bool_type='B', datetime_type='D', float_type='F', int_type='I',  varchar_type='V',
  };*/
  static std::unordered_map<char, DataType> Char2Datatype{{'N', DataType::no_type},
                                                {'B', DataType::bool_type},
                                                {'D', DataType::datetime_type},
                                                {'F', DataType::float_type},
                                                {'I', DataType::int_type},
                                                {'V', DataType::varchar_type}
                                                };
  static std::unordered_map<DataType, std::string> DataType2String{{DataType::no_type, "no type"},
                                                                   {DataType::datetime_type, "date"},
                                                                   {DataType::float_type, "float"},
                                                                   {DataType::int_type, "integer"},
                                                                   {DataType::varchar_type, "varchar"},
                                                                   {DataType::bool_type, "boolean"}};
  class Attribute {
  protected:
    std::string   name;
    DataType      type;
    //STUDENT: What other data should you save in each attribute?
    size_t length;
    bool auto_increment;
    bool isPrimaryKey;
    bool nullable;
    bool hasDefault;
    std::string DefaultValue;
  public:
          
    Attribute(DataType aType=DataType::no_type);
    Attribute(std::string aName, DataType aType /*maybe more?*/);
    Attribute(const Attribute &aCopy);
    ~Attribute();
    Attribute& operator = (const Attribute& aCopy){
        name = aCopy.name;
        type = aCopy.type;
        length = aCopy.length;
        auto_increment = aCopy.auto_increment;
        isPrimaryKey = aCopy.isPrimaryKey;
        nullable = aCopy.nullable;
        hasDefault = aCopy.hasDefault;
        DefaultValue = aCopy.DefaultValue;
        return *this;
    }
    Attribute&    setName(std::string aName){
        name = aName;
        return *this;
    };
    Attribute&    setType(DataType aType){
        type = aType;
        return *this;
    };

    bool          isValid(); //is this schema valid? Are all the attributes value?
    
    const std::string&  getName() const {return name;}
    DataType            getType() const {return type;}
    Attribute& setlength(size_t aLen) {
        length = aLen;
        return *this;
    }
    size_t getlength(){
        return length;
    }
    Attribute& setAuto_Incre(bool tmp){
        auto_increment = tmp;
        return *this;
    }
    bool iftAuto_increment(){
        return auto_increment;
    }
    Attribute& setPrimaryKey(bool tmp){
        isPrimaryKey = tmp;
        if(tmp) nullable = false;
        return *this;
    }
    bool ifPrimarykey(){
        return isPrimaryKey;
    }
    Attribute& setNullable(bool tmp){
        nullable = tmp;
        return *this;
    }
    bool ifNullable(){
        return nullable;
    }
    DefaultType  getDefaultValue(){
        return DefaultValue;
    }
    bool hasDefaultValue(){
        return hasDefault;
    }
    Attribute& setDefaultValue(std::string aValue){
        hasDefault = true;
        /*if(aValue.index() == 3){
            if(type == DataType::int_type) DefaultValue = std::stoi(std::get<std::string>(aValue));
            else if(type == DataType::float_type) DefaultValue = std::stof(std::get<std::string>(aValue));
            else if(type == DataType::bool_type) {
                char c = std::get<std::string>(aValue)[0];
                if(c == 'T' || c == 't') DefaultValue = true;
                else DefaultValue = false;
            }
            else DefaultValue = aValue;
        }
        else DefaultValue = aValue;*/
        DefaultValue = aValue;
        return *this;
    }
    std::string TypeToString(){
       if(type == DataType::varchar_type) {
           std::string res = "varchar(";
           res += std::to_string(length);
           res += ")";
           return res;
       }
       else return DataType2String[type];
    }
    //STUDENT: are there other getter/setters to manage other attribute properties?
    friend class Schema;
    friend class CreateTableStatement;
  };
  

}


#endif /* Attribute_hpp */
