//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"

namespace ECE141 {
  
  //STUDENT: You need to fully implement these methods...

  Row::Row() {}
  Row::Row(const Row &aRow) {
      rowdata.clear();
      for(auto thepair : aRow.rowdata){
          rowdata[thepair.first] = thepair.second;
      }
  }
  Row& Row::operator=(const Row &aRow) {
      rowdata.clear();
      for(auto thepair : aRow.rowdata){
          rowdata[thepair.first] = thepair.second;
      }
      return *this;
  }
  bool Row::operator==(Row &aCopy) const {return false;}
  Row::~Row() {}
  StatusResult Row::encode(std::ostream &aWriter) {
      aWriter << rowdata.size() << " "; // total number of the data
      for(auto thepair: rowdata){
          // the fromation is "key : value,
          aWriter << thepair.first << ' ' << ':';
          thepair.second.encode(aWriter);
          aWriter << ',';
      }
      return StatusResult();
  }
  StatusResult Row::decode(std::istream &aReader) {
      rowdata.clear();
      int count = 0;
      aReader >> count;
      for(int i = 0; i<count ; i++){
          std::string attr_name;
          char c;
          aReader >> attr_name >> c; // decode name from file
          ValueType aValue;
          aValue.decode(aReader);
          aReader >> c;   // get the ',';
          rowdata[attr_name] = aValue;
      }
      return StatusResult{};
  }

}
