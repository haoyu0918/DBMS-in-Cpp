//
//  main.cpp
//  Database2
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>

#include "AppProcessor.hpp"
#include "Tokenizer.hpp"
#include "Errors.hpp"
#include "Storage.hpp"
#include "DatabaseProcessor.hpp"
#include "FolderReader.hpp"
#include "Database.hpp"
#include "SQLProcessor.hpp"
//#include "Value.hpp"
#include "Row.hpp"
// USE: ---------------------------------------------
using namespace std;
static std::map<int, std::string> theErrorMessages = {
  {ECE141::illegalIdentifier, "Illegal identifier"},
  {ECE141::unknownIdentifier, "Unknown identifier"},
  {ECE141::databaseExists, "Database exists"},
  {ECE141::tableExists, "Table Exists"},
  {ECE141::syntaxError, "Syntax Error"},
  {ECE141::unknownCommand, "Unknown command"},
  {ECE141::unknownDatabase,"Unknown database"},
  {ECE141::unknownTable,   "Unknown table"},
  {ECE141::primaryKeyError, "Primary Key Error"},
  {ECE141::unknownError,   "Unknown error"},
  {ECE141::noDatabaseSpecified, "No Database specified"},
  {ECE141::invalidArguments, "Invalid Arguments"},
  {ECE141::unknownAttribute, "Unknown Attribute"}
};

void showError(ECE141::StatusResult &aResult) {
  std::string theMessage="Unknown Error";
  if(theErrorMessages.count(aResult.code)) {
    theMessage=theErrorMessages[aResult.code];
  }
  std::cout << "Error (" << aResult.code << ") " << theMessage << "\n";
}

//build a tokenizer, tokenize input, ask processors to handle...
ECE141::StatusResult handleInput(std::istream &aStream, ECE141::CommandProcessor &aProcessor) {
  ECE141::Tokenizer theTokenizer(aStream);
  
  //tokenize the input from aStream...
  ECE141::StatusResult theResult=theTokenizer.tokenize();
  while(theResult && theTokenizer.more()) {
    if(";"==theTokenizer.current().data) {
      theTokenizer.next();  //skip the ";"...
    }
    else theResult=aProcessor.processInput(theTokenizer);
  }
  return theResult;
}

//----------------------------------------------
void testIO(){
    ECE141::ValueType v1, v2, v3, v4;
    v1.data = uint32_t (11);
    v2.data = float(12.43);
    v3.data = std::string("lihoaoyefuefu");
    v4.data = true;
    stringstream ss(ios::in | ios::out | ios::binary);
    v1.encode(ss);
    v2.encode(ss);
    v3.encode(ss);
    v4.encode(ss);
    ECE141::ValueType vv;
    vv.decode(ss);
    vv.decode(ss);
    vv.decode(ss);
    vv.decode(ss);


}
void testRowIO(){
    ECE141::Row aRow;
    aRow.rowdata["status"] = ECE141::ValueType{false};
    aRow.rowdata["title"] = ECE141::ValueType{std::string("haoyu")};
    aRow.rowdata["id"] = ECE141::ValueType{uint32_t(90)};
    stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    aRow.encode(ss);
    std::clog << ss.str() << std::endl;
    stringstream newss(std::ios::in | std::ios::out | std::ios::binary);
    newss.str(ss.str());
    ECE141::Row newRow;
    newRow.decode(newss);
    newRow.ShowData(std::clog);
    return ;
}
void tryinput(){
    std::string thestr = "insert into users (id, name) values (9, 'foo')";
    std::stringstream ss(thestr);
    ECE141::Tokenizer thetokenizer(ss);
    thetokenizer.tokenize();
    return;
}
int main(int argc, const char * argv[]) {
    /*ECE141::FolderReader areader(ECE141::StorageInfo::getDefaultStoragePath());
    ECE141::testFolderListener alistener;
    areader.each(alistener, ".txt");*/
    //testIO();
    /*ECE141::Database thedb("test_db2", ECE141::OpenExistingStorage{});
    ECE141::StorageBlock aBlock(ECE141::BlockType::free_block);
    strcpy(aBlock.data, "userafeffhuehfhehfe");
    thedb.getStorage().writeBlock(aBlock, 2);
    std::cout << thedb.getStorage().getTotalBlockCount() << std::endl;
    return 0;*/

    /*ECE141::Database thedb("test_db2", ECE141::OpenExistingStorage{});
    std::cout << thedb.getStorage().getTotalBlockCount() << std::endl;
    std::cout << thedb.getStorage().findFreeBlockNum().value << std::endl;
    ECE141::StorageBlock aBlock(ECE141::BlockType::free_block);
    thedb.getStorage().writeBlock(aBlock, 1);
    std::cout << thedb.getStorage().findFreeBlockNum().value << std::endl;
    return 0;*/
    ECE141::SQLProcessor theSQLProcessor;
   ECE141::DatabaseProcessor theDBprocessor(&theSQLProcessor);
  ECE141::AppCmdProcessor   theProcessor(&theDBprocessor);  //add your db processor here too!
  ECE141::StatusResult      theResult{};
  if(argc>1) {
    std::ifstream theStream(argv[1]);
    return handleInput(theStream, theProcessor);
  }
  else{
    std::string theUserInput;
    bool running=true;
    do {
      std::cout << "\n> ";
      if(std::getline(std::cin, theUserInput)) {
        if(theUserInput.length()) {
          std::stringstream theStream(theUserInput);
          theResult=handleInput(theStream, theProcessor);
          if(!theResult) showError(theResult);
        }
        if(ECE141::userTerminated==theResult.code)
          running=false;
      }
    }
    while (running);
  }
  return 0;
}

