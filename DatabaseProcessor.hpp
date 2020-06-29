//
// Created by 李浩宇 on 4/12/20.
//

#ifndef ASSIGNMENT2_DATABASEPROCESSOR_HPP

#define ASSIGNMENT2_DATABASEPROCESSOR_HPP
#include <stdio.h>
#include "CommandProcessor.hpp"
#include <filesystem>
#include "Storage.hpp"
#include <map>
namespace ECE141{

    class DatabaseProcessor: public CommandProcessor{
    public:
        DatabaseProcessor(CommandProcessor* aNext = nullptr);
        virtual ~DatabaseProcessor();
        virtual Statement*    getStatement(Tokenizer &aTokenizer);
        virtual StatusResult  interpret(const Statement &aStatement);
    };
}
#endif //ASSIGNMENT2_DATABASEPROCESSOR_HPP
