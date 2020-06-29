//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "Storage.hpp"
//#include "Database.hpp"
#include "Value.hpp"
namespace ECE141 {

  struct Row : public Storable {
  public:

    Row();
    Row(const Row &aRow);
    ~Row();
    Row& operator=(const Row &aRow);
    bool operator==(Row &aCopy) const;
    
      //STUDENT: What other methods do you require?
      Row& ShowData(std::ostream& anOutput){
          //std::clog << "--------" << std::endl;
          //std::clog << "show data here" << std::endl;
          for(auto thepair : rowdata){
              anOutput << thepair.first << "=";
              std::visit(Visitor{anOutput}, thepair.second.data);
              anOutput << ',';
          }
          return *this;
      }
      StatusResult  encode(std::ostream &aWriter);
      StatusResult  decode(std::istream &aReader);

      BlockType     getType() const{
          return BlockType::data_block;
      };
      uint32_t getHeaderId() const{
          return HeaderId;
      };
      void setHeaderId(uint32_t anum){
          HeaderId = anum;
      };

    //KeyValues data;  //you're free to change this if you like...
    KeyValues  rowdata;
    uint32_t HeaderId; // save the hashing number of table containing it...
  };
  using RowCollection = std::vector<Row>;

}

#endif /* Row_hpp */
