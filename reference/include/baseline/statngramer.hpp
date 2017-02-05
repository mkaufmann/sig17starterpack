#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename Processor> struct StatNgramer {
   Processor processor;
   uint64_t countDocs;
   uint64_t wordInDocs;
   uint64_t matchesDocs;
   uint64_t countAdd;
   uint64_t countRemove;
   std::unordered_set<std::string> searchNgrams;
   std::unordered_map<std::string, uint32_t> docWords;

   StatNgramer()
      : processor(), countDocs(0), wordInDocs(0), matchesDocs(0), countAdd(0), countRemove(0), searchNgrams(), docWords()
   {
   }

   std::vector<std::string> query(const std::string& document)
   {
      countDocs++;
      std::stringstream docstream(document);
      for (std::string line; std::getline(docstream, line, ' ');) {
         docWords[line]++;
         wordInDocs++;
      }
      auto result = processor.query(document);
      matchesDocs += result.size();
      return result;
   }

   void add(std::string&& ngram)
   {
      countAdd++;
      searchNgrams.insert(ngram);
      processor.add(std::move(ngram));
   }

   void remove(const std::string& ngram)
   {
      countRemove++;
      searchNgrams.insert(ngram);
      processor.remove(std::move(ngram));
   }
};
