//
//  Schema.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <stdio.h>
#include <vector>
#include "Attribute.hpp"
#include "Errors.hpp"
#include <optional>
#include <iostream>
#include "Storage.hpp"
#include <iomanip>
//#include "Row.hpp"
using StringOp = std::optional<std::string>;
namespace ECE141 {
  using AttrOpt = std::optional<Attribute>;
  struct Block;
  struct Expression;
  class  Database;
  class  Tokenizer;
  
  //using StringList = std::vector<std::string>;
  //using attribute_callback = std::function<bool(const Attribute &anAttribute)>;
  
  using AttributeList = std::vector<Attribute>;
  
  //STUDENT: If you're using the Storable interface, add that to Schema class?

  class Schema : public Storable{
  public:
                          Schema(const std::string aName);
                          Schema(const Schema &aCopy);
    
                          ~Schema();
        
    const std::string&    getName() const {return name;}
    const AttributeList&  getAttributes() const {return attributes;}
    uint32_t              getBlockNum() const {return blockNum;}
    Schema&               setBlockNum(uint32_t index){ blockNum = index; return *this;}
    bool                  isChanged() {return changed;}
    
    Schema&               addAttribute(const Attribute &anAttribute);
    AttrOpt               getAttribute(const std::string &aName) const ;
    Schema&               PrintInfo(){
        for(auto tmp: attributes){
            std::cerr << tmp.getName() << " " << static_cast<char>(tmp.getType()) << " " << tmp.getlength() << std::endl;
        }
        return *this;
    }
    Schema&              TableView(std::ostream& anOutput);
      
        //STUDENT: These methods will be implemented in the next assignment...
    
    //Value                 getDefaultValue(const Attribute &anAttribute) const;
    //StatusResult          validate(KeyValues &aList);
    
    StringOp           getPrimaryKeyName() const;
    uint32_t              getNextAutoIncrementValue(){
        return  (++total_data_num);
    };

      StatusResult  encode(std::ostream &aWriter);
      StatusResult  decode(std::istream &aReader);
      BlockType     getType() const;
      std::vector<std::string> GetAutoIncrementName(){
          std::vector<std::string> res;
          for(auto& attr: attributes){
              if(attr.iftAuto_increment()) res.push_back(attr.getName());
          }
          return res;
      }
      std::string getNameandPrimaryKey(){
          std::string res = name + ".";
          res += getPrimaryKeyName().value();
          return res;
      }
    //STUDENT: Do you want to provide an each() method for observers?
    
    friend class Database; //is this helpful?
  protected:
    uint32_t        total_data_num;
    AttributeList   attributes;
    std::string     name;
    uint32_t        blockNum;  //storage location.
    bool            changed;
  };
  
}
#endif /* Schema_hpp */
