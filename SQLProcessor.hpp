//
//  SQLProcessor.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CommandProcessor.hpp"
#include "Tokenizer.hpp"
#include "Schema.hpp"
#include "FolderReader.hpp"
#include "Database.hpp"
#include <string>
#include <iostream>
#include <algorithm>
#include "Filters.hpp"
#include <math.h>
#include "Helpers.hpp"
class Statement;
class Database; //define this later...

namespace ECE141 {

    class SQLProcessor;
    class DeleteStatement: public Statement{
    private:
        SQLProcessor &theSQlProcessor;
        std::string table_name;
        std::vector<Token> token_list;
    public:
        DeleteStatement(SQLProcessor& aProcessor): theSQlProcessor(aProcessor){}
        virtual   StatusResult  parse(Tokenizer &aTokenizer);
        virtual   StatusResult  run(std::ostream &aStream) const;
    };

    class SQLProcessor : public CommandProcessor {
  public:
    
    SQLProcessor(CommandProcessor *aNext=nullptr);
    virtual ~SQLProcessor();
    
    virtual Statement*    getStatement(Tokenizer &aTokenizer);
    virtual StatusResult  interpret(const Statement &aStatement);
    
    StatusResult createTable(Schema &aSchema);

    StatusResult dropTable(const std::string &aName);
    StatusResult describeTable(const std::string &aName) const;
    StatusResult showTables() const;
    StatusResult insertData(const std::string& tablename, const std::vector<std::string>& attributes, const std::vector< std::vector<Token>>& records_list) const;
    StatusResult deleteData(const std::string& tablename);
      StatusResult setActiveDatabase(const std::string dbname){
          if(activeDB) delete activeDB;
          activeDB = new Database(dbname, OpenExistingStorage{});
          return StatusResult{};
      }
      Database* getActiveDatabase(){
          return activeDB;
      }
    
/*  do these in next assignment
    StatusResult insert();
    StatusResult update();
    StatusResult delete();
*/
    
  protected:

    //do you need other data members?
    Database* activeDB;
  };
    class UseDBStatement:public Statement{
    private:
        std::vector<Token> token_list;
        SQLProcessor &theSQLProcessor;
    public:
        UseDBStatement(Tokenizer& aTokenizer, SQLProcessor& aDbProcessor):Statement(Keywords::use_kw), theSQLProcessor(aDbProcessor){
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                token_list.push_back(aTokenizer.current());
                aTokenizer.next();
            }
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list.size() != 2 || token_list[0].keyword != Keywords::database_kw) return StatusResult(Errors::unknownCommand);
            std::string dbname = token_list[1].data;
            std::string db_path = getDatabasePath(dbname);
            FolderReader aFileReader;
            if(!aFileReader.exists(db_path)) return StatusResult(Errors::unknownDatabase);
            else {
                theSQLProcessor.setActiveDatabase(dbname);
                //std::clog << theSQLProcessor.getActiveDatabase()->getName() << std::endl;
                anOutput << "Using database " << dbname << " (OK) ";
                return StatusResult{};
            }
        }

    };

}
#endif /* SQLProcessor_hpp */
