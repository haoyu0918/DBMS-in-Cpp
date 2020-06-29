//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include "AppProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <vector>
namespace ECE141 {

    static std::unordered_map<Keywords, std::string> Help_List = {{Keywords::select_kw, "--selects columns from a database"},
                                                                  {Keywords::create_kw, "--creates a database"},
                                                                  {Keywords::help_kw, "--obtains instructions"},
                                                                  {Keywords::version_kw, "--gets the information of current version"},
                                                                  {Keywords::quit_kw, "quits from the software"},
                                                                  {Keywords::drop_kw, "drops the given database"},
                                                                  {Keywords::use_kw, "uses the given database"},
                                                                  {Keywords::describe_kw, "gets the schema of the given database"},
                                                                  {Keywords::show_kw, "shows the list of all databases available"}
                                                                  };
  
  class VersionStatement : public Statement {
  public:
    VersionStatement() :  Statement(Keywords::version_kw) {}
  };
  //---------help statement
  class HelpStatement:public Statement{
  private:
      std::vector<Keywords> help_items;
  public:
      HelpStatement():Statement(Keywords::help_kw){}
      StatusResult parse(Tokenizer& aTokenizer){
          while(aTokenizer.more() && aTokenizer.current().data != ";") {
              Token& thetoken = aTokenizer.current();
              aTokenizer.next();
              help_items.push_back(thetoken.keyword);
          }
          return StatusResult();
      }
      StatusResult run(std::ostream& anOutput) const{
          //std::clog << "called here" << std::endl;
          if(help_items.size() > 1) return StatusResult(Errors::unknownCommand);
          if(help_items.empty()) anOutput << "help \n-- the available list of commands shown below:\n-- help - shows this list of commands\n-- version -- shows the current version of this application\n-- quit  -- terminates the execution of this DB application \n-- create database <name> -- creates a new database  \n-- drop database <name> -- drops the given database\n-- use database <name>  -- uses the given database \n-- describe database <name>  -- describes the given database\n-- show databases   -- shows the list of databases available" << std::endl;
          else if(Help_List.find(help_items.at(0)) != Help_List.end()) anOutput<< Help_List[help_items[0]] << std::endl;
          else return StatusResult(Errors::unknownCommand);
          return StatusResult();
      }
  };

  //--------quit statement
  class QuitStatement:public Statement{
  public:
      QuitStatement():Statement(Keywords::quit_kw){}
  };
  //.....................................

  AppCmdProcessor::AppCmdProcessor(CommandProcessor *aNext) : CommandProcessor(aNext) {
  }
  
  AppCmdProcessor::~AppCmdProcessor() {
  }
  
  // USE: -----------------------------------------------------
  StatusResult AppCmdProcessor::Run_Version() {
      std::cout << "ECE141b-1" << std::endl;
      return StatusResult();
  }
  StatusResult AppCmdProcessor::Run_Quit() {
      return StatusResult(Errors::userTerminated);
  }
  StatusResult AppCmdProcessor::Run_Help(const ECE141::Statement &aStatement) {
      //std::clog<< "help here" << std::endl;
      return aStatement.run(std::cout);
  }
  StatusResult AppCmdProcessor::interpret(const Statement &aStatement) {
    //STUDENT: write code related to given statement
    switch(aStatement.getType()){
        case Keywords::version_kw: return Run_Version();
        case Keywords::quit_kw: return Run_Quit();
        case Keywords::help_kw: return Run_Help(aStatement);
    }
  }
  
  // USE: factory to create statement based on given tokens...
  Statement* AppCmdProcessor::getStatement(Tokenizer &aTokenizer) {
    //STUDENT: Analyze tokens in tokenizer, see if they match one of the
    //         statements you are supposed to handle. If so, create a
    //         statement object of that type on heap and return it.
    //         If you recognize the tokens, consider using a factory
    //         to construct a custom statement object subclass.
    std::vector<Keywords> theTerms{Keywords::quit_kw, Keywords::help_kw, Keywords::version_kw};
    Token& theToken = aTokenizer.current();
    if(std::find(theTerms.begin(), theTerms.end(), theToken.keyword) != theTerms.end()) {
        aTokenizer.next();
         // we cheat here, should return a subclass of statement...
         switch(theToken.keyword){
             case Keywords::version_kw: return new VersionStatement();
             case Keywords::quit_kw: return new QuitStatement();
             case Keywords::help_kw:
                 HelpStatement* ahelpstatement = new HelpStatement();
                 ahelpstatement->parse(aTokenizer);
                 return ahelpstatement;
         }
    }
    return nullptr;
  }
  
}
