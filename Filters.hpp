//
//  Filters.hpp
//  RGAssignment6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <string>
#include "Errors.hpp"
#include "Value.hpp"
#include "Tokenizer.hpp"
#include <unordered_map>
namespace ECE141 {



  
  class Row;
  class Schema;
  using ValTypeOpt = std::optional<ValueType>;
  struct Operand {
    Operand() {}
    Operand(std::string &aName, TokenType aType, ValueType &aValue, uint32_t anId=0)
      : name(aName), type(aType), entityId(anId), value(aValue) {}
    
    TokenType   type; //so we know if it's a field, a const (number, string)...
    std::string name; //for named attr. in schema
    ValueType   value;
    uint32_t    entityId;
  };



  //---------------------------------------------------

  struct Expression {
    Operand     lhs;
    Operand     rhs;
    Operators   op;
    
    Expression(Operand &aLHSOperand, Operators anOp, Operand &aRHSOperand)
      : lhs(aLHSOperand), op(anOp), rhs(aRHSOperand) {}
    
    bool operator()(KeyValues &aList);
  };
  
  //---------------------------------------------------

  using Expressions = std::vector<Expression*>;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    Filters(const Filters &aFilters);
    ~Filters();
    
    size_t        getCount() const {return expressions.size();}
    bool          matches(KeyValues &aList) const;
    Filters&      add(Expression *anExpression);
    Filters&      addLogic(Keywords abool){
        Logic_list.push_back(abool);
        return *this;
    }

    void         PrintInfo() const {
        std::clog << "the filter is:\n";
        for(auto& theexp: expressions){
            std::clog << theexp->lhs.name << ": "; std::visit(Visitor(std::clog), theexp->rhs.value.data);
            std::clog << std::endl;
        }
        std::clog << "the filter ends here" << std::endl;
    }
    ValTypeOpt hasAttribute(std::string& aName){
        for(auto theexp: expressions){
            if(theexp->lhs.name == aName) return theexp->rhs.value;
        }
        return std::nullopt;
    }
    bool ChoosenbyPrimaryKey(const std::string& primary_key){
        if(expressions.size() == 1 && expressions.at(0)->lhs.name == primary_key){
            return true;
        }
        else return false;
    }
    bool MatchPrimaryKey(const ValueType& aValue);
    friend class Tokenizer;
    friend class Database;
    friend class SelectStatement;
  protected:
    Expressions  expressions;
    std::vector<Keywords> Logic_list;
  };
   
}

#endif /* Filters_h */

