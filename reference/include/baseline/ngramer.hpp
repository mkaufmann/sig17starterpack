#pragma once

#include <string>
#include <vector>

class Ngramer {
   std::vector<std::string> ngrams;

   public:
   Ngramer() = default;

   std::vector<std::string> query(const std::string& document);
   void add(std::string&& ngram);
   void remove(const std::string& ngram);
};

std::string stringify(const std::vector<std::string>& strings);
