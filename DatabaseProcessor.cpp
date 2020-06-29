//
// Created by 李浩宇 on 4/12/20.
//
#include "Statement.hpp"
#include <iostream>
#include "DatabaseProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>
#include "Storage.hpp"
#include "FolderReader.hpp"
#include <fstream>
#include "Database.hpp"
#include "StorageBlock.hpp"
namespace ECE141{
    class CreateDBStatement:public Statement{
    private:
        std::vector<Token> token_list;
    public:
        CreateDBStatement(Tokenizer& aTokenizer):Statement(Keywords::create_kw){
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                token_list.push_back(aTokenizer.current());
                aTokenizer.next();
            }

        }
        StatusResult parse(Tokenizer& aTokenizer){
            return StatusResult();
    }
    StatusResult checkinput() const{
            if(token_list.size() != 2) return StatusResult(Errors::unknownCommand);
            if(token_list[0].keyword != Keywords::database_kw) return StatusResult(Errors::unknownCommand);
            if(token_list[1].keyword != Keywords::unknown_kw) return StatusResult(Errors::illegalIdentifier);
            if(token_list[1].data == "test") return StatusResult(Errors::illegalIdentifier);
            std::string thedbname = token_list[1].data;
            std::string thePath = getDatabasePath(thedbname);
            FolderReader aFileReader;
            if(aFileReader.exists(thePath)) return StatusResult(Errors::databaseExists);
            return StatusResult();
        }
    StatusResult run(std::ostream& anOutput) const{
            StatusResult check_res = checkinput();
            if(check_res.code != Errors::noError) return check_res;
            std::string database_name = token_list[1].data;
            Database theNewDB(database_name, CreateNewStorage{});
            std::string thePath = getDatabasePath(database_name);
            FolderReader aFileReafer;
            if(aFileReafer.exists(thePath)) std::cout << "created" << " " << database_name << " (OK)" << std::endl;
            else return StatusResult{Errors::unknownError};
            StorageBlock aBlock(BlockType::meta_block);
            theNewDB.getStorage().writeBlock(aBlock, 0);
            return StatusResult{};
        }

    };
    class DropDBStatement:public Statement{
    private:
        std::vector<Token> token_list;
        DatabaseProcessor& theDBProcessor;
    public:
        DropDBStatement(Tokenizer& aTokenizer, DatabaseProcessor& aProcessor):Statement(Keywords::drop_kw), theDBProcessor(aProcessor){
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                token_list.push_back(aTokenizer.current());
                aTokenizer.next();
            }
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list.size() != 2 || token_list[0].keyword!= Keywords::database_kw) return StatusResult(Errors::unknownCommand);
            std::string db_name = token_list[1].data;
            std::string DBPath = getDatabasePath(db_name);
            if(!FolderReader().exists(DBPath)) return StatusResult{Errors::unknownDatabase};
            Database* activeDB = theDBProcessor.getActiveDatabase();
            if(activeDB && activeDB->getName() == db_name) return activeDB->dropDatabase(std::cout);
            else{
                Database theDB(db_name, OpenExistingStorage{});
                return theDB.dropDatabase(std::cout);
            }
        }
    };
    // ---------USE Statement-----------------------------

    class DescribeDBStatement:public Statement{
    private:
        DatabaseProcessor& theDBProcessor;
        std::vector<Token> token_list;
    public:
        DescribeDBStatement(Tokenizer& aTokenizer, DatabaseProcessor& aProcessor): Statement(Keywords::describe_kw), theDBProcessor(aProcessor){
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                token_list.push_back(aTokenizer.current());
                aTokenizer.next();
            }
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list.size() != 2 || token_list[0].keyword != Keywords::database_kw) return StatusResult(Errors::unknownCommand);
            std::string db_name = token_list[1].data;
            std::string db_path = getDatabasePath(db_name);
            FolderReader aFileReader;
            if(!aFileReader.exists(db_path)) return StatusResult(Errors::unknownDatabase);
            else{
                Database* activeDB = theDBProcessor.getActiveDatabase(); // active DB is the one?
                if(activeDB && activeDB->getName() == db_name) return activeDB->describeDatabase(std::cout);
                else{
                    Database theDB(db_name, OpenExistingStorage{});
                    return theDB.describeDatabase(std::cout);
                }
            }
        }
    };
    class ShowDBStatement:public Statement{
    private:
        std::vector<Token> token_list;
    public:
        ShowDBStatement(Tokenizer& aTokenizer): Statement(Keywords::show_kw){
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                token_list.push_back(aTokenizer.current());
                aTokenizer.next();
            }
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list.size() != 1 || token_list[0].keyword!= Keywords::databases_kw) return StatusResult(Errors::unknownCommand);
            std::filesystem::path dir_path (StorageInfo::getDefaultStoragePath());
            std::filesystem::directory_iterator db_it(dir_path);
            for(auto temp:db_it){
                std::string db_name(temp.path().filename());
                size_t pos = db_name.find(".");
                anOutput << db_name.substr(0, pos) << std::endl;
            }
            return StatusResult();
        }
    };

    //Use-------------------------------------------------------------------------------
    DatabaseProcessor::DatabaseProcessor(CommandProcessor* aNext): CommandProcessor(aNext){}
    DatabaseProcessor::~DatabaseProcessor(){
        //std::cout << "Database Processsor destructor here" << std::endl;
    }

    StatusResult DatabaseProcessor::interpret(const Statement &aStatement) {
        //STUDENT: write code related to given statement
        return aStatement.run(std::cout);
    }

    Statement* DatabaseProcessor::getStatement(Tokenizer &aTokenizer) {
        //STUDENT: Analyze tokens in tokenizer, see if they match one of the
        //         statements you are supposed to handle. If so, create a
        //         statement object of that type on heap and return it.
        //         If you recognize the tokens, consider using a factory
        //         to construct a custom statement object subclass.
        std::vector<Keywords> theTerms{Keywords::create_kw, Keywords::drop_kw, Keywords::describe_kw, Keywords::show_kw};
        Token& theToken = aTokenizer.current();
        if(aTokenizer.remaining() == 1) return nullptr;
        if(std::find(theTerms.begin(), theTerms.end(), theToken.keyword) != theTerms.end() && aTokenizer.more()) {
            if(aTokenizer.peek().keyword != Keywords::database_kw && aTokenizer.peek().keyword != Keywords::databases_kw) return nullptr;
            aTokenizer.next();
            // we cheat here, should return a subclass of statement...
            switch(theToken.keyword){
                case Keywords::create_kw: return new CreateDBStatement(aTokenizer);
                case Keywords::drop_kw: return new DropDBStatement(aTokenizer, *this);
                case Keywords::describe_kw: return new DescribeDBStatement(aTokenizer, *this);
                case Keywords::show_kw: return new ShowDBStatement(aTokenizer);
            }
        }
        return nullptr;
    }

}