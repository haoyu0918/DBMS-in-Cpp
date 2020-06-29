//
// Created by 李浩宇 on 5/23/20.
//

#include "Index.hpp"
#include "Attribute.hpp"
#include <iostream>
namespace ECE141{
    Index& Index::addKeyValue(const ValueType &aKey, uint32_t aValue) {
        list[aKey] = aValue;
        setChanged();
        return *this;
    }

    Index& Index::removeKeyValue(const ValueType &aKey) {
        list.erase(aKey);
        setChanged();
        return *this;
    }

    bool Index::contains(const ValueType &aKey) {
        if(list.find(aKey) != list.end()) return true;
        else return false;
    }

    uint32_t Index::getValue(const ValueType &aKey) {
        if(list.find(aKey) != list.end())
            return list[aKey];
        else return -1; // -1 means could not find the key in list
    }

    bool Index::each(BlockVisitor &aVisitor) {
        // iterate through the block of the table
        for(auto& thepair: list){
            StorageBlock aBlock;
            thestorage.readBlock(aBlock, thepair.second);
            aVisitor.visit(aBlock);
        }
        return true;
    }
    bool Index::canIndexBy(const std::string &aField) {
        return field == aField;
    }

    BlockType Index::getType() const {
        return BlockType::index_block;
    }

    /*std::string   field; //what field are we indexing?
    DataType    type;
    uint32_t      schemaId;
    bool          changed;
    uint32_t      blockNum;  //storage block# of index...

    ValueMap      list;
     */
    StatusResult   Index::encode(std::ostream &aWriter) {
        aWriter << field <<  ' ' << ',';
        aWriter << static_cast<char>(type) << ' ' << ',';
        aWriter << schemaId << ' ' << ',';
        aWriter << list.size() << ' ' << ',';
        for(auto thepair: list){
            // the fromation is "key : value,
            ValueType aValue = thepair.first;
            aValue.encode(aWriter);
            aWriter << ' ' << ':' << thepair.second << ',';
        }
        return StatusResult{};
    }

    StatusResult   Index::decode(std::istream &aReader) {
        char c;
        aReader >> field >> c;
        char typechar;
        aReader >> typechar >> c;
        type = Char2Datatype[typechar];
        aReader >> schemaId >> c;
        int count = 0;
        aReader >> count >> c;
        for(int i = 0; i<count; i++){
            ValueType tmp_key;
            uint32_t  thevalue;
            tmp_key.decode(aReader);
            char c1, c2;
            aReader >> c1 >> thevalue >> c2;
            list[tmp_key] = thevalue;
        }
        return StatusResult{};
    }
    Index& Index::showData(std::ostream &anOutput) {
        anOutput << field << ',' << schemaId <<',';
        for(auto& thepair: list){
            ValueType aValue = thepair.first;
            std::visit(Visitor{anOutput}, aValue.data);
            anOutput << ':' << thepair.second << ",  ";
        }
        return *this;
    }
    Index& Index::clearall(){
        isChanged();
        list.clear();
        return *this;
    }
}