//
//  SQLProcessor.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "SQLProcessor.hpp"
#include <vector>
#include "Schema.hpp"
namespace ECE141 {
    //-------------Some Utility function for parsing-----------------------------------

    static std::vector<Keywords> Attrkeywords = {Keywords::integer_kw, Keywords::varchar_kw, Keywords::float_kw, Keywords::boolean_kw, Keywords::datetime_kw, Keywords::auto_increment_kw,
                                                           Keywords::primary_kw, Keywords::not_kw};
    StatusResult ParseAttributeHelper(std::vector<Token>& token_list, std::vector<Attribute>& res){
        size_t index = 0;
        Attribute CurAttribute;
        while(index < token_list.size()){
            // we get one attribute
            //std::clog << token_list[index].data << std::endl;
            if(token_list[index].data == ",") {
                res.push_back(CurAttribute);
                CurAttribute = Attribute();
            }
            else{
                if(token_list[index].type == TokenType::identifier) {
                    if(token_list[index].data == "default" || token_list[index].data == "Default" || token_list[index].data == "DEFAULT"){
                        //CurAttribute.setDefaultValue(token_list[++index].data);
                        index++;
                        std::string thedata  = "";
                        while(std::find(Attrkeywords.begin(), Attrkeywords.end(), token_list[index].keyword) == Attrkeywords.end() && token_list[index].data != ",") {
                            thedata += token_list[index].data;
                            index++;
                        }
                        CurAttribute.setDefaultValue(thedata);
                        index--;
                    }
                    else if(token_list[index].data == "Timestamp" ||token_list[index].data == "TIMESTAMP" || token_list[index].data == "timestamp"){
                        CurAttribute.setType(DataType::datetime_type);
                    }
                    else CurAttribute.setName(token_list[index].data);
                }
                else if(token_list[index].type == TokenType::keyword){
                    switch(token_list[index].keyword){
                        case Keywords::integer_kw : CurAttribute.setType(DataType::int_type); break;
                        case Keywords::varchar_kw: if(token_list[++index].type != TokenType::number) return StatusResult{Errors::syntaxError};
                        CurAttribute.setType(DataType::varchar_type); CurAttribute.setlength(std::stoi(token_list[index].data)); break;
                        case Keywords::float_kw: CurAttribute.setType(DataType::float_type); break;
                        case Keywords ::boolean_kw: CurAttribute.setType(DataType::bool_type); break;
                        case Keywords::datetime_kw: CurAttribute.setType(DataType::datetime_type); break;
                        case Keywords::auto_increment_kw: CurAttribute.setAuto_Incre(true); break;
                        case Keywords::primary_kw: if(token_list[++index].keyword != Keywords::key_kw) return StatusResult{Errors::syntaxError}; CurAttribute.setPrimaryKey(true); break;
                        case Keywords::not_kw: if(token_list[++index].keyword != Keywords::null_kw) return StatusResult{Errors::syntaxError}; CurAttribute.setNullable(false); break;
                        default: return StatusResult{Errors::syntaxError};
                    }
                }
            }
            index++;
        }
        return StatusResult{};
    }


    class CreateTableStatement: public Statement{
    private:
        SQLProcessor& theSQLProcessor;
        std::string table_name;
        std::vector<Token> token_list;
        std::vector<Attribute> Attribute_list;
    public:


        CreateTableStatement(SQLProcessor& aProcessor):theSQLProcessor(aProcessor){}
        StatusResult  AddPrimaryKey(Schema& aSchema) const{
            Attribute newAttribute;
            newAttribute.setName("ThePrimaryKey");
            newAttribute.setPrimaryKey(true);
            newAttribute.setAuto_Incre(true);
            newAttribute.setType(DataType::int_type);
            aSchema.addAttribute(newAttribute);
            return StatusResult{};
        }

        StatusResult parse(Tokenizer& aTokenizer){
            if(aTokenizer.peek().keyword != Keywords::table_kw) return StatusResult{syntaxError};
            aTokenizer.next();
            aTokenizer.next();
            table_name = aTokenizer.current().data;
            //std::clog << "The name is" << table_name << std::endl;
            aTokenizer.next();
            if(aTokenizer.current().data != "(")  return StatusResult{Errors::syntaxError};
            //std::clog << "The name is" << table_name << std::endl;
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                if(aTokenizer.current().data == "(" or aTokenizer.current().data == ")") aTokenizer.next();
                else{
                    token_list.push_back(aTokenizer.current());
                    aTokenizer.next();
                }
            }
            if(token_list.back().data != ","){
            Token endToken;
            endToken.data = ",";
            token_list.push_back(endToken);}
            if(!ParseAttributeHelper(token_list, Attribute_list)) {
                std::clog << "errors in parsing" << std::endl;
                return StatusResult{Errors::syntaxError};
            }
            return StatusResult{};
        }

        StatusResult run(std::ostream& anOutput) const{
            Schema theSchema(table_name);
            for(auto attr:Attribute_list){
                theSchema.addAttribute(attr);
            }
            //if(theSchema.getPrimaryKeyName() == std::nullopt) return StatusResult(primaryKeyError);
            if(theSchema.getPrimaryKeyName() == std::nullopt) AddPrimaryKey(theSchema);
            return theSQLProcessor.createTable(theSchema);
        }
    };
    class ShowTablesStatement:public Statement{
    protected:
        SQLProcessor& theSQLProcessor;
        std::vector<Token> token_list;
    public:
        ShowTablesStatement(SQLProcessor& aProcessor): theSQLProcessor(aProcessor){}
        StatusResult parse(Tokenizer& aTokenizer){
            if(aTokenizer.peek().keyword != Keywords::tables_kw) return StatusResult{Errors::syntaxError};
            aTokenizer.next();
            while(aTokenizer.more() && aTokenizer.current().data != ";")
            {token_list.push_back(aTokenizer.current());
            aTokenizer.next();}
            if(token_list.size() > 1) return StatusResult{unknownCommand};
            return StatusResult{};
        }
        StatusResult run(std::ostream& anOutput) const{
            return theSQLProcessor.showTables();
        }
    };

    class DropTableStatement:public Statement{
    protected:
        SQLProcessor& theSQLProcessor;
        std::vector<Token> token_list;
    public:
        DropTableStatement(SQLProcessor& aProcessor): theSQLProcessor(aProcessor){}
        StatusResult parse(Tokenizer& aTokenizer){
            if(aTokenizer.peek().keyword != Keywords::table_kw) return StatusResult{syntaxError};
            aTokenizer.next();
            while(aTokenizer.more() && aTokenizer.current().data != ";")
            {token_list.push_back(aTokenizer.current());
                aTokenizer.next();}
            if(token_list.size() != 2) return StatusResult{unknownCommand};
            return StatusResult{};
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list[1].type != TokenType::identifier) return StatusResult{illegalIdentifier};
            return theSQLProcessor.dropTable(token_list[1].data);
        }
    };


    class DescribeTableStatement:public Statement{
    protected:
        SQLProcessor& theSQLProcessor;
        std::vector<Token> token_list;
    public:
        DescribeTableStatement(SQLProcessor& aProcessor): theSQLProcessor(aProcessor){}
        StatusResult parse(Tokenizer& aTokenizer){
            if(aTokenizer.peek().type != TokenType::identifier) return StatusResult{syntaxError};
            while(aTokenizer.more() && aTokenizer.current().data != ";")
            {token_list.push_back(aTokenizer.current());
                aTokenizer.next();}
            if(token_list.size() != 2) return StatusResult{unknownCommand};
            return StatusResult{};
        }
        StatusResult run(std::ostream& anOutput) const{
            if(token_list[1].type != TokenType::identifier) return StatusResult{illegalIdentifier};
            return theSQLProcessor.describeTable(token_list[1].data);
        }
    };
    class InsertStatement:public Statement{
        protected:
        SQLProcessor& theSQLProcessor;
        std::vector<Token> token_list;
        std::vector<std::string> attributes_list;
        std::vector<std::vector<Token>> records_list;
        std::string table_name;
    public:
        StatusResult AttributesParseHelper(Tokenizer& aTokenizer){
            while(aTokenizer.more() && aTokenizer.current().data != ";" && aTokenizer.current().data != ")" ){
                Token cur_token = aTokenizer.current();
                if(cur_token.data != ",") attributes_list.push_back(cur_token.data);
                aTokenizer.next();
            }
            if(aTokenizer.current().data == ")") {
                aTokenizer.next();
                return StatusResult{};
            }
            else return StatusResult{syntaxError};
        }
        StatusResult GetARecord(Tokenizer& aTokenizer, std::vector<Token>& res){
            if(aTokenizer.current().data == "(") aTokenizer.next();
            while(aTokenizer.more() && aTokenizer.current().data != ")"){
                Token cur_token = aTokenizer.current();
                if(cur_token.data != ",") res.push_back(cur_token);
                aTokenizer.next();
            }
            if(!aTokenizer.more()) return StatusResult{syntaxError};
            else {
                aTokenizer.next();
                return StatusResult{};
            }
        }
        StatusResult RecordsParserHelper(Tokenizer& aTokenizer){
            if(aTokenizer.current().keyword != Keywords::values_kw) return StatusResult{syntaxError};
            aTokenizer.next();
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                std::vector<Token> tmp;
                if(!GetARecord(aTokenizer, tmp)) return StatusResult{syntaxError};
                /*std::clog << "The record is:";
                for(auto inner: tmp){
                    std::clog << (inner.data.size()? inner.data: "NULL") << ", ";
                }
                std::clog << std::endl;*/
                records_list.push_back(tmp);
                while (aTokenizer.more() && aTokenizer.current().data == ",") aTokenizer.next();
            }
            return StatusResult{};
        }
        InsertStatement(SQLProcessor& aProcessor): theSQLProcessor(aProcessor){};
        StatusResult  parse(Tokenizer &aTokenizer){
            if(aTokenizer.peek().keyword == Keywords::into_kw){
                aTokenizer.next(2);
                table_name = aTokenizer.current().data; // get the name of the table to be inserted
                if(aTokenizer.peek().data != "(") return StatusResult{syntaxError};
                aTokenizer.next(2);
                if(!AttributesParseHelper(aTokenizer)) return StatusResult{syntaxError};
                if(!RecordsParserHelper(aTokenizer)) return StatusResult{syntaxError};
                return StatusResult{};
            }
            else return StatusResult(syntaxError);
        }
        StatusResult  run(std::ostream &aStream) const{
            return theSQLProcessor.insertData(table_name, attributes_list, records_list);
        }

    };
    StatusResult DeleteStatement::parse(Tokenizer &aTokenizer) {
        if (aTokenizer.peek().keyword != Keywords::from_kw) return StatusResult{syntaxError};
        aTokenizer.next(2);
        while (aTokenizer.more() && aTokenizer.current().data != ";") {
            token_list.push_back(aTokenizer.current());
            aTokenizer.next();
        }
        if (token_list.size() != 1) return StatusResult{syntaxError};
        table_name = token_list[0].data;
        return StatusResult{};
    }
    StatusResult DeleteStatement::run(std::ostream &aStream) const {
        return theSQlProcessor.deleteData(table_name);
    }


    StatusResult  getFilter(Tokenizer& aTokenizer, Filters& theFilters){
        aTokenizer.next();
        while(aTokenizer.more() && aTokenizer.current().keyword != Keywords::order_kw && aTokenizer.current().data != ";"){
            Token curToken = aTokenizer.current();
            if(curToken.data == "LIMIT" || curToken.data == "limit" || curToken.data == "Limit") return StatusResult{};
            if(curToken.keyword == Keywords::and_kw || curToken.keyword == Keywords::or_kw || curToken.keyword == Keywords::not_kw){
                theFilters.addLogic(curToken.keyword);
                aTokenizer.next();
                continue;
            }
            if(aTokenizer.peek().type == TokenType::operators){
                Operand LHS, RHS;
                LHS.name = aTokenizer.current().data;
                aTokenizer.next();
                std::string theOpStr = "";
                while(aTokenizer.more() && aTokenizer.current().type == TokenType::operators) {
                    theOpStr += aTokenizer.current().data;
                    aTokenizer.next();
                }
                if(Char2Operators.find(theOpStr) == Char2Operators.end()) return StatusResult{syntaxError};
                Operators  theOp = Char2Operators[theOpStr];
                Token thenextToken = aTokenizer.current();
                RHS.type = thenextToken.type;
                RHS.value.data = thenextToken.data;
                Expression* newExpression = new Expression(LHS, theOp, RHS);
                theFilters.add(newExpression);
                aTokenizer.next();
            }
            else return StatusResult{syntaxError};
        }
        return StatusResult{};
    }

    class SelectStatement: public Statement{
    protected:
        SQLProcessor& theSQLProcessor;
        std::string Ordered_by;
        int LimitNum;
        std::vector<std::string> attributes_list;
        std::string table_name;
        Filters     theFilters;
        std::vector<Join> joins;
    public:
        SelectStatement(SQLProcessor& aSQLProcessor): theSQLProcessor(aSQLProcessor), Ordered_by(""), LimitNum(1 << 30), table_name(""){}
        ~SelectStatement(){}


        StatusResult          parseJoin(Tokenizer& aTokenizer){
            Token &theToken = aTokenizer.current();
            StatusResult theResult{joinTypeExpected}; //add joinTypeExpected to your errors file if missing...
            Keywords theJoinType{Keywords::join_kw}; //could just be a plain join
            if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
                theJoinType=theToken.keyword;
                aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
                if(aTokenizer.skipIf(Keywords::join_kw)) {
                    std::string theTable;
                    if((theResult=parseTableName(aTokenizer, theTable))) {
                        Join theJoin(theTable, theJoinType, std::string(""),std::string(""));
                        theResult.code=keywordExpected; //on...
                        if(aTokenizer.skipIf(Keywords::on_kw)) { //LHS field = RHS field
                            TableField LHS("");
                            if((theResult=parseTableField(aTokenizer, theJoin.onLeft))) {
                                if(aTokenizer.skipIf('=')) {
                                    if((theResult=parseTableField(aTokenizer, theJoin.onRight))) {
                                        joins.push_back(theJoin);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return theResult;
        }


        StatusResult          parseTableField(Tokenizer& aTokenizer, TableField& aTableField){
            /*std::string tablename1, tablename2;
            tablename1 = aTokenizer.current().data;
            aTokenizer.next();
            if(!aTokenizer.skipIf('.')) return StatusResult{syntaxError};
            TableField field1(""), field2("");
            field1 = aTokenizer.current().data;
            aTokenizer.next();
            if(!aTokenizer.skipIf(Operators::equal_op)) return StatusResult{syntaxError};
            tablename2 = aTokenizer.current().data;
            aTokenizer.next();
            if(!aTokenizer.skipIf('.')) return StatusResult{syntaxError};
            field2 = aTokenizer.current().data;
            aTokenizer.next();
            if(aJoin.table == tablename1) {
                aJoin.onLeft = field2;
                aJoin.onRight = field1;
                return StatusResult{};
            }
            else if(aJoin.table == tablename2){
                aJoin.onLeft = field1;
                aJoin.onRight = field2;
                return StatusResult{};
            }
            else return StatusResult{unknownTable};*/
            std::string thestr = "";
            thestr += aTokenizer.current().data;
            aTokenizer.next();
            if(aTokenizer.skipIf('.')) {
                thestr += ".";
                thestr += aTokenizer.current().data;
                aTokenizer.next();
            }
            aTableField = thestr;
            return StatusResult{};
        }


        StatusResult          parseTableName(Tokenizer& aTokenizer, std::string& thestring){
            thestring = aTokenizer.current().data;
            aTokenizer.next();
            return StatusResult{};
        }
        StatusResult          setLimit(Tokenizer& aTokenizer){
            if(aTokenizer.peek().type == TokenType::number){
                aTokenizer.next();
                LimitNum = std::stoi(aTokenizer.current().data);
                aTokenizer.next();
                return StatusResult{};
            }
            else return StatusResult{syntaxError};
        }
        StatusResult          setOrderBy(Tokenizer& aTokenizer){
            if(aTokenizer.peek().keyword == Keywords::by_kw){
                aTokenizer.next(2);
                Ordered_by = aTokenizer.current().data;
                aTokenizer.next();
                return StatusResult{};
            }
            else return StatusResult{syntaxError};
        }
        virtual   StatusResult  parse(Tokenizer &aTokenizer){
            aTokenizer.next();
            while(aTokenizer.more() && aTokenizer.current().keyword != Keywords::from_kw){
                if(aTokenizer.current().data!= ","){
                    attributes_list.push_back(aTokenizer.current().data);
                }
                aTokenizer.next();
            }
            aTokenizer.next(); // get the name of the table
            table_name = aTokenizer.current().data;
            aTokenizer.next();
            //std::clog << aTokenizer.current().data << std::endl;
            if(in_array<Keywords>(gJoinTypes, aTokenizer.current().keyword)) {
                std::clog << "run join" << std::endl;
                parseJoin(aTokenizer);
            }
            while(aTokenizer.more() && aTokenizer.current().data != ";"){
                Token curToken = aTokenizer.current();
                if(curToken.keyword == Keywords::where_kw){
                    if(!getFilter(aTokenizer, theFilters)) return StatusResult{syntaxError};
                }
                else if(curToken.keyword == Keywords::order_kw){
                    if(!setOrderBy(aTokenizer)) return StatusResult{syntaxError};
                }
                else if(curToken.data == "LIMIT" || curToken.data == "limit" || curToken.data == "Limit"){
                    if(!setLimit(aTokenizer)) return StatusResult{syntaxError};
                }
                else return StatusResult{syntaxError};
            }
            return StatusResult{};
        }
        virtual   StatusResult  run(std::ostream &aStream) const;
    };

    StatusResult SelectStatement::run(std::ostream &aStream) const {
        /*std::clog << "the attributes are:" << std::endl;
        for(auto name: attributes_list){
            std::clog << name << std::endl;
        }
        std::clog << "The limit is: " << LimitNum << std::endl;
        std::clog << "Ordered by: " << Ordered_by << std::endl;
        theFilters.PrintInfo();*/
        //std::clog << " The join is:" << joins[0].table << " " << joins[0].onLeft << " " << joins[0].onRight << std::endl;
        Database* activeDB = theSQLProcessor.getActiveDatabase();
        if(!activeDB) return StatusResult{noDatabaseSpecified};
        else {
            Filters filterCopy(theFilters);
            std::vector<Join> joinsCopy(joins);
            if(joins.size() == 0)
             return activeDB->selectData(table_name, attributes_list, filterCopy, Ordered_by, LimitNum);
            else // joins is not empty
            return activeDB->selectJoinData(table_name, attributes_list, filterCopy, Ordered_by, LimitNum, joinsCopy);
        }
    }


class UpdateStatement: public Statement{
    private:
    SQLProcessor& theSQLProcessor;
    std::string theTableName;
    Filters     theFilter, theSettings;
    public:
        UpdateStatement(SQLProcessor& aSQLProcessor): theSQLProcessor(aSQLProcessor){}
        StatusResult getSettings(Tokenizer& aTokenizer){
            while(aTokenizer.more() && aTokenizer.current().keyword != Keywords::where_kw && aTokenizer.current().data != ";")
            {
                Token& curToken = aTokenizer.current();
                if(curToken.data == ",") { aTokenizer.next(); continue;}
                if(aTokenizer.peek().data == "="){
                    Operand LHS, RHS;
                    LHS.name = aTokenizer.current().data;
                    aTokenizer.next(2);
                    Operators  theOp = Operators::equal_op;
                    Token thenextToken = aTokenizer.current();
                    RHS.type = thenextToken.type;
                    RHS.value.data = thenextToken.data;
                    Expression* newExpression = new Expression(LHS, theOp, RHS);
                    theSettings.add(newExpression);
                    aTokenizer.next();
                }
                else return StatusResult{syntaxError};
            }
            return StatusResult{};
        }
    virtual   StatusResult  parse(Tokenizer &aTokenizer){
            aTokenizer.next();
            theTableName = aTokenizer.current().data;
            if(aTokenizer.peek().keyword != Keywords::set_kw) return StatusResult{syntaxError};
            aTokenizer.next(2);
            if(getSettings(aTokenizer)){
                if(getFilter(aTokenizer, theFilter)) return StatusResult{};
                else return StatusResult{syntaxError};
            }
            else return StatusResult{syntaxError};
    }
    virtual   StatusResult  run(std::ostream &aStream) const;
    };



    StatusResult UpdateStatement::run(std::ostream &aStream) const {
        Database* activeDB = theSQLProcessor.getActiveDatabase();
        if(!activeDB) return StatusResult{noDatabaseSpecified};
        else {
            Filters filterCopy(theFilter);
            Filters settingCopy(theSettings);
            return activeDB->updateData(theTableName, settingCopy, filterCopy);
        }
    }

    class ShowIndexStatement: public Statement{
    private:
        SQLProcessor& theSQLProcessor;
    public:
        ShowIndexStatement(SQLProcessor& aProcessor): theSQLProcessor(aProcessor){}
        StatusResult  parse(Tokenizer& aTokenizer) override{
            aTokenizer.next();
            std::string thedata = aTokenizer.current().data;
            while(aTokenizer.more() && aTokenizer.current().data != ";") aTokenizer.next();
            if(thedata == "indexes" || thedata == "Indexes" || thedata == "INDEXES") return StatusResult{};
            else return StatusResult{syntaxError};
        }
        StatusResult run(std::ostream& anOutput) const override{
            Database* activeDB = theSQLProcessor.getActiveDatabase();
            if(!activeDB) return StatusResult{noDatabaseSpecified};
            else return activeDB->showIndexes(std::cout);
        }
    };




    class AlterStatement: public Statement{
    protected:
        SQLProcessor& theSQLProcessor;
        std::string tablename;
        std::vector<Token> token_list;
        std::vector<Attribute> add_attr_list;
    public:
        AlterStatement(SQLProcessor& aProcessor) : theSQLProcessor(aProcessor){}
        StatusResult parseAttribute(){
            size_t index = 0;
            while(index < token_list.size()){
                Attribute theAttribute;
                Token& theToken = token_list[index];
                if(theToken.type == TokenType::identifier){
                    theAttribute.setName(theToken.data);
                    index++;
                }
                theToken = token_list[index];
                switch(theToken.keyword){
                    case Keywords::integer_kw : theAttribute.setType(DataType::int_type); break;
                    case Keywords::varchar_kw: if(token_list[++index].type != TokenType::number) return StatusResult{Errors::syntaxError};
                        theAttribute.setType(DataType::varchar_type); theAttribute.setlength(std::stoi(token_list[index].data)); break;
                    case Keywords::float_kw: theAttribute.setType(DataType::float_type); break;
                    case Keywords ::boolean_kw: theAttribute.setType(DataType::bool_type); break;
                    case Keywords::datetime_kw: theAttribute.setType(DataType::datetime_type); break;
                    default: return StatusResult{Errors::syntaxError};
                }
                add_attr_list.push_back(theAttribute);
                index++;
                }
                return StatusResult{};
            }

        StatusResult parse(Tokenizer& aTokenizer) override{
            aTokenizer.next(2);
            tablename = aTokenizer.current().data;
            aTokenizer.next();
            if(aTokenizer.skipIf(Keywords::add_kw)){
                while(aTokenizer.more() && aTokenizer.current().data != ";"){
                    if(aTokenizer.current().data == "(" or aTokenizer.current().data == ")") aTokenizer.next();
                    else{
                        token_list.push_back(aTokenizer.current());
                        aTokenizer.next();
                    }
                }
                return parseAttribute();
            }
            else return StatusResult{syntaxError};

        }
        StatusResult  run(std::ostream& anOutput) const override{
            Database* activeDB = theSQLProcessor.getActiveDatabase();
            if(!activeDB) return StatusResult{noDatabaseSpecified};
            return activeDB->alterTable(tablename, add_attr_list);
        }
    };

    //STUDENT: Implement the SQLProcessor class here...
    SQLProcessor::SQLProcessor(CommandProcessor *aNext): CommandProcessor(aNext), activeDB(nullptr){}
 SQLProcessor::~SQLProcessor(){
     //std::cout << "sql destructor here" << std::endl;
     if(activeDB) delete activeDB;
 }
 StatusResult SQLProcessor::interpret(const Statement &aStatement) {
     return aStatement.run(std::cout);
 }





 Statement* SQLProcessor::getStatement(Tokenizer &aTokenizer) {
     std::vector<Keywords> theTerms{Keywords::create_kw, Keywords::drop_kw, Keywords::show_kw, Keywords::describe_kw, Keywords::insert_kw, Keywords::delete_kw, Keywords::select_kw, Keywords::update_kw, Keywords::alter_kw};
     Token& theToken = aTokenizer.current();
     if(theToken.keyword == Keywords::use_kw){
         if(aTokenizer.peek().keyword != Keywords::database_kw) return nullptr;
         else{
             aTokenizer.next();
             return new UseDBStatement(aTokenizer, *this);
         }
     }
     Keywords thekeyword = aTokenizer.current().keyword;
     if(std::find(theTerms.begin(), theTerms.end(), thekeyword) != theTerms.end()){
         Statement* theStatement_ptr = nullptr;
         switch(thekeyword){
             case Keywords::create_kw: theStatement_ptr = new CreateTableStatement(*this);break;
             case Keywords::show_kw: {
                 if(aTokenizer.peek().keyword == Keywords::tables_kw) theStatement_ptr = new ShowTablesStatement(*this);
                 else theStatement_ptr = new ShowIndexStatement(*this);
                 break;
             }
             case Keywords::drop_kw: theStatement_ptr = new DropTableStatement(*this); break;
             case Keywords::describe_kw: theStatement_ptr = new DescribeTableStatement(*this); break;
             case Keywords::insert_kw: theStatement_ptr = new InsertStatement(*this); break;
             case Keywords::delete_kw: theStatement_ptr = new DeleteStatement(*this); break;
             case Keywords::select_kw: theStatement_ptr = new SelectStatement(*this); break;
             case Keywords::update_kw: theStatement_ptr = new UpdateStatement(*this); break;
             case Keywords::alter_kw: theStatement_ptr = new AlterStatement(*this); break;
         }
         if(!theStatement_ptr->parse(aTokenizer))
             return nullptr;
         else return theStatement_ptr;
     }
     else return nullptr;
 }



 StatusResult SQLProcessor::createTable(Schema &aSchema) {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     return activeDB->createTable(aSchema, std::cout);
 }
 StatusResult SQLProcessor::showTables() const {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     return activeDB->showTables(std::cout);
 }
 StatusResult SQLProcessor::dropTable(const std::string &aName) {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     return activeDB->dropTable(aName, std::cout);
 }
 StatusResult SQLProcessor::describeTable(const std::string &aName) const {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     else return activeDB->describeTable(aName, std::cout);
 }
 StatusResult SQLProcessor::insertData(const std::string &tablename,const  std::vector<std::string> &attributes, const std::vector<std::vector<Token> > &records_list) const {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     else return activeDB->insertData(tablename, attributes, records_list);
 }
 StatusResult SQLProcessor::deleteData(const std::string &tablename) {
     if(!activeDB) return StatusResult{noDatabaseSpecified};
     else return activeDB->deleteData(tablename);
 }

}
