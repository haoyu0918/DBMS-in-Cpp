//
//  Database.cpp
//  Database1
//
//  Created by rick gessner on 4/12/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <sstream>
#include "Database.hpp"
#include "View.hpp"
#include "Storage.hpp"
#include "FolderReader.hpp"
#include <iomanip>

//this class represents the database object.
//This class should do actual database related work,
//we called upon by your db processor or commands

namespace ECE141 {
    
  Database::Database(const std::string aName, CreateNewStorage)
    : name(aName), storage(aName, CreateNewStorage{}), isChanged(true), IndexofIndex(storage, "IndexofIndex"){
  }
  
  Database::Database(const std::string aName, OpenExistingStorage)
    : name(aName), storage(aName, OpenExistingStorage{}), isChanged(false), theToc(), IndexofIndex(storage, "IndexofIndex"){
      if(!storage.loadfromBlock(theToc, 0)) throw "Errors";
      if(!storage.loadfromBlock(IndexofIndex, 1)) throw "Errors"; // load indexofindex from database
  }
  
  Database::~Database() {
      //std::clog << "DB desctructor called" << std::endl;
      std::string thePath = getDatabasePath(name);
      if(isChanged && FolderReader().exists(thePath)) {
          //std::cout << "call the destructor of DB" << std::endl;
          std::ostringstream anOutstream;
          theToc.encode(anOutstream);
          StorageBlock aBlock(BlockType::meta_block);
          aBlock.header.id = 0;
          const char* theBuff = anOutstream.str().c_str();
          memcpy(aBlock.data, theBuff, kPayloadSize);
          storage.writeBlock(aBlock, 0);
          storage.overwriteBlock(IndexofIndex, 1);
      }
  }
  StatusResult Database::showoneBlock(uint32_t index, BlockType aType, std::ostream &anOutput) {
      if(aType == BlockType::meta_block) {
          anOutput << std::setw(14) << "meta" << '|';
          anOutput << std::setw(55) << " " << '|';
      }
      else if(aType == BlockType::entity_block){
          anOutput << std::setw(14) << "schema" << "|";
          Schema aSchema("");
          storage.loadfromBlock(aSchema, index);
          anOutput << std::setw(55) << aSchema.getName() << '|';
      }
      else if(aType == BlockType::data_block){
          anOutput << std::setw(14) << "data" << "|";
          Row aRow;
          storage.loadfromBlock(aRow, index);
          std::stringstream ss;
          aRow.ShowData(ss);
          uint32_t theheaderid = aRow.getHeaderId();
          ss<< " " << "\"" << getTableNameFromHash(theheaderid) << '\"';
          anOutput << std::setw(55) << ss.str() << '|';
      }
      else if(aType == BlockType::index_block){
          anOutput << std::setw(14) << "index" << "|";
          Index theIndexBlock(storage);
          storage.loadfromBlock(theIndexBlock, index);
          std::stringstream ss;
          theIndexBlock.showData(ss);
          anOutput << std::setw(55) << ss.str() << '|';
      }
      return StatusResult{};
  }
  StatusResult Database::describeDatabase(std::ostream& anOutput) {
      anOutput << "+-----------+--------------+-------------------------------------------------------+" << std::endl;
      anOutput << "| Block#    | Type         | Other                                                 |\n";
      anOutput << "+-----------+--------------+-------------------------------------------------------+\n";
      uint32_t total = storage.getTotalBlockCount();
      int count = 0;
      for(uint32_t indx = 0; indx<total; indx++){
          BlockType thetype = char2Blocktype[storage.GetTheBlockHeader(indx).type];
          if(thetype != BlockType::free_block) {
              anOutput << '|' << std::setw(11) << std::left << indx << "|";
              showoneBlock(indx, thetype, anOutput);
              anOutput << std::endl;
              count++;
          }
      }
      anOutput << "+-----------+--------------+-------------------------------------------------------+\n";
      anOutput << count << " rows in set" << std::endl;
      return StatusResult{};
  }
  StatusResult Database::dropDatabase(std::ostream& anOutput) {
      storage.free();
      std::filesystem::path thepath(getDatabasePath(name));
      std::filesystem::remove(thepath);
      anOutput << "dropped database " << name << std::endl;
      return StatusResult{};
  }
  StatusResult  Database::createTableIndex(Schema &aSchema) {
      StringOp thefield = aSchema.getPrimaryKeyName();
      if(thefield == std::nullopt) return StatusResult{primaryKeyError};
      uint32_t theHashNum = hashString(aSchema.name.c_str());
      DataType thetype = aSchema.getAttribute(*thefield)->getType();
      Index theIndex(storage, *thefield, theHashNum, thetype);
      StatusResult theres = storage.savetoBlock(theIndex);
      std::string thename = aSchema.name + "." + (*thefield);
      ValueType theKey{std::string(thename)};
      IndexofIndex.addKeyValue(theKey, theres.value); // save the change to IndexofIndex
      return StatusResult{};
  }
  StatusResult Database::createTable(Schema &aSchema, std::ostream& anOutput) {
      if(theToc.Schemas_Map.find(aSchema.getName()) != theToc.Schemas_Map.end()) return StatusResult{Errors::tableExists};
      isChanged = true;
      //std::clog << "am writing by save" << std::endl;
      StatusResult theres = storage.savetoBlock(aSchema);
      if(!theres) return StatusResult{Errors::unknownError};
      theToc.Schemas_Map[aSchema.getName()] = theres.value;
      anOutput << "Created Table " << aSchema.getName() << " (OK) " << std::endl;
      aSchema.setBlockNum(theres.value);
      StatusResult saveres = createTableIndex(aSchema);
      if(!saveres) return saveres;
      return StatusResult{};
  }
  StatusResult Database::showTables(std::ostream &anOutput) {
      anOutput << "+----------------------+\n";
      anOutput << "| Tables_in_mydb       |\n";
      anOutput << "+----------------------+\n";
      for(auto thepair: theToc.Schemas_Map){
          anOutput << "| " << std::setw(21) << std::left << thepair.first << '|' << std::endl;
      }
      anOutput << "+----------------------+\n";
      anOutput << theToc.Schemas_Map.size() << " rows in set" << std::endl;
      return StatusResult{};
  }
  StatusResult Database::dropTable(const std::string &tableName, std::ostream &anOutput) {
      if(theToc.Schemas_Map.find(tableName) == theToc.Schemas_Map.end()) return StatusResult{unknownTable};
      uint32_t  schemaIndex = theToc.Schemas_Map[tableName];
      Schema theSchema("");
      storage.loadfromBlock(theSchema, schemaIndex);
      ValueType theKey{std::string(theSchema.getNameandPrimaryKey())};
      uint32_t tableIndexNum = IndexofIndex.getValue(theKey);
      deleteData(tableName);
      storage.freeABlock(schemaIndex);
      storage.freeABlock(tableIndexNum);
      isChanged = true;
      theToc.Schemas_Map.erase(tableName);
      IndexofIndex.removeKeyValue(theKey);
      anOutput << "table " << tableName<< " was dropped" << std::endl;
      return StatusResult{noError};
  }
  StatusResult Database::describeTable(const std::string &tableName, std::ostream &anOutput) {
      if(theToc.Schemas_Map.find(tableName) == theToc.Schemas_Map.end()) return StatusResult{unknownTable};
      uint32_t index = theToc.Schemas_Map[tableName];
      Schema theSchema("");
      if(storage.loadfromBlock(theSchema, index)){
          theSchema.TableView(anOutput);
          return StatusResult{};
      }
      else return StatusResult{readError};
  }
  using Attname_type = std::unordered_map<std::string, DataType>;


  StatusResult readOneAttr(const Token& aToken, Row& aRow, const std::string& attr_name, DataType Type){
      if(Type == DataType::int_type){
          if(aToken.type == TokenType::number){
              uint32_t  thenum = uint32_t(std::stoi(aToken.data));
              aRow.rowdata[attr_name] = ValueType(thenum);
              return StatusResult{};
          }
          else return StatusResult{Errors::invalidArguments};
      }
      else if(Type == DataType::float_type){
          if(aToken.type == TokenType::number){
              float  thenum = float(std::stof(aToken.data));
              aRow.rowdata[attr_name] = ValueType(thenum);
              return StatusResult{};
          }
          else return StatusResult{Errors::invalidArguments};
      }
      else if(Type == DataType::bool_type){
          std::vector<std::string> bool_keywords{"false", "False", "FALSE", "True", "TRUE", "true"};
          if(std::find(bool_keywords.begin(), bool_keywords.end(), aToken.data) != bool_keywords.end()) {
              bool thebool;
              if(aToken.data[0] == 'T' || aToken.data[0] == 't') thebool = true;
              else thebool = false;
              aRow.rowdata[attr_name] = ValueType{thebool};
              return StatusResult{};
          }
          else return StatusResult{invalidArguments};
      }
      else {
          aRow.rowdata[attr_name] = ValueType{std::string(aToken.data)};
          return StatusResult{};
      }
  }
  StatusResult GetOneRow(Attname_type& theMap, const std::vector<std::string>& attributes, const std::vector<Token>& aRecord, Row& aRow){
      if(attributes.size() != aRecord.size()) return StatusResult{Errors::syntaxError};
      for(int i = 0; i < attributes.size(); i++){
          std::string attr_name = attributes[i];
          DataType aType = theMap[attr_name];
          StatusResult res = readOneAttr(aRecord[i], aRow, attributes[i], aType);
          if(!res) return res;
      }
      return StatusResult{};

  }
  vartype getDafaultValue(DataType aType, std::string astring){
      vartype  res;
      if(aType == DataType::int_type){
          res = uint32_t(std::stoi(astring));
      }
      else if(aType == DataType::float_type){
          res = float(std::stof(astring));
      }
      else if(aType == DataType::bool_type){
          if(astring[0] == 'T' || astring[0] == 't') res = true;
          else res = false;
      }
      else res = std::string(astring);
      return res;
  }
  StatusResult SetDefaultForRow(Row& aRow, std::vector<Attribute>& AttributesofDefault){
      for(auto& theattr: AttributesofDefault){
          ValueType avalue;
          avalue.data = getDafaultValue(theattr.getType(), std::get<std::string>(theattr.getDefaultValue()));
          aRow.rowdata[theattr.getName()] = avalue;
      }
      return StatusResult{};
  }
  StatusResult Database::insertData(const std::string &tablename, const std::vector<std::string> &attributes, const std::vector<std::vector<Token> > &records_list) {
      if(theToc.Schemas_Map.find(tablename) == theToc.Schemas_Map.end()) return StatusResult{Errors::unknownTable};
      uint32_t  theindex = theToc.Schemas_Map[tablename];
      Schema theSchema("");
      storage.loadfromBlock(theSchema, theindex);
      uint32_t  TableIndexNum = IndexofIndex.getValue(ValueType{std::string(theSchema.getNameandPrimaryKey())});
      Index  TableIndex(storage, "");
      storage.loadfromBlock(TableIndex, TableIndexNum); // load the index from database
      std::string primaryKeyName = theSchema.getPrimaryKeyName().value();
      std::vector<Attribute> AttributesofDefault;
      std::vector<std::string> auto_imcrement_attributes;
      for(auto& theAttr: theSchema.attributes){
          if(std::find(attributes.begin(), attributes.end(),theAttr.getName()) == attributes.end() && theAttr.hasDefaultValue()){
              AttributesofDefault.push_back(theAttr);
          }
          if(std::find(attributes.begin(), attributes.end(), theAttr.getName())== attributes.end() && theAttr.iftAuto_increment()){
              auto_imcrement_attributes.push_back(theAttr.getName());
          }
      }
      Attname_type attributes_type;
      for(auto attr_name : attributes){
          AttrOpt findres = theSchema.getAttribute(attr_name);
          if( findres!= std::nullopt){
              attributes_type[attr_name] = findres->getType();
          }
          else return StatusResult{Errors::unknownAttribute};
      }
      const char* thestr = tablename.c_str();
      uint32_t  tablehashNum = hashString(thestr);
      for(auto& one_record: records_list){
          Row aRow;
          StatusResult res = GetOneRow(attributes_type, attributes, one_record, aRow);
          if(!res) return res;
          //aRow.ShowData(std::clog);
          aRow.setHeaderId(tablehashNum);
          SetDefaultForRow(aRow, AttributesofDefault);
          for(auto attr : auto_imcrement_attributes){
              vartype thevar = uint32_t(theSchema.getNextAutoIncrementValue());
              aRow.rowdata[attr].data = thevar;
          }
          StatusResult saveres = storage.savetoBlock(aRow);
          TableIndex.addKeyValue(aRow.rowdata[primaryKeyName], saveres.value); // add key value to the index
      }
      storage.overwriteBlock(theSchema, theindex); // overwrite the schema to file
      if(TableIndex.isChanged()) storage.overwriteBlock(TableIndex, TableIndexNum);
      std::cerr << "inserted successful" << std::endl;
      return StatusResult{};
  }

  StatusResult Database::deleteData(const std::string &tablename) {
      // if could not find the table name...
      if(theToc.Schemas_Map.find(tablename) == theToc.Schemas_Map.end()) return StatusResult{Errors::unknownTable};
      Schema theSchema("");
      storage.loadfromBlock(theSchema, theToc.Schemas_Map[tablename]);
      uint32_t tableIndexNum = IndexofIndex.getValue(ValueType{std::string(theSchema.getNameandPrimaryKey())});
      Index tableIndex(storage, "");
      storage.loadfromBlock(tableIndex, tableIndexNum);
      ValueMap& thelist = tableIndex.getList();
      Timer aTimer;
      int affected = 0;
      aTimer.start();
      for(auto& thepair: thelist){
          storage.freeABlock(thepair.second);
          affected++;
      } // delete all data from the table
      aTimer.stop();
      tableIndex.clearall();
      theSchema.total_data_num = 0;
      storage.overwriteBlock(theSchema, theToc.Schemas_Map[tablename]); // update the schema
      storage.overwriteBlock(tableIndex, tableIndexNum); // update the index of the table
      std::cout << affected << " rows affected ( " << aTimer.elapsed() << " ms.)" <<std::endl;
      return StatusResult{};
  }
  StatusResult ChangingType(Expression* aExpression, DataType aType){
      if(aType == DataType::int_type){
          if(aExpression->rhs.type == TokenType::number){
              uint32_t thenum = uint32_t (std::stoi(std::get<std::string>(aExpression->rhs.value.data)));
              aExpression->rhs.value.data = thenum;
              return StatusResult{};
          }
          else return StatusResult{Errors::invalidArguments};
      }
      else if(aType == DataType::float_type){
          if(aExpression->rhs.type == TokenType::number){
              float thenum = float (std::stof(std::get<std::string>(aExpression->rhs.value.data)));
              aExpression->rhs.value.data = thenum;
              return StatusResult{};
          }
          else return StatusResult{Errors::invalidArguments};
      }
      else if(aType == DataType::bool_type){
          std::vector<std::string> bool_keywords{"false", "False", "FALSE", "True", "TRUE", "true"};
          std::string  thevalue = std::get<std::string>(aExpression->rhs.value.data);
          if(std::find(bool_keywords.begin(), bool_keywords.end(),thevalue) != bool_keywords.end()) {
              bool thebool;
              if( thevalue[0] == 'T' || thevalue[0] == 't') thebool = true;
              else thebool = false;
              aExpression->rhs.value = thebool;
              return StatusResult{};
          }
          else return StatusResult{invalidArguments};
      }
      else {
          return StatusResult{};
      }
  }
  StatusResult Database::ChangingFilters(Filters& aFilter, const Schema& aSchema){
      for(Expression* aExpression: aFilter.expressions){
          std::string attr_name = aExpression->lhs.name;
          AttrOpt the_attr = aSchema.getAttribute(attr_name);
          if(the_attr != std::nullopt){
              if(!ChangingType(aExpression, the_attr->getType()))
                  return StatusResult{invalidArguments};
          }
          else return StatusResult{unknownAttribute};
      }
      return StatusResult{};
  }
  class RowCollector: public BlockVisitor{
  public:
      RowCollection res;
      uint32_t target_num;
      RowCollector(uint32_t aNum): target_num(aNum){}
      StatusResult  visit(StorageBlock& aBlock) override {
          if(aBlock.header.type == 'D' && aBlock.header.id == target_num){
              Row aRow;
              std::stringstream  anInstream(std::ios::in | std::ios::out | std::ios::binary);
              anInstream.write((char*)&aBlock.data, kPayloadSize);
              aRow.decode(anInstream);
              aRow.setHeaderId(aBlock.header.id);
              res.push_back(aRow);
          }
          return StatusResult{};
      }
  };
  RowCollection  Database::GetRowsofAtable(Schema& aSchema) {
      uint32_t  tableIndexNum = IndexofIndex.getValue(ValueType{std::string(aSchema.getNameandPrimaryKey())});
      Index tableIndex(storage, "");
      storage.loadfromBlock(tableIndex, tableIndexNum);
      RowCollector aRowCollector(tableIndex.getSchemaId());
      tableIndex.each(aRowCollector);
      /*for(uint32_t index = 0; index < count ; index ++){
          BlockHeader cur_header = storage.GetTheBlockHeader(index);
          if(cur_header.type == 'D' && cur_header.id == HashNum){
              // find the data of table name
              Row aRow;
              storage.loadfromBlock(aRow, index);
              res.push_back(aRow);
          }
      }*/
      return aRowCollector.res;
  }

  StatusResult Database::selectData(std::string tablename, const std::vector<std::string> &attributes_name, Filters &thefilters, const std::string &Ordered_by, const int LimitNum) {
      if(theToc.Schemas_Map.find(tablename) == theToc.Schemas_Map.end()) return StatusResult{unknownTable};
      uint32_t  table_index = theToc.Schemas_Map[tablename];
      Schema theSchema("");
      storage.loadfromBlock(theSchema, table_index);
      if(attributes_name.at(0) == "*") {
          if(attributes_name.size() != 1) return StatusResult{syntaxError};
      }
      if(Ordered_by != "")
          if(theSchema.getAttribute(Ordered_by) == std::nullopt) return StatusResult{unknownAttribute};
      for(auto attr_name: attributes_name){ // check if the attributes are in the table
          if(attr_name != "*"){
              if(theSchema.getAttribute(attr_name) == std::nullopt) return StatusResult{unknownAttribute};
          }
      }
      StatusResult res = ChangingFilters(thefilters, theSchema);
      bool OnlyFilterByPrimaryKey = thefilters.ChoosenbyPrimaryKey(theSchema.getPrimaryKeyName().value());
      if(!res) return res;
      Timer theTimer;
      theTimer.start();
      RowCollection  res_rows;
      if(!OnlyFilterByPrimaryKey){
      RowCollection  allrows = GetRowsofAtable(theSchema);
      for(auto aRow: allrows){
          if(thefilters.matches(aRow.rowdata)) res_rows.push_back(aRow);
      }
      }
      else{
          SelectOnlyByPrimaryKey(theSchema, res_rows, thefilters);
      }
      theTimer.stop();
      //std::cout << "the res num is:" << res_rows.size() << std::endl;
      ShowSelectTableView(std::cout, res_rows, Ordered_by, LimitNum, attributes_name, theSchema, theTimer);
      return StatusResult{};
  }

  StatusResult   Database::SelectOnlyByPrimaryKey(Schema& aSchema, RowCollection &aRowCollection, Filters& theFilter) {
      std::clog << "select only by primary key " << std::endl;
      uint32_t  tableIndexNum = IndexofIndex.getValue(ValueType{std::string(aSchema.getNameandPrimaryKey())});
      Index tableIndex(storage, "");
      storage.loadfromBlock(tableIndex, tableIndexNum);
      for(auto& thepair: tableIndex.getList()){
          if(theFilter.MatchPrimaryKey(thepair.first)){
              Row theRow;
              storage.loadfromBlock(theRow, thepair.second);
              aRowCollection.push_back(theRow);
          }
      }
      return StatusResult{};
  }

  struct Compare{
      std::string attr_name;
      Compare(std::string astr): attr_name(astr){}
      bool operator ()(Row& e1, Row& e2) {
          return e1.rowdata[attr_name].data < e2.rowdata[attr_name].data;
      }
  };
  StatusResult ShowContent(std::ostream& anOutput, std::vector<std::string>& columns, RowCollection& aRowCollections, const int limitNum, int width, double thetime){
      for(int i = 0; i< width ; i++)
          anOutput << "+--------------------";
      anOutput << "+\n" << std::left;
      for(int i = 0; i< width; i++){
          anOutput << "|" << std::setw(20) << columns.at(i);
      }
      anOutput << "|\n";
      for(int i = 0; i< width ; i++)
          anOutput << "+--------------------";
      anOutput << "+\n" << std::left;
      int count = 0;
      for(auto& arow: aRowCollections){
          if(count == limitNum) break;
          count++;
          for(auto& attr_name: columns){
              vartype content = std::string(" ");
              if(arow.rowdata.find(attr_name) != arow.rowdata.end())
                  content = arow.rowdata[attr_name].data;
              anOutput << "|" << std::setw(20);
              std::visit(Visitor(anOutput), content);
          }
          anOutput << "|\n";
      }
      for(int i = 0; i< width ; i++)
          anOutput << "+--------------------";
      anOutput << "+\n" << std::left;
      anOutput << count << " rows in set ";
      anOutput << "(" << thetime << " ms.)\n";
      return StatusResult{};
  }
  StatusResult  Database::ShowSelectTableView(std::ostream &anOutput, RowCollection& aRowCollection, const std::string& Ordered_by, const int Limitnum, const std::vector<std::string>& attributes_name, Schema& aSchema, Timer& aTimer) {
      if(Ordered_by != ""){// sort the vector
          std::sort(aRowCollection.begin(), aRowCollection.end(), Compare(Ordered_by));
      }
      int width;
      std::vector<std::string> columns;
      if(attributes_name[0] != "*") {
          width = attributes_name.size();
          columns = attributes_name;
      }
      else {
          width = aSchema.attributes.size();
          for(auto attr: aSchema.attributes) columns.push_back(attr.getName());
      }
      ShowContent(anOutput, columns, aRowCollection, Limitnum, width, aTimer.elapsed());
      return StatusResult{};
  }

  StatusResult  Database::updateData(std::string tablename, Filters &theSettings, Filters& theFilter) {
      if(theToc.Schemas_Map.find(tablename) == theToc.Schemas_Map.end()) return StatusResult{unknownTable};
      uint32_t  table_index = theToc.Schemas_Map[tablename];
      Schema theSchema("");
      storage.loadfromBlock(theSchema, table_index);
      StatusResult res = ChangingFilters(theSettings, theSchema);
      if(!res) return res;
      res = ChangingFilters(theFilter, theSchema);
      if(!res) return res;
      return updateDataHelper(theSchema, theSettings, theFilter);
  }

  StatusResult  Database::updateDataHelper(Schema& aSchema, Filters &theSetting, Filters &theFilter) {
      uint32_t count = storage.getTotalBlockCount();
      uint32_t tableIndexNum = IndexofIndex.getValue(ValueType{std::string(aSchema.getNameandPrimaryKey())});
      Index tableIndex(storage, "");
      storage.loadfromBlock(tableIndex, tableIndexNum);
      ValueMap& thelist = tableIndex.getList();
      std::string primary_key_name = aSchema.getPrimaryKeyName().value();
      ValTypeOpt search_res = theSetting.hasAttribute(primary_key_name);
      std::vector<ValueType> update_memo;
      Timer aTimer;
      aTimer.start();
      int affected = 0;
      for(auto& thepair: thelist){
          uint32_t  theindex = thepair.second;
          Row aRow;
          storage.loadfromBlock(aRow, theindex);
          if(theFilter.matches(aRow.rowdata)){
              if(search_res != std::nullopt){ // update primary key
                  update_memo.push_back(aRow.rowdata[primary_key_name]);
              }
              updateOneRow(aRow, theSetting);
              affected ++;
              storage.overwriteBlock(aRow, theindex);
          }
      }
      aTimer.stop();
      if(search_res != std::nullopt){
          if(!updateIndex(tableIndex, update_memo, search_res.value())) return StatusResult{primaryKeyError};
      }
      if(tableIndex.isChanged()) storage.overwriteBlock(tableIndex, tableIndexNum);
      std::clog << "Update successful" << std::endl;
      std::cout << affected <<  "rows affected " << "( " << aTimer.elapsed() << " ms.)\n";
     return StatusResult{};
  }
  StatusResult Database::updateIndex(Index &theIndex, std::vector<ValueType> &thememo, ValueType &UpdatedValue) {
      if(thememo.size() > 1) return StatusResult{primaryKeyError};
      for(auto& thevalue: thememo){
          uint32_t theBlockNum = theIndex.getValue(thevalue);
          theIndex.removeKeyValue(thevalue);
          theIndex.addKeyValue(UpdatedValue, theBlockNum);
      }
      return StatusResult{};
  }
  StatusResult  Database::updateOneRow(Row &aRow, Filters &aSetting) {
      for(auto& exp : aSetting.expressions){
          std::string attr_name = exp->lhs.name;
          aRow.rowdata[attr_name] = exp->rhs.value;
      }
      return StatusResult{};
  }

  StatusResult Database::showIndexes(std::ostream &anOutput) {
      anOutput << "+----------------------------+------------------------------+\n";
      anOutput << "|" << std::setw(28) << std::left << "table" << "|" << std::setw(30) << "field" << "|\n";
      anOutput << "+----------------------------+------------------------------+\n";
      int count = 0;
      for(auto& thepair:IndexofIndex.getList()){
          std::string thekey = std::get<std::string>(thepair.first.data);
          size_t pos = thekey.find(".");
          std::string str1 = thekey.substr(0, pos), str2 = thekey.substr(pos+1);
          anOutput << "|" << std::setw(28) << str1 << "|" << std::setw(30) << str2 << "|\n";
          count++;
      }
      anOutput << "+----------------------------+------------------------------+\n";
      anOutput << count << " rows in set\n";
      return StatusResult{};
  }

  StatusResult   Database::selectJoinData(std::string tablename, const std::vector<std::string> &attributes_name, Filters &thefilters, const std::string &Ordered_by, const int LimitNum, std::vector<Join>& joins) {
      if(theToc.Schemas_Map.count(tablename) == 0 || theToc.Schemas_Map.count(joins[0].table) == 0) return StatusResult{unknownTable};
      Schema theSchema1(""), theSchema2("");
      uint32_t  index1 = theToc.Schemas_Map[tablename], index2 = theToc.Schemas_Map[joins[0].table];
      storage.loadfromBlock(theSchema1, index1); storage.loadfromBlock(theSchema2, index2);
      std::vector<std::string> table1FieldNames, table2FieldNames;
      if(attributes_name[0] != "*"){
          for(auto theattr: attributes_name){
              if(theSchema1.getAttribute(theattr) != std::nullopt) table1FieldNames.push_back(theattr);
              else if(theSchema2.getAttribute(theattr) != std::nullopt) table2FieldNames.push_back(theattr);
              else // both don't have
              return StatusResult{unknownAttribute};
          }
      }
      std::string join1_field = "", join2_field = "";
      std::vector<std::string> temp1 = ParseTableNameField(joins[0].onLeft), temp2 = ParseTableNameField(joins[0].onRight);
      if(temp1[0] == tablename) {
          join1_field = temp1[1];
          join2_field = temp2[1];
      }
      else {
          join1_field = temp2[1];
          join2_field = temp1[1];
      }
      joins[0].onLeft = join1_field;
      joins[0].onRight = join2_field;
      Timer aTimer;
      aTimer.start();
      RowCollection  join_res = selectJoinDataHelper(theSchema1, theSchema2, joins[0], table1FieldNames, table2FieldNames);
      aTimer.stop();
      Schema theSchema("");
      ShowSelectTableView(std::cout, join_res, Ordered_by, LimitNum, attributes_name, theSchema, aTimer);
      return StatusResult{};
  }

  StatusResult LeftJoinRows(RowCollection& rowsFromTable1, RowCollection& rowsFromTable2, std::string& fieldname1, std::string& fieldname2, const std::vector<std::string>& table1Fields, const std::vector<std::string>& table2Fields, RowCollection& res ){
      for(auto& therow: rowsFromTable1){
          bool hasMatch = false;
          for(auto& newrow: rowsFromTable2){
              if(therow.rowdata[fieldname1].data == newrow.rowdata[fieldname2].data){
                  Row table1row(therow);
                  for(auto new_attr_name: table2Fields){
                      table1row.rowdata[new_attr_name] = newrow.rowdata[new_attr_name];
                  }
                  res.push_back(table1row);
                  hasMatch = true;
              }
          }
          if(!hasMatch) {
              Row table1row(therow);
              for(auto new_attr_name: table2Fields){
                  table1row.rowdata[new_attr_name] = std::string("NULL");
              }
              res.push_back(table1row);
          }
      }
      return StatusResult{};
  }

    StatusResult RightJoinRows(RowCollection& rowsFromTable1, RowCollection& rowsFromTable2, std::string& fieldname1, std::string& fieldname2, const std::vector<std::string>& table1Fields, const std::vector<std::string>& table2Fields, RowCollection& res ){
        for(auto& therow: rowsFromTable2){
            bool hasMatch = false;
            for(auto& newrow: rowsFromTable1){
                if(therow.rowdata[fieldname2].data == newrow.rowdata[fieldname1].data){
                    Row table2row(therow);
                    for(auto new_attr_name: table1Fields){
                        table2row.rowdata[new_attr_name] = newrow.rowdata[new_attr_name];
                    }
                    res.push_back(table2row);
                    hasMatch = true;
                }
            }
            if(!hasMatch) {
                Row table2row(therow);
                for(auto new_attr_name: table1Fields){
                    table2row.rowdata[new_attr_name] = std::string("NULL");
                }
                res.push_back(table2row);
            }
        }
        return StatusResult{};
    }
  RowCollection   Database::selectJoinDataHelper(Schema &theSchema1, Schema &theSchema2, Join &theJoin, const std::vector<std::string> &table1Fields, const std::vector<std::string> &table2Fields) {
      RowCollection rowsFromTable1 = GetRowsofAtable(theSchema1), rowsFromTable2 = GetRowsofAtable(theSchema2);
      std::string fieldname1 = theJoin.onLeft, fieldname2 = theJoin.onRight;
      RowCollection res;
      if(theJoin.joinType == Keywords::left_kw)
          LeftJoinRows(rowsFromTable1, rowsFromTable2, fieldname1, fieldname2, table1Fields, table2Fields, res);
      else
          RightJoinRows(rowsFromTable1, rowsFromTable2, fieldname1, fieldname2, table1Fields, table2Fields, res);
      return res;
  }

  StatusResult  Add_attr_helper(Row& aRow, Attribute& theAttribute){
      std::string attr_name = theAttribute.getName();
      DataType theType = theAttribute.getType();
      if(theType == DataType::int_type){
          aRow.rowdata[attr_name] = ValueType(uint32_t(0));
      }
      else if(theType == DataType::float_type){
          aRow.rowdata[attr_name] = ValueType(float(0.0));
      }
      else if(theType == DataType::varchar_type){
          aRow.rowdata[attr_name] = ValueType(std::string(""));
      }
      else if(theType == DataType::bool_type){
          aRow.rowdata[attr_name] = ValueType(false);
      }
      else if(theType == DataType::datetime_type){
          aRow.rowdata[attr_name] = ValueType(std::string("No Date"));
      }
      return StatusResult{};
  }
  StatusResult Database::alterTable(std::string tablename, const std::vector<Attribute> &add_attr_list) {
      if(add_attr_list.size() != 1) return StatusResult{syntaxError};
      Attribute attr_to_be_add = add_attr_list.at(0);
      Schema theSchema("");
      uint32_t  theIndexofSchema = theToc.Schemas_Map[tablename];
      storage.loadfromBlock(theSchema, theIndexofSchema); // load from the table
      if(theSchema.getAttribute(attr_to_be_add.getName()) != std::nullopt) return StatusResult{Errors::invalidAttribute};
      theSchema.addAttribute(attr_to_be_add); // change the schema
      storage.overwriteBlock(theSchema, theIndexofSchema); // save the schema
      uint32_t tableIndexNum = IndexofIndex.getValue(ValueType{std::string(theSchema.getNameandPrimaryKey())});
      Index tableIndex(storage, "");
      storage.loadfromBlock(tableIndex, tableIndexNum);
      ValueMap& thelist = tableIndex.getList(); // get the block number of all data in db
      for(auto& thepair: thelist){ // we would update all rows in the db, and add the new attribute to the row
          uint32_t  theindex = thepair.second;
          Row aRow;
          storage.loadfromBlock(aRow, theindex);
          Add_attr_helper(aRow, attr_to_be_add);
          storage.overwriteBlock(aRow, theindex); // save the row to the db
      }
      return StatusResult{};
  }



}

