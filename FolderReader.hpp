//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <filesystem>
#include <iostream>
namespace ECE141 {
  
  class FolderListener {
  public:
    virtual bool operator()(const std::string &aName)=0;
  };
  class testFolderListener:public FolderListener{
  public:
      virtual bool operator()(const std::string& aName){
          std::cout << aName << std::endl;
          return true;
      }
  };
  class FolderReader {
  public:
        
                  FolderReader(const char *aPath) : path(aPath) {}
                  FolderReader(std::string aPath = ""):path(aPath){}
    virtual       ~FolderReader() {}
    
    virtual bool  exists(const std::string &aPath) const{
                    //STUDENT: add logic to see if FOLDER at given path exists.
                    std::filesystem::path thepath(aPath);
                    if(std::filesystem::exists(thepath)) return true;
                    else return false;
                  }
    
    virtual void  each(FolderListener &aListener, const std::string &anExtension) const {
                    //STUDENT: iterate db's, pass the name of each to listener
                    if(!exists(path)) {
                        std::cout << "The path doesn't exist" << std::endl;
                        return;}
                    std::filesystem::path dir_path(path);
                    std::filesystem::directory_iterator file_it(dir_path);
                    for(auto temp: file_it){
                        aListener(std::string(temp.path().filename()));
                    }
                  };
      
    std::string path;
  };
  
}

#endif /* FolderReader_h */
