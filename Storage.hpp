//
//  Storage.hpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <variant>
#include "Errors.hpp"
#include "StorageBlock.hpp"
#include <filesystem>
#include "Value.hpp"

namespace ECE141 {
    
  //first, some utility classes...

    class BlockVisitor{
    public:
        // may be overwrite in other class
        virtual StatusResult visit(StorageBlock& aBlock){
            return StatusResult{};
        }
    };
    //interface of BlockIterator

    struct BlockIterator {
        virtual bool each(BlockVisitor &aVisitor)=0;
        virtual bool canIndexBy(const std::string &aField) {return false;} //override this
    };

  class StorageInfo {
  public:
    static const char* getDefaultStoragePath();
  };
  
  struct CreateNewStorage {};
  struct OpenExistingStorage {};

  /*struct Storable {
    virtual StatusResult  encode(std::ostream &aWriter)=0;
    virtual StatusResult  decode(std::istream &aReader)=0;
    virtual BlockType     getType() const=0; //what kind of block is this?
    //virtual StatusResult  setType(BlockType aType)  = 0;
  };*/
  std::string getDatabasePath(const std::string &aDBName);
  // USE: Our storage manager class...
  class Storage :public BlockIterator{
  public:
        
    Storage(const std::string aName, CreateNewStorage);
    Storage(const std::string aName, OpenExistingStorage);
    ~Storage();        
    uint32_t        getTotalBlockCount();

        //high-level IO (you're not required to use this, but it may help)...    
    StatusResult    savetoBlock(Storable &aStorable); //using a stream api
    StatusResult    loadfromBlock(Storable &aStorable, uint32_t blockindex); //using a stream api
    StatusResult    overwriteBlock(Storable& aStorable, uint32_t blockindex); // overwrite a block
    //StatusResult    savetoFile(Storable& aStorable);
    //StatusResult    loadfromFile(Storable& aStorable, uint32_t BlockIndex);
        //low-level IO...    
    StatusResult    readBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    StatusResult    writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    StatusResult    freeABlock(uint32_t aBlockNum);
    Storage& free(){
        stream.close();
        return *this;
    }
    StatusResult    findFreeBlockNum();
    BlockHeader       GetTheBlockHeader(uint32_t theIndex);
    bool each(BlockVisitor& aVisitor) override ;
  protected:
    bool            isReady() const;

    std::string     name;
    std::string     filepath;
    std::fstream    stream;    
  };


}

#endif /* Storage_hpp */
