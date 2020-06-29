//
//  Storage.cpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Storage.hpp"
#include "FolderReader.hpp"
#include <fstream>
#include <cstdlib>

namespace ECE141 {

  // USE: Our main class for managing storage...
  const char* StorageInfo::getDefaultStoragePath() {
    //STUDENT -- MAKE SURE TO SET AN ENVIRONMENT VAR for DB_PATH! 
    //           This lets us change the storage location during autograder testing

    //WINDOWS USERS:  Use forward slash (/) not backslash (\) to separate paths.
    //                (Windows convert forward slashes for you)
    
    const char* thePath = std::getenv("DB_PATH");
    //const char* thePath = "/Users/lihaoyu/CLionProjects/ece_141b_sp2020/ece_141_dbfile";
    return thePath;
  }

  //----------------------------------------------------------

  //path to the folder where you want to store your DB's...
  std::string getDatabasePath(const std::string &aDBName) {
    std::string thePath;
    //build a full path (in default storage location) to a given db file..
    std::string dir_path = StorageInfo::getDefaultStoragePath();
    thePath = dir_path + "/" + aDBName + ".db";
    return thePath;
  }

  // USE: ctor ---------------------------------------
  Storage::Storage(const std::string aName, CreateNewStorage) : name(aName) {
      filepath = getDatabasePath(name);
    //try to create a new db file in known storage location.
    //throw error if it fails...
    FolderReader afile_reader;
    if(!afile_reader.exists(filepath)) {
        std::ofstream outfile(filepath, std::ios::binary); // use ofstream to create the file
        outfile.close();
        //stream.open(thePath, std::ios_base::in|std::ios_base::out|std::ios::binary);
    }
    else throw "The file exists";
  }

  // USE: ctor ---------------------------------------
  Storage::Storage(const std::string aName, OpenExistingStorage) : name(aName) {
    filepath = getDatabasePath(aName);
    //try to OPEN a db file a given storage location
    //if it fails, throw an error
    //stream.open(thePath, std::ios_base::in | std::ios_base::out | std::ios::binary);
    //if(!isReady()) {std::cout << "error" << std::endl;
    //throw "The file doesn't exists";}
  }

  // USE: dtor ---------------------------------------
  Storage::~Storage() {
      if(isReady()) stream.close();
  }


  // USE: validate we're open and ready ---------------------------------------
  bool Storage::isReady() const {
    return stream.is_open();
  }

  // USE: count blocks in file ---------------------------------------
  uint32_t Storage::getTotalBlockCount() {
    //how can we compute the total number of blocks in storage?
    stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
    uint32_t theCount=0;
    if(isReady()){
        stream.seekg(0, std::ios::end);
        size_t end_pos = stream.tellg();
        size_t block_size = sizeof(char) + sizeof(uint32_t) + kPayloadSize;
        if(end_pos % block_size != 0) throw "errors";
        else theCount = end_pos / block_size;
        stream.close();
        stream.clear();
    }
    return theCount;
  }

  // Call this to locate a free block in this storage file.
  // If you can't find a free block, then append a new block and return its blocknumber
  StatusResult Storage::findFreeBlockNum() {
      uint32_t total = getTotalBlockCount();
      //std::clog << "total num is" << total << std::endl;
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      uint32_t res = total;
      for(uint32_t i = 0; i<total; i++){
          char cur_type;
          int thenum = i * aBLockSize + sizeof(uint32_t);
          stream.seekp(thenum, std::ios::beg);
          stream.read(&cur_type, sizeof(char));
          if(cur_type == 'F') {
              res = i;
              break;
          }
      }
      stream.close();
      stream.clear();
    return StatusResult{noError, res}; //return blocknumber in the 'value' field...
  }

  // USE: for use with "storable API" [NOT REQUIRED -- but very useful]

  StatusResult Storage::savetoBlock(Storable &aStorable) {
    //High-level IO: save a storable object (like a table row)...
    std::stringstream anOutstream(std::ios::in | std::ios::out | std::ios::binary);
    aStorable.encode(anOutstream);
    uint32_t index = findFreeBlockNum().value;
    StorageBlock aBlock(aStorable.getType());
    //-----------------------------------------
    uint32_t  assignedid = aStorable.getHeaderId();
    aBlock.header.id = assignedid;
    //-----------------------------------------
    //std::clog << index << std::endl;
    //std::cerr << anOutstream.str()<< std::endl;
    anOutstream.read((char*)&aBlock.data, kPayloadSize);
    /*std::string thestr = anOutstream.str();
    const char* theBuff = thestr.c_str();
    memcpy(aBlock.data, theBuff, kPayloadSize);*/
    writeBlock(aBlock, index);
    return StatusResult{noError, index};
  }

// USE: for use with "storable API" [NOT REQUIRED -- but very useful]

  StatusResult Storage::loadfromBlock(Storable &aStorable, uint32_t blockindex) {
    //high-level IO: load a storable object (like a table row)
    StorageBlock aBlock;
    readBlock(aBlock, blockindex);
    if(aBlock.header.type != static_cast<char>(aStorable.getType())) return StatusResult{Errors::readError};
    std::stringstream  anInstream(std::ios::in | std::ios::out | std::ios::binary);
    anInstream.write((char*)&aBlock.data, kPayloadSize);
    aStorable.decode(anInstream);
    aStorable.setHeaderId(aBlock.header.id);
    return StatusResult{noError};
  }

  // USE: write data a given block (after seek)
  StatusResult Storage::writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
    //STUDENT: Implement this; this is your low-level block IO...
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      if(isReady()){
        size_t pos = aBLockSize * aBlockNumber;
        stream.seekp(pos, std::ios::beg);
        stream.write((char*)&aBlock.header.id, sizeof(uint32_t));
        //std::clog << "the writing block is" << aBlock.header.type << std::endl;
        stream.write(&aBlock.header.type, sizeof(char));
        stream.write(aBlock.data, kPayloadSize);
        //std::clog << "done writing" << std::endl;
        stream.close();
        stream.clear();
        return StatusResult{};
    }
    else{
        throw "Error";
        return StatusResult{Errors::unknownError};
    }
  }

  // USE: read data from a given block (after seek)
  StatusResult Storage::readBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
    //STUDENT: Implement this; this is your low-level block IO...
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      if(isReady()) {
        size_t pos = aBLockSize * aBlockNumber;
        stream.seekg(pos, std::ios::beg);
        stream.read((char*)(&aBlock.header.id), sizeof(uint32_t));
        stream.read(&aBlock.header.type, sizeof(char));
        stream.read(aBlock.data, kPayloadSize);
        stream.close();
        stream.clear();
        return StatusResult{};
    }
    else{
        throw "Error";
        return StatusResult{Errors::unknownError};
    }
  }
  BlockHeader    Storage::GetTheBlockHeader(uint32_t theIndex) {
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      BlockHeader aHeader;
      stream.seekg(theIndex * aBLockSize, std::ios::beg);
      stream.read((char*)(&aHeader.id), sizeof(uint32_t));
      stream.read(&(aHeader.type), sizeof(char));
      stream.close();
      return aHeader;
  }

  StatusResult Storage::overwriteBlock(Storable &aStorable, uint32_t index) {
      std::stringstream anOutstream(std::ios::in | std::ios::out | std::ios::binary);
      aStorable.encode(anOutstream);
      StorageBlock aBlock(aStorable.getType());
      uint32_t  assignedid = aStorable.getHeaderId();
      aBlock.header.id = assignedid;
      anOutstream.read((char*)&aBlock.data, kPayloadSize);
      writeBlock(aBlock, index);
      return StatusResult{noError, index};

  }
  /*StatusResult Storage::savetoFile(Storable &aStorable)  {
      uint32_t index = findFreeBlockNum().value;
      StorageBlock aBlock(aStorable.getType());
      //-----------------------------------------
      uint32_t  assignedid = aStorable.getHeaderId();
      aBlock.header.id = assignedid;
      //-----------------------------------------
      //std::clog << index << std::endl;
      writeBlock(aBlock, index);
      stream.clear();
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      stream.seekp(index * aBLockSize + sizeof(uint32_t) + sizeof(char), std::ios::beg);
      aStorable.encode(stream);
      stream.close();
      return StatusResult{noError, index};
  }*/
  /*StatusResult Storage::loadfromFile(Storable &aStorable, uint32_t BlockIndex) {
      stream.clear();
      stream.open(filepath, std::ios::in | std::ios::out | std::ios::binary);
      uint32_t theID;
      stream.seekg(BlockIndex * aBLockSize, std::ios::beg);
      stream.read((char*)&theID, sizeof(uint32_t));
      char theType;
      stream.read(&theType, sizeof(char));
      if(theType != static_cast<char>(aStorable.getType())) return StatusResult{readError};
      aStorable.decode(stream);
      aStorable.setHeaderId(theID);
      stream.close();
      return StatusResult{};
  }*/

   bool Storage::each(BlockVisitor& aVisitor) {
      uint32_t  total = getTotalBlockCount();
      for(uint32_t index = 0; index < total ; index ++){
          StorageBlock aBlock;
          readBlock(aBlock, index);
          aVisitor.visit(aBlock);
      }
      return true;
  }

  StatusResult Storage::freeABlock(uint32_t aBlockNum) {
       StorageBlock FreeBlock(BlockType::free_block);
       writeBlock(FreeBlock, aBlockNum);
       return StatusResult{};
   }

}


