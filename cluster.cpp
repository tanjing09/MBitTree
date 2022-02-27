#ifndef CLUSTER_CPP
#define CLUSTER_CPP

#include<iostream>
#include<stdio.h>
#include<string.h>
#include<vector>
#include<ctype.h>

#include "cluster.h"

using namespace std;

#define K 4			// number of cluster

unsigned int getDistance(Rule r1, Rule r2)
{
	unsigned int d;
	d = (r1.prefix_length[FieldSA] - r2.prefix_length[FieldSA]) * (r1.prefix_length[FieldSA] - r2.prefix_length[FieldSA]) + 
			(r1.prefix_length[FieldDA] - r2.prefix_length[FieldDA]) * (r1.prefix_length[FieldDA] - r2.prefix_length[FieldDA]);
	return d;
}
 
void getMean(vector<Rule> rule, pc_point *mean, int *center, int number_rule)
{
	pc_point temp;
	int i, j, count = 0;
	for(i = 0; i < K; i++)
	{
		count = 0;
		temp.mask[0] = 0;   
		temp.mask[1] = 0;
        for(j = 0; j < number_rule; j++)
		{
			if(i == center[j])
			{
				count++;
				temp.mask[0] += rule[j].prefix_length[0];
				temp.mask[1] += rule[j].prefix_length[1];
			}
		}
		if(count == 0)
		;
		else
		{			
			temp.mask[0] /= count;
			temp.mask[1] /= count;
			mean[i].mask[0] = temp.mask[0];
			mean[i].mask[1] = temp.mask[1];
		}

	}
	for(i = 0; i < K; ++i)
    {
    	printf("The new center point of cluster-%d is : \t( %d, %d )\n", i, mean[i].mask[0], mean[i].mask[1]);
    }
}
 
unsigned int getE(vector<Rule> rule, pc_point *mean, int *center, int number_rule)
{
	int i, j;
	int cnt = 0, sum = 0;
	for(i = 0; i < K; i++)
	{
		for(j = 0; j < number_rule; j++)
		{
			if(i == center[j])
			{
				cnt = (rule[j].prefix_length[0] - mean[i].mask[0]) * (rule[j].prefix_length[0] - mean[i].mask[0]) + 
						(rule[j].prefix_length[1] - mean[i].mask[1]) * (rule[j].prefix_length[1] - mean[i].mask[1]);
				sum += cnt;
			}
		}
	}
	return sum;
}
 
void cluster(vector<Rule> rule, pc_point *mean, int *center, int number_rule, int *number_subset, vector<Rule> subset[4])
{
	int i, j, q;
	int min;
	int distance[number_rule][K];
	int temp_number_subset[K] = {0,0,0,0};
    for(int i=0; i<4; i++)  max_pri[i]=-1;
	for(i = 0; i < number_rule; i++)
	{
		min = 2048;
		for(j = 0; j < K; j++)
		{
			distance[i][j] = (rule[i].prefix_length[0]-mean[j].mask[0])*(rule[i].prefix_length[0]-mean[j].mask[0])
                                + (rule[i].prefix_length[1]-mean[j].mask[1])*(rule[i].prefix_length[1]-mean[j].mask[1]);
			/// printf("%f\n", distance[i][j]);  // the distance from the point to the center of cluster
		}
		for(q = 0; q < K; q++)
		{
			if(distance[i][q] < min)
			{
				min = distance[i][q];
        		center[i] = q;
			}
		}
		//count the number of subset
		for(q=0; q<K; q++)
		{
			if(center[i] == q)
            {
                subset[q].push_back(rule[i]);
                temp_number_subset[q]++;
                if(rule[i].priority > max_pri[q])  max_pri[q] = rule[i].priority;
				//subset[q][temp_number_subset[q]++] = rule[i];
				//subset[q][number_subset[q]++] = rule[i];
            }

		}
//		printf("( %d, %d)\t in cluster-%d\n", point[i].mask[0], point[i].mask[1], center[i]);
	}
	for(i=0; i<K; i++)
		number_subset[i] = temp_number_subset[i];
	//for test
	printf("The number of rules in subset (0,0) = %d\n", number_subset[0]);
	printf("The number of rules in subset (0,24) = %d\n", number_subset[1]);
	printf("The number of rules in subset (24,0) = %d\n", number_subset[2]);
	printf("The number of rules in subset (24,24) = %d\n", number_subset[3]);
//	printf("-----------------------------\n");

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  partition
 *  Description:  partition the ruleset based on the Mask length
 * =====================================================================================
 */
void partition(vector<Rule> rule, int number_rule, int *number_subset, vector<Rule> subset[4])
{
	int *center;  	        // what clusters that every rules belong to
	pc_point mean[K];       //  center of cluster
	center = (int*)malloc(number_rule * sizeof(int));

	int i, j, n = 0;
    int temp1;
    int temp2, t;
	// 初始化k个中心点
    mean[0] = {0,{0,0}};
    mean[1] = {1,{24,0}};
	mean[2] = {2,{0,24}};
	mean[3] = {3,{24,24}};

	printf("----------------Data sets, [%d]rules----------------\n", number_rule);
/*    
	for(i = 0; i < number_rule; ++i)
    {
    	printf("\t( %d, %d )\n", point[i].mask[0], point[i].mask[1]);
    }
    printf("-----------------------------\n");
*/
    cluster(rule, mean, center, number_rule, number_subset, subset);          
    temp1 = getE(rule, mean, center, number_rule);
    printf("The E1 is: %d\n\n", temp1); 
    getMean(rule, mean, center, number_rule);

    if((mean[0].mask[0]>=5) || (mean[0].mask[1]>=5))    {printf("\nThe total number of cluster is: 2\n");  return;}
    if((mean[1].mask[1]>=5) || (mean[2].mask[0]>=5))    {printf("\nThe total number of cluster is: 2\n");  return;}

    for(i=0; i<K; i++)
        subset[i].clear();
    n++;                   //how many times 
    
    cluster(rule, mean, center, number_rule, number_subset, subset);
    temp2 = getE(rule, mean, center, number_rule);                  //get new center
    n++;
 
    printf("The E2 is: %d\n\n", temp2);    
 
    while((temp2 - temp1) != 0)   //  when stop iteration
    {
        if((mean[0].mask[0]>=5) || (mean[0].mask[1]>=5))    break;
        if((mean[1].mask[1]>=5) || (mean[2].mask[0]>=5))    break;
    	for(i=0; i<K; i++)
            subset[i].clear();
        temp1 = temp2;
        getMean(rule, mean, center, number_rule);
    	cluster(rule, mean, center, number_rule, number_subset, subset);
    	temp2 = getE(rule, mean, center, number_rule);
    	n++;

    	printf("The E%d is: %d\n\n", n, temp2);
    }

    printf("The total number of cluster is: %d\n", n);  // Total iteration times
    printf("-------------------------------------------------\n\n");
    
    // test the partition function
    /*
	for(int i=0; i<number_subset[0]; i++)
		printf("The id of rule[%d] in subset[0]: %d\n", i, subset[0][i].id);
	*/
}

//count the number of "1" in integer type
unsigned int NumCount(unsigned int number)
{
    unsigned int count=0;
    while(number != 0)
    {
        count++;
        number = number&(number-1);
    }
    return count; 
}


#endif // !1