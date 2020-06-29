//
//  Schema.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Schema.hpp"

namespace ECE141 {

 //STUDENT: Implement the Schema class here...
Schema::Schema(const std::string aName): name(aName), total_data_num(0){
}
Schema::Schema(const Schema& aCopy){
    name = aCopy.name;
    attributes = aCopy.attributes;
    blockNum = aCopy.blockNum;
    changed = aCopy.changed;
    total_data_num = aCopy.total_data_num;
}
Schema::~Schema() {}
Schema& Schema::addAttribute(const Attribute &anAttribute) {
    changed = true;
    attributes.push_back(anAttribute);
    return *this;
}
AttrOpt   Schema::getAttribute(const std::string &aName) const {
    for(int i = 0; i<attributes.size(); i++){
        if(attributes[i].name == aName) return attributes[i];
    }
    return std::nullopt;
}
StringOp Schema::getPrimaryKeyName() const {
    std::vector<std::string> names;
    for(auto att:attributes){
        if(att.ifPrimarykey()) names.push_back(att.getName());
    }
    if(names.size() != 1) return std::nullopt;
    else return names[0];
}
BlockType Schema::getType() const {
    return BlockType::entity_block;
}
StatusResult Schema::encode(std::ostream &aWriter) {
    aWriter << name << ' ' << attributes.size() << ' ';
    for(auto attr : attributes){
        aWriter << attr.name << " " << static_cast<char>(attr.type) << " " << attr.auto_increment << " " << attr.isPrimaryKey << " ";
        aWriter << attr.nullable << " " << attr.hasDefault << " " << attr.length << " " << attr.DefaultValue << " ";
        aWriter << ", ";
    }
    aWriter << " " << total_data_num;
    return StatusResult{};
}
StatusResult Schema::decode(std::istream &aReader) {
    aReader >> name;
    int count;
    attributes.clear();
    aReader >> count;
    for(int i = 0; i< count; i++){
        Attribute tmp;
        aReader >> tmp.name;
        char typechar;
        aReader >> typechar;
        tmp.type = Char2Datatype[typechar];
        aReader >> tmp.auto_increment >> tmp.isPrimaryKey >> tmp.nullable >> tmp.hasDefault >> tmp.length >> tmp.DefaultValue;
        char end;
        aReader >> end;
        attributes.push_back(tmp);
    }
    aReader >> total_data_num;
    return StatusResult{};
}
Schema&   Schema::TableView(std::ostream &anOutput) {
    anOutput << "+-----------+--------------+------+-----+---------+-----------------------------+" << "\n";
    anOutput << "| Field     | Type         | Null | Key | Default | Extra                       |" << "\n";
    anOutput << "+-----------+--------------+------+-----+---------+-----------------------------+" << "\n";
    for(auto attr:attributes){
        anOutput << "| " << std::setw(10) << std::left << attr.name;
        anOutput << "| " << std::setw(13) << attr.TypeToString();
        anOutput << "| "  << std::setw(5) << (attr.nullable? "YES":"NO");
        anOutput << "| " << std::setw(4) << (attr.isPrimaryKey? "YES":"");
        anOutput << "| " << std::setw(8) << (attr.hasDefault? attr.DefaultValue:"NULL");
        std::string extrainfo = "";
        extrainfo += (attr.auto_increment? "auto_increment ": "");
        extrainfo += (attr.isPrimaryKey? "primary key": "");
        anOutput << "| " << std::setw(28) << extrainfo;
        anOutput << "|\n";
    }
    anOutput << "+-----------+--------------+------+-----+---------+-----------------------------+" << "\n";
    anOutput << attributes.size() << " rows in set" << std::endl;
    return *this;
}
}
