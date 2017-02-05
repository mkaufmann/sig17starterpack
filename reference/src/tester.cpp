#include "baseline/ngramer.hpp"
#include <cstdint>
#include <gtest/gtest.h>

TEST(Ngramer, SimpleTest)
{
   Ngramer ngramer;
   ngramer.add("director");
   ngramer.add("conference");
   ngramer.add("health");
   ngramer.add("health director");
   ngramer.add("one of the");
   auto q1 = ngramer.query("for the eighth year the princeton review has named nkua "
                           "university one of the best colleges in the midwest");
   ngramer.add("public health");
   auto q2 = ngramer.query("the 2013 new york state health director conference "
                           "as always it is an excellent opportunity to meet "
                           "and mingle with more than 350 of the states top "
                           "public health leaders");
   ngramer.add("directory");
   ngramer.remove("health");
   ngramer.remove("does_not_exist");
   auto q3 = ngramer.query("the public health director is a job that allows you "
                           "to enjoy all the benefits that the health field has "
                           "to offer");
   std::vector<std::string> q1Result = {"one of the"};
   ASSERT_EQ(q1, q1Result);
   std::vector<std::string> q2Result = {"health", "health director", "director", "conference", "public health"};
   ASSERT_EQ(q2, q2Result);
   std::vector<std::string> q3Result = {"public health", "health director", "director"};
   ASSERT_EQ(q3, q3Result);
}

TEST(Ngramer, Stringify)
{
   std::vector<std::string> testA = {"a", "b", "c"};
   std::vector<std::string> testB = {};
   std::vector<std::string> testC = {"a"};

   ASSERT_EQ(stringify(testA), "a|b|c");
   ASSERT_EQ(stringify(testB), "");
   ASSERT_EQ(stringify(testC), "a");
}