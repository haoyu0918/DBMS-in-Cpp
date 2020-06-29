//
//  Attribute.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Attribute.hpp"

namespace ECE141 {

 //STUDENT: Implement the attribute class here...
 Attribute::Attribute(DataType aType):type(aType), name(""), length(0), auto_increment(false), isPrimaryKey(false), nullable(true), DefaultValue("NULL"), hasDefault(false){}
 Attribute::Attribute(std::string aName, DataType aType):name(aName), type(aType), length(0), auto_increment(false), isPrimaryKey(false), nullable(true), DefaultValue("NULL"), hasDefault(false){}
 Attribute::Attribute(const Attribute& aCopy){
     type = aCopy.type;
     name = aCopy.name;
     length = aCopy.length;
     hasDefault = aCopy.hasDefault;
     auto_increment = aCopy.auto_increment;
     isPrimaryKey = aCopy.isPrimaryKey;
     nullable = aCopy.nullable;
     DefaultValue = aCopy.DefaultValue;
 }
 Attribute::~Attribute() {}
}
