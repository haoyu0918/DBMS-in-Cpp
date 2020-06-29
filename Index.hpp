//
//  Index.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/17/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Index_h
#define Index_h

#include "Storage.hpp"
#include "keywords.hpp"

namespace ECE141 {
  
  using IntOpt = std::optional<uint32_t>;
  
  struct LessKey {
    bool operator()(const ValueType& anLHS, const ValueType& aRHS) const {
      return anLHS.data < aRHS.data;
    }
  };
  using ValueMap = std::map<ValueType, uint32_t, LessKey>;
  class Index : public BlockIterator, public Storable {
  public:

    Index(Storage& aStorage, const std::string &aField = "", uint32_t aHashId = 0, DataType aType = DataType::no_type)
      : field(aField), type(aType), schemaId(aHashId), blockNum(0), thestorage(aStorage) {
        changed=false;
      }

    
    virtual ~Index() {}
    
    ValueMap&           getList() {return list;}
    void                setChanged(bool aValue=true) {changed=aValue;}
    bool                isChanged() {return changed;}
    const std::string&  getFieldName() const {return field;}
    uint32_t            getBlockNum() const {return blockNum;}
    Index&              setBlockNum(uint32_t anIndex){
        blockNum = anIndex;
        return *this;
    }
    
      //manage keys/values in the index...
    Index& addKeyValue(const ValueType &aKey, uint32_t aValue) ;
    Index& removeKeyValue(const ValueType &aKey);
    Index& clearall();
    bool contains(const ValueType &aKey);    
    uint32_t getValue(const ValueType &aKey);
    
      //don't forget to support the storable interface IF you want it...
    StatusResult encode(std::ostream &aWriter) override;
    StatusResult decode(std::istream &aReader) override;
    BlockType     getType() const  override ;
    //void initBlock(StorageBlock &aBlock);

      //and the blockIterator interface...
    bool each(BlockVisitor &aVisitor) override;  
    bool canIndexBy(const std::string &aField) override;
    void setSchemaId(uint32_t aNum){
        schemaId = aNum;
    }
    uint32_t  getSchemaId(){
        return schemaId;
    }
    Index& showData(std::ostream& anOutput);
  protected:
    
    std::string   field; //what field are we indexing?
    DataType    type;
    uint32_t      schemaId;
    bool          changed;
    uint32_t      blockNum;  //storage block# of index...
    
    ValueMap      list;
    Storage&      thestorage;
  };

}
#endif /* Index_h */
