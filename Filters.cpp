//
//  Filters.hpp
//  Assignement6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//


#include <stdio.h>
#include "Filters.hpp"
#include "Row.hpp"
#include "Schema.hpp"

namespace ECE141 {
    using OperationFunc = bool(*)(const ValueType& lhs, const Operand& rhs);

    //-------------convert from keyword to bool result---------------
    bool equals(const ValueType& lhs, const Operand& rhs){
        return lhs.data == rhs.value.data;
    }
    bool not_equal(const ValueType& lhs, const Operand& rhs){
        return lhs.data != rhs.value.data;
    }
    bool less_than(const ValueType& lhs, const Operand& rhs){
        return lhs.data < rhs.value.data;
    }
    bool less_than_or_equal(const ValueType& lhs, const Operand& rhs){
        return lhs.data <= rhs.value.data;
    }
    bool greater_than(const ValueType& lhs, const Operand& rhs){
        return lhs.data > rhs.value.data;
    }
    bool greater_than_or_equal(const ValueType& lhs, const Operand& rhs){
        return lhs.data >= rhs.value.data;
    }


    static std::unordered_map<Operators, OperationFunc> OperatorKeyword_bool = {{Operators::equal_op, equals},
                                                                                {Operators::notequal_op, not_equal},
                                                                                {Operators::lt_op, less_than},
                                                                                {Operators::lte_op, less_than_or_equal},
                                                                                {Operators::gt_op, greater_than},
                                                                                {Operators::gte_op, greater_than_or_equal}};

    bool Expression::operator()(KeyValues &aList) {
        return OperatorKeyword_bool[op](aList[lhs.name], rhs);
    }


    Filters::Filters() {
        Logic_list.clear();
        Logic_list.push_back(Keywords::and_kw);
    }
    Filters::Filters(const ECE141::Filters &aFilters) {
        expressions.clear();
        Logic_list.clear();
        for(auto ptr: aFilters.expressions){
            Expression* tmp = new Expression(ptr->lhs, ptr->op, ptr->rhs);
            expressions.push_back(tmp);
        }
        for(auto theKey: aFilters.Logic_list){
            Logic_list.push_back(theKey);
        }
    }
    Filters::~Filters() {
        for(auto ptr: expressions){
            if(ptr) delete ptr;
        }
    }
    Filters& Filters::add(Expression *anExpression) {
        expressions.push_back(anExpression);
        return *this;
    }
    bool Filters::matches(KeyValues &aList) const {
        bool res = true;
        size_t total = expressions.size();
        for(int i = 0; i<total; i++){
            if(Logic_list.at(i) == Keywords::and_kw) res = res && (*expressions[i])(aList);
            else res = res || (*expressions[i])(aList);
        }
        return res;
    }
    bool Filters::MatchPrimaryKey(const ValueType &aValue) {
        Operators theOp = expressions[0]->op;
        return OperatorKeyword_bool[theOp](aValue, expressions[0]->rhs);
    }
}



