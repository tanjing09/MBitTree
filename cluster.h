#pragma once

#ifndef CLUSTER_H
#define CLUSTER_H

#include "MBitTree.h"

#define MAXNUMBER 100
#define K 4			//聚类数
int max_pri[4] = {-1,-1,-1,-1};

class pc_point
{
    public:
        int id;
        int mask[2];
};

//function declaration
unsigned int getDistance(Rule r1, Rule r2);
void getMean(std::vector<Rule> rule, pc_point *mean, int *center, int number_rule);
unsigned int getE(std::vector<Rule> rule, pc_point *mean, int *center, int number_rule);
void cluster(std::vector<Rule> rule, pc_point *mean, int *center, int number_rule, int *number_subset, std::vector<Rule> subset[4]);
void partition(std::vector<Rule> rule, int number_rule, int *number_subset, std::vector<Rule> subset[4]);
unsigned int CommonPrefix(unsigned int a, unsigned int b);

/*
* transform the int type to string type if need
void tostring(unsigned num, char* string, int length, int mask);
void GetBitstring(vector<Rule> rule, vector<bit_string> bitstring, bit_count *bitcount);
*/

#endif // !CLUSTER_H
