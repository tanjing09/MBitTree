#ifndef MBITTREE_CPP
#define MBITTREE_CPP

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "MBitTree.h"
using namespace std;

//initial object function
MBitTree::MBitTree(int k1, std::vector<Rule>& classifier)
{
    k = k1;
    this->rules = classifier;
    numrules = classifier.size();
    nodeSet = new TreeNode[MAXNODES+1];
    root = 1;           //the first tree node

    pass = 1;
    Total_Rule_Size = 0;
    Total_Array_Size = 0;
    Leaf_Node_Count = 0;
    NonLeaf_Node_Count = 0;
    total_bit_memory_in_KB = 0;
    ave_level = 0;
    wst_depth=0;
    aver_depth=0;

    //initial root node
    nodeSet[root].bit_num = 0;
    nodeSet[root].depth = 1;
    nodeSet[root].isleaf = 0;
    nodeSet[root].nrules = numrules;
    nodeSet[root].classifier = classifier;

    freelist = 2;
}

MBitTree::~MBitTree()
{
    delete [] nodeSet;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  BitSelect
 *  Description:  select a bit which has the biggest value(number_zero * number_one)
 * =====================================================================================
 */
int BitSelect_once(TreeNode* node ,TreeNode* childnode, int t)
{
	int flag = -1;	        // -1 mean fail where 1 mean success
	int bitselect = -1, bitdimen = -1;
	
	int number_one=0, number_zero=0, number_star=0, maxvalue=0;
	for(int d=0; d<5; d++){
		//computer sip
		for(int i=0; i<32; i++)
		{
			{
				int temp_zero=0, temp_one=0, temp_star=0, temp_value = 0; //just to count
				for(int j=0; j< childnode->classifier.size(); j++)
				{
					if(childnode->classifier[j].prefix_length[d] < (i+1))
						temp_star++;
					else if(((childnode->classifier[j].range[d][0] >> (31-i)) & 1 ) == 1)
						temp_one++;
					else if(((childnode->classifier[j].range[d][0] >> (31-i)) & 1 ) == 0)
						temp_zero++;
				}
				temp_value = temp_one * temp_zero;
				if((temp_value > maxvalue) && ((10*temp_star) < node->classifier.size()))			//bit select condition 
				{
					bitselect = i;
					bitdimen = d;
					maxvalue = temp_value;
					number_one = temp_one;
					number_zero = temp_zero;
					number_star = temp_star;
				}
			}
		}
	}
		if(bitselect != -1)
		{
			node->sbits[t] = bitselect;
			node->sdimen[t] = bitdimen;
            node->ncuts *= 2;
            node->bit_num++;
			flag = 1;
		}

	return flag;
}

int BitSelect(TreeNode* node, int binth)
{
	int flag = -1;
    int num_star[4] = {0, 0, 0, 0};
	TreeNode child_1[2];
	TreeNode child_2[4];
	TreeNode child_3[8];
//    node->sbits[0] = node->sbits[1] = node->sbits[2] = node->sbits[3] = -1;
//    node->sdimen[0] = node->sdimen[1] = node->sdimen[2] = node->sdimen[3] = -1;

    //the first bit select process
	flag = BitSelect_once(node, node, 0);
	if(flag == -1) 
		return -1;

    //the second bitselect process
 	for(int i=0; i < node->classifier.size(); i++)
	{
		if(((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 1)
			child_1[1].classifier.push_back(node->classifier[i]);
		else if(((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 0)
			child_1[0].classifier.push_back(node->classifier[i]);		
	}

    if((child_1[0].classifier.size() <= binth) && (child_1[1].classifier.size() <= binth))
    {
        return 0;
    }
//    printf("number of child_1 = %lu, %lu\n", child_1[0].classifier.size(), child_1[1].classifier.size());

    int num_child_1 = ((child_1[0].classifier.size() > child_1[1].classifier.size()) ? 0:1);
  	flag = BitSelect_once(node, &child_1[num_child_1], 1);

    if(flag == -1)
    {
        return -1;
    } 

    //the third select
    for(int i=0; i< node->classifier.size(); i++)
    {
        if( (((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 0) &&
                (((node->classifier[i].range[node->sdimen[1]][0] >> (31-node->sbits[1])) & 1 ) == 0) )
			child_2[0].classifier.push_back(node->classifier[i]);
        else if( (((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 1) &&
                (((node->classifier[i].range[node->sdimen[1]][0] >> (31-node->sbits[1])) & 1 ) == 0) )
			child_2[1].classifier.push_back(node->classifier[i]);
        else if( (((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 0) &&
                (((node->classifier[i].range[node->sdimen[1]][0] >> (31-node->sbits[1])) & 1 ) == 1) )
			child_2[2].classifier.push_back(node->classifier[i]);
        else if( (((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) == 1) &&
                (((node->classifier[i].range[node->sdimen[1]][0] >> (31-node->sbits[1])) & 1 ) == 1))
			child_2[3].classifier.push_back(node->classifier[i]);
    }

//    printf("number of child_2 = %lu, %lu, %lu, %lu\n", child_2[0].classifier.size(), child_2[1].classifier.size(), 
//                               child_2[2].classifier.size(), child_2[3].classifier.size());

    int num_child_2 = max4_id(child_2[0].classifier.size(), child_2[1].classifier.size(), 
                                child_2[2].classifier.size(), child_2[3].classifier.size());    
    //judge if nedd the third bit select
    if(child_2[num_child_2].classifier.size() <= binth)
        return 0;   

    flag = BitSelect_once(node, &child_2[num_child_2], 2);
    if(flag == -1)
        return -1;    
	
    
    //the fourth bit selecet process
    for(int i=0; i<node->classifier.size(); i++){
        int cnode = ((node->classifier[i].range[node->sdimen[0]][0] >> (31-node->sbits[0])) & 1 ) +
                      ((node->classifier[i].range[node->sdimen[1]][0] >> (31-node->sbits[1])) & 1 )*2 +
                      ((node->classifier[i].range[node->sdimen[2]][0] >> (31-node->sbits[2])) & 1 )*4  ;
        child_3[cnode].classifier.push_back(node->classifier[i]);
    }

//    printf("num of child_3 = %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu\n", child_3[0].classifier.size(), child_3[1].classifier.size(),
//            child_3[2].classifier.size(), child_3[3].classifier.size(), child_3[4].classifier.size(), child_3[5].classifier.size(),
//           child_3[6].classifier.size(), child_3[7].classifier.size()); 

    int num_child_3 = max8_id(child_3[0].classifier.size(), child_3[1].classifier.size(),
            child_3[2].classifier.size(), child_3[3].classifier.size(), child_3[4].classifier.size(), child_3[5].classifier.size(),
            child_3[6].classifier.size(), child_3[7].classifier.size());
    
    if(child_3[num_child_3].classifier.size() <= binth)
    {
        return 0;
    }
    
    flag = BitSelect_once(node, &child_3[num_child_3], 3);
    
	//for test
    
//    printf("Bit selected = %d, %d, %d, %d,\tnum_star = %d, %d, %d, %d,\tdimension = %d,%d,%d,%d\n", node->sbits[0], node->sbits[1], node->sbits[2], node->sbits[3],
//             num_star[0], num_star[1], num_star[2], num_star[3], node->sdimen[0], node->sdimen[1], node->sdimen[2], node->sdimen[3]);
             
	return flag;
}


void MBitTree::ConstructClassifier(const vector<Rule>& rules)
{
    int v,u;
    int flag;

    qnode.push(root);

    while(!qnode.empty())
    {
        v = qnode.front();
        qnode.pop();

        BitSelect(&nodeSet[v], binth);

        if((nodeSet[v].classifier.size() <= binth ) || (nodeSet[v].sbits[0] == -1))      //leafnode, use linear search
        {
            nodeSet[v].isleaf = 1;
            Total_Rule_Size += nodeSet[v].classifier.size();
            Leaf_Node_Count++;
            ave_level += nodeSet[v].depth;
            if(wst_depth < (nodeSet[v].depth + nodeSet[v].nrules))
                {
                    wst_depth = nodeSet[v].depth + nodeSet[v].nrules;
                }
            aver_depth += (nodeSet[v].depth + nodeSet[v].nrules);
        }

		/*
		else if(nodeSet[v].nrules >binth && flag == -1)
        {
            nodeSet[v].isleaf = 1;
            Total_Rule_Size += nodeSet[v].nrules;
            Leaf_Node_Count++;
        }
		*/

        else                        //NonLeaf
        {
            NonLeaf_Node_Count++;
            Total_Array_Size += nodeSet[v].ncuts;
            nodeSet[v].child = (int*)malloc(sizeof(int) * nodeSet[v].ncuts);
            
//            printf("node-%d bit select = %d, %d, %d, %d\tdimen = %d, %d, %d, %d\n", v, nodeSet[v].sbits[0], nodeSet[v].sbits[1], nodeSet[v].sbits[2],
//             nodeSet[v].sbits[3], nodeSet[v].sdimen[0], nodeSet[v].sdimen[1], nodeSet[v].sdimen[2], nodeSet[v].sdimen[3]);
//			printf("node-%d has %d cuts\n", v, nodeSet[v].ncuts);
            
            /* for test tree build function
            printf("rules in node-%d is: ", v);
            for(const Rule& r1: nodeSet[v].classifier)
            {
                printf(" %d (priority: %d)", r1.id, r1.priority);
            }
            printf("\n");
            */

            for(int i=0; i<nodeSet[v].ncuts; i++)
            {
                nodeSet[v].child[i] = freelist;
                u = freelist;
                freelist++;

                for(const Rule& r: nodeSet[v].classifier)       //seperate every rule to a child
                {
                    /*
                    int index = 0;
                    int cnode = 0;
                    for(int j=0; j<MaxBits; j++)
                    {
                        if(nodeSet[v].sbits[j] != -1)
                        {	
							//if(r.prefix_length[nodeSet[v].sdimen[j] ])
                            cnode += ((r.range[nodeSet[v].sdimen[j]][0] >> (31-nodeSet[v].sbits[j])) & 1) * ((int)pow(2, index));
                            index++;
                        }
                    }
                    if(cnode == i)
                        nodeSet[u].classifier.push_back(r);
                }
                */
                    //there is wildcar in biteselect
                    int cnode[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};   //store childnode count

                    //select four bits
                    if(nodeSet[v].sbits[3] != -1){
                        //no wildcard
                        if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) )
                        cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        
                        //one bit(0) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 1;                        
                        }
                        
                        //one bit(1) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 2;                        
                        }

                        //one bit(2) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 4;                        
                        }

                        //one bit(3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 8;                        
                        }

                        //two bits(0, 1) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 1;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 2;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 3;                                                                                                                                   
                        }

                        //two bits(0, 2) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 1;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 4;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 5;                                                                                                                                   
                        }

                        //two bits(0, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 1;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 8;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 9;                                                                                                                                   
                        }

                        //two bits(1, 2) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 2;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 4;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 6;                                                                                                                                   
                        }

                        //two bits(1, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 2;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 8;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 10;                                                                                                                                   
                        }

                        //two bits(2, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                        cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2;
                        cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 4;
                        cnode[2] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 8;
                        cnode[3] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 12;                                                                                                                                   
                        }

                        //three bits(0, 1, 2) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] >= (nodeSet[v].sbits[3]+1)) ){
                            cnode[0] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 ;
                            cnode[1] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 1;
                            cnode[2] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 2;
                            cnode[3] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 3;
                            cnode[4] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 4;
                            cnode[5] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 5;
                            cnode[6] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 6;
                            cnode[7] =  ((r.range[nodeSet[v].sdimen[3]][0] >> (31-nodeSet[v].sbits[3])) & 1)*8 + 7;                                                                                                                                   
                        }

                        //three bits(0, 1, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                            cnode[0] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 ;
                            cnode[1] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 1;
                            cnode[2] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 2;
                            cnode[3] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 8;
                            cnode[4] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 3;
                            cnode[5] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 9;
                            cnode[6] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 10;
                            cnode[7] =  ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 11;                                                                                                                                   
                        }

                        //three bits(0, 2, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                            cnode[0] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 ;
                            cnode[1] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 1;
                            cnode[2] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 4;
                            cnode[3] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 8;
                            cnode[4] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 5;
                            cnode[5] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 9;
                            cnode[6] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 12;
                            cnode[7] =  ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 13;                                                                                                                                   
                        }

                        //three bits(1, 2, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                            cnode[0] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) ;
                            cnode[1] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 2;
                            cnode[2] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 4;
                            cnode[3] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 8;
                            cnode[4] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 6;
                            cnode[5] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 10;
                            cnode[6] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 12;
                            cnode[7] =  ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 14;                                                                                                                                   
                        }

                        //four bits(1, 2, 3) is wildcard
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[3]] < (nodeSet[v].sbits[3]+1)) ){
                            for(int i=0; i<16; i++)
                                cnode[i] = i;                                                                                                                                   
                        }
                    } //end for sbits[3] != -1

                    //select three bits
                    else if(nodeSet[v].sbits[2] != -1){
                        if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1))  )
                        cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2;
                                cnode[1] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +4;
                            }

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                                cnode[1] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +2;
                            }

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                                cnode[1] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 +
                                    ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 +1;
                            }                            

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] >= (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4;
                                cnode[1] = ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 1;
                                cnode[2] = ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 2;
                                cnode[3] = ((r.range[nodeSet[v].sdimen[2]][0] >> (31-nodeSet[v].sbits[2])) & 1)*4 + 3;
                            }

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2;
                                cnode[1] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 1;
                                cnode[2] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 4;
                                cnode[3] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1)*2 + 5;
                            }

                        else if ( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) &&
                            (r.prefix_length[nodeSet[v].sdimen[2]] < (nodeSet[v].sbits[2]+1))  ){

                                cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1);
                                cnode[1] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 2;
                                cnode[2] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 4;
                                cnode[3] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) + 6;
                            }

                        else
                        {
                            cnode[0] = 0; cnode[1] = 1; cnode[2] = 2; cnode[3] = 3; cnode[4]=4; cnode[5]=5; cnode[6]=6; cnode[7]=7;
                        }
                                                     
                    }

                    //select two bits
                    else if(nodeSet[v].sbits[1] != -1){
                        if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1))  &&  
                            (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) )  //no wildcard
                            cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) +
                                        ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1) * 2;
                        
                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] < (nodeSet[v].sbits[0]+1))  &&  
                                (r.prefix_length[nodeSet[v].sdimen[1]] >= (nodeSet[v].sbits[1]+1)) )     //the first bit is wildcard
                            {
                            cnode[0] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1) * 2;
                            cnode[1] = ((r.range[nodeSet[v].sdimen[1]][0] >> (31-nodeSet[v].sbits[1])) & 1) * 2 + 1;
                            }

                        else if( (r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1))  &&  
                                (r.prefix_length[nodeSet[v].sdimen[1]] < (nodeSet[v].sbits[1]+1)) )
                            {
                            cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1) ;
                            cnode[1] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1)  + 2;                         
                            }
                        else{
                            cnode[0] = 0;   cnode[1] = 1;   cnode[2] = 2;   cnode[3] = 3;
                        }
                    }

                    //just select one bit
                    else if(nodeSet[v].sbits[0] != -1){
                        if(r.prefix_length[nodeSet[v].sdimen[0]] >= (nodeSet[v].sbits[0]+1))     //no wildcard
                            cnode[0] = ((r.range[nodeSet[v].sdimen[0]][0] >> (31-nodeSet[v].sbits[0])) & 1);
                        else{
                            cnode[0] = 0;
                            cnode[1] = 1;
                        }
                    }
                    
                    
                    for(int k=0; k<16; k++){
                        if(cnode[k] == i)
                            nodeSet[u].classifier.push_back(r);
                    }
                }

                //handele childnode
                nodeSet[u].nrules = nodeSet[u].classifier.size();
                if(nodeSet[u].nrules > 0)
                {
                    nodeSet[u].depth = nodeSet[v].depth+1;

                    if(nodeSet[u].nrules <= binth)          //leafnode
                    {
                        nodeSet[u].isleaf = 1;
                        Total_Rule_Size += nodeSet[u].nrules;
                        Leaf_Node_Count++;
                        ave_level += nodeSet[u].depth;
                        if(wst_depth < (nodeSet[u].depth + nodeSet[u].nrules))
                        {
                            wst_depth = nodeSet[u].depth + nodeSet[u].nrules;
                        }
                        if(pass < nodeSet[u].depth)
                            pass = nodeSet[u].depth;
                        aver_depth += nodeSet[u].depth + nodeSet[u].nrules;
                    }
                    else
                    {
                        nodeSet[u].isleaf = 0;
                        if(pass < nodeSet[u].depth)
                            pass = nodeSet[u].depth;
					
						qnode.push(u);
                    }
//					memcpy(nodeSet[u].bitFlag, nodeSet[v].bitFlag, sizeof(nodeSet[u].bitFlag));
                }
                else		//empty node
                    nodeSet[v].child[i] = Null;
//               printf("Child-%d has %d rules\n",i, nodeSet[u].nrules);
                /*
                printf("rule-id is:[ ");
                for(int i=0;i<nodeSet[u].nrules;i++)
                    printf(" %d ",nodeSet[u].classifier[i].id);
                printf("]\n");
                */
            }
        }       //loop for: nodeSet[v].ncuts        
    }   //while(!classifier.empty())
}

int MBitTree::ClassifyAPacket(const Packet& packet)
{
    int cnode = 1;
    int match_id = -1;

    while(nodeSet[cnode].isleaf != 1)
    {
                // for test
        //printf("node-%d bit select = %d, %d, %d, %d\tdimen=%d, %d, %d, %d\n", cnode, nodeSet[cnode].sbits[0],nodeSet[cnode].sbits[1],nodeSet[cnode].sbits[2],
                //nodeSet->sbits[3],nodeSet[cnode].sdimen[0], nodeSet[cnode].sdimen[1],nodeSet[cnode].sdimen[2], nodeSet[cnode].sdimen[3]);

//        int index = 0;
        int cchild=0;


        for(int i=0;i<nodeSet[cnode].bit_num;i++)
            cchild += ((packet[nodeSet[cnode].sdimen[i]] >> (31-nodeSet[cnode].sbits[i])) & 1)<<i;
        
        cnode = nodeSet[cnode].child[cchild];       //search the right child node 
        querycount[0]++;                            //count the times

        if(cnode == Null)
            break;
    }

    if((cnode != Null) && (nodeSet[cnode].isleaf == 1))         //linear serach in  leafnode 
    {
        for(int i=0; i<nodeSet[cnode].classifier.size(); i++)
        {
            querycount[0]++;        //record linear search times
//            printf("linear search time: %d\n", querycount[0]);
            if(packet[0] >= nodeSet[cnode].classifier[i].range[0][LowDim] && packet[0] <= nodeSet[cnode].classifier[i].range[0][HighDim] &&
                packet[1] >= nodeSet[cnode].classifier[i].range[1][LowDim] && packet[1] <= nodeSet[cnode].classifier[i].range[1][HighDim] &&
                packet[2] >= nodeSet[cnode].classifier[i].range[2][LowDim] && packet[2] <= nodeSet[cnode].classifier[i].range[2][HighDim] &&
                packet[3] >= nodeSet[cnode].classifier[i].range[3][LowDim] && packet[3] <= nodeSet[cnode].classifier[i].range[3][HighDim] &&
                packet[4] >= nodeSet[cnode].classifier[i].range[4][LowDim] && packet[4] <= nodeSet[cnode].classifier[i].range[4][HighDim]){
                match_id = nodeSet[cnode].classifier[i].priority;
                break;
            }
        }
    }
//    printf("packet match_id = %d\n", match_id);
    return match_id;        //return the final match rule
}

#endif // !1
