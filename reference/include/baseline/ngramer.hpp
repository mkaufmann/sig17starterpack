#pragma once

#include <string>
#include <vector>

class Ngramer {
   std::vector<std::string> ngrams;

   public:
   Ngramer() = default;

   std::vector<std::pair<std::string, size_t>> query(const std::string& document);
   void add(std::string&& ngram);
   void remove(const std::string& ngram);
};

std::vector<std::string> orderResult(std::vector<std::pair<std::string, size_t>>&& ngrams);
std::string stringify(const std::vector<std::string>& strings);
