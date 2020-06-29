//
//  Database.hpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include "Storage.hpp"
#include <iomanip>
#include <filesystem>
#include "Schema.hpp"
#include <unordered_map>
#include "Tokenizer.hpp"
#include <algorithm>
#include "Filters.hpp"
#include "Row.hpp"
#include "Index.hpp"
#include <utility>
#include "Timer.hpp"
#include "Helpers.hpp"

namespace ECE141 {
    struct TOC: public Storable{
        std::unordered_map<std::string, uint32_t> Schemas_Map;
        BlockType theType;
        bool isChanged;
        TOC():theType(BlockType::meta_block), isChanged(false){}
        StatusResult  encode(std::ostream &aWriter){
            aWriter << Schemas_Map.size() << " ";
            for(auto thePair: Schemas_Map){
                aWriter << thePair.first << " " << ":" << " " << thePair.second << " " << ", " << " ";
            }
            return StatusResult{};
        }
        StatusResult  decode(std::istream &aReader){
            int count = 0;
            aReader >> count;
            for(int i = 0; i<count; i++){
                std::string tablename;
                uint32_t index;
                char c1,c2;
                aReader >> tablename >> c1 >> index >> c2;
                Schemas_Map[tablename] = index;
            }
            return StatusResult{};
        }
        BlockType   getType() const {
            return BlockType::meta_block;
        }
    };

  class Database  {
  public:
    
    Database(const std::string aPath, CreateNewStorage);
    Database(const std::string aPath, OpenExistingStorage);
    ~Database();
    
    Storage&          getStorage() {return storage;}
    std::string&      getName() {return name;}
    StatusResult      describeDatabase(std::ostream& anOutput);
    StatusResult      dropDatabase(std::ostream& anOutput);
    StatusResult      createTable(Schema& aSchema, std::ostream& anOutput);
    StatusResult      showTables(std::ostream& anOutput);
    StatusResult      dropTable(const std::string& tableName, std::ostream& anOutput);
    StatusResult      describeTable(const std::string& tableName, std::ostream& anOutput);
    StatusResult      insertData(const std::string &tablename, const std::vector<std::string> &attributes, const std::vector<std::vector<Token> > &records_list);

    StatusResult      deleteData(const std::string& tablename);
    StatusResult      selectData(std::string tablename, const std::vector<std::string>& attributes_name, Filters& thefilters, const std::string& Ordered_by, const int LimitNum);
    StatusResult      ChangingFilters(Filters& aFilter, const Schema& aSchema);
    StatusResult      ShowSelectTableView(std::ostream &anOutput, RowCollection& aRowCollection, const std::string& Ordered_by, const int Limitnum, const std::vector<std::string>& attributes_name, Schema& aSchema, Timer& theTimer);
    RowCollection     GetRowsofAtable(Schema& aSchema);
    StatusResult      SelectOnlyByPrimaryKey(Schema& aSchema, RowCollection& aRowCollection, Filters& theFilter);
    StatusResult      selectJoinData(std::string tablename, const std::vector<std::string>& attributes_name, Filters& thefilters, const std::string& Ordered_by, const int LimitNum, std::vector<Join>& joins);


    StatusResult      updateData(std::string tablename, Filters& theSettings, Filters& theFilter);
    StatusResult      updateDataHelper(Schema& aSchema, Filters& theSetting, Filters& theFilter);
    StatusResult      updateOneRow(Row& aRow, Filters& aSetting);
    StatusResult      updateIndex(Index& theIndex, std::vector<ValueType>& thememo, ValueType& UpdatedValue);
    StatusResult      createTableIndex(Schema& aSchema);
    StatusResult      showIndexes(std::ostream& anOutput);
    RowCollection     selectJoinDataHelper(Schema& theSchema1, Schema& theSchema2, Join& theJoin, const std::vector<std::string>& table1Fields, const std::vector<std::string>& table2Fields);



    StatusResult      alterTable(std::string tablename, const std::vector<Attribute>& add_attr_list);
    uint32_t          hashString(const char* str) {
          const int Gm = 37;
          uint32_t h{0};
          unsigned char *p;
          for(p = (unsigned char*)str; *p != '\0'; p++){
              h = Gm * h + *p;
          }
          return h;
      }
      std::string getTableNameFromHash(uint32_t aNum){
        for(auto thePair: theToc.Schemas_Map){
            std::string temp = thePair.first;
            std::cerr << "The db is: " << temp << std::endl;
            if(hashString(temp.c_str()) == aNum) return temp;
        }
        return "Not found!";
        }
        StatusResult showoneBlock(uint32_t index, BlockType aType, std::ostream& anOutput);
        std::vector<std::string> ParseTableNameField(std::string astr){
          std::vector<std::string> res;
          size_t thepos = astr.find('.');
          res.push_back(astr.substr(0, thepos));
          res.push_back(astr.substr(thepos+1, std::string::npos));
          return res;
      }

  protected:
    std::string     name;
    Storage         storage;
    bool isChanged;
    TOC theToc;
    Index IndexofIndex;
  };

}

#endif /* Database_hpp */
