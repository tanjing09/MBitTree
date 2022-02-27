#pragma once

#ifndef  _MBITTREE_H
#define  _MBITREEE_H

#include "./OVS/TupleSpaceSearch.h"

#define MAXPACKETS 2000000 //to avoid segmentation fault, please change stack size to 81920 or larger, run "ulimit -s 81920"
#define MaxBits    4

class range  
{
public:
   unsigned low;
   unsigned high;
};

struct TreeNode
{
    int depth;
    bool isleaf;

    std::vector<Rule> classifier;
    int nrules = 0;

    //max number bit
    int sdimen[MaxBits] = {-1, -1, -1, -1};      //select dimension
    int sbits[MaxBits]  = {-1, -1, -1, -1};      //select bits
    int bit_num = 0;
    int ncuts = 1;              //number of cuts
    int* child;
};

class MBitTree : public PacketClassifier
{
    public:
        MBitTree(int K1, std::vector<Rule>& classifier);
        ~MBitTree();
        virtual void ConstructClassifier(const std::vector<Rule>& rules);
        virtual int ClassifyAPacket(const Packet& packet);
        virtual void DeleteRule(const Rule& delete_rule) {;};
	    virtual void InsertRule(const Rule& insert_rule) {;};
	    virtual Memory MemSizeBytes() const { return 0; } // TODO
	    virtual int MemoryAccess() const { return 0; } // TODO
	    virtual size_t NumTables() const { return 1; }
	    virtual size_t RulesInTable(size_t tableIndex) const { return rules.size(); }

        //printf private member
        void prints()
        {
		    total_bit_memory_in_KB = double(Total_Rule_Size*PTR_SIZE+Total_Array_Size*PTR_SIZE+LEAF_NODE_SIZE*Leaf_Node_Count+
                                            (TREE_NODE_SIZE+4)*NonLeaf_Node_Count)/1024;
 
		    if(numrules>0)
            {
			    if(k==1)
				    printf("************SA Subset Tree************\n");
			    else if(k==2)
				    printf("************DA Subset Tree************\n");
                else if(k==3)
                    printf("************SA+DA Subset Tree************\n");
			    printf("\tnumber of rules: %d\n",numrules);
                printf("\tnumber of rules in leafnode: %d\n", Total_Rule_Size);
                printf("\tnumber of leafnode: %d\n", Leaf_Node_Count);
                printf("\tnumber of non-leaf node: %d\n", NonLeaf_Node_Count);
                printf("\tnumber of child node pointer: %d\n", Total_Array_Size);            
                printf("\taverage  tree level: %.2f\n",ave_level/(double)Leaf_Node_Count);
	     // printf("\tworst case tree level: %d\n",pass);
             // printf("\tworst case tree depth (level+binth): %d\n", wst_depth);
                printf("\taverage tree depth (level+binth): %.1f\n", aver_depth/(double)Leaf_Node_Count);
			    printf("\ttotal memory: %f(KB)\n",total_bit_memory_in_KB);
		    }
	    }

    unsigned int TablesQueried()
    {
	    return querycount[0];
    }
    double MemoryQueried()
    {
        return total_bit_memory_in_KB;
    } 


    private:
        std::vector<Rule> rules;
        TreeNode *nodeSet;         //base of array of nodeitem
        int k;                      //0:(0,0), 1:(24,0); 2:(0,24), 3:(24,24) 

        std::queue<int> qnode;      //queue for node
        int numrules = 0;
        int root = 1;
        int binth = 8;              //at most 8 rules in leafnode
        int pass;                   //max trie level
        int ave_level;
        int aver_depth;
        int wst_depth;
        int freelist;               //first nodeItem in  freelist

        unsigned int querycount[2] = {0,0};

        int Total_Rule_Size;        //number of rules in leafnode
        int Total_Array_Size;       //number of tree node
        int Leaf_Node_Count;        //number of leafnode
        int NonLeaf_Node_Count;     //number of non-leafnode

        double total_bit_memory_in_KB; 
};

//function declare
std::vector<Rule> loadrule(FILE *fp);
std::vector<Packet> loadpacket(FILE *fp);
int BitSelect_once(TreeNode* node ,TreeNode* childnode, int t, int* num_star);
int BitSelect(TreeNode* node, int binth);

int max(int a, int b)  {return (a>b ? a:b);}
int max4_id(int a, int b, int c, int d)
{
    int max;
    if((a >=b) && (a >=c) && (a >=d))
        return 0;
    else if((b >=a) && (b >=c) && (b >=d))
        return 1;
    else if((c >=a) && (c >=b) && (c >=d))
        return 2;
    else
        return 3;        
}

int max8_id(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    if((a0 >= a1) && (a0 >= a2) && (a0 >= a3) && (a0 >= a4) && (a0 >= a5) && (a0 >= a6) && (a0 >= a7))
        return 0;
    else if ((a1 >= a0) && (a1 >= a2) && (a1 >= a3) && (a1 >= a4) && (a1 >= a5) && (a1 >= a6) && (a1 >= a7))
        return 1;
    else if ((a2 >= a0) && (a2 >= a1) && (a2 >= a3) && (a2 >= a4) && (a2 >= a5) && (a2 >= a6) && (a2 >= a7))
        return 2;
    else if ((a3 >= a0) && (a3 >= a1) && (a3 >= a2) && (a3 >= a4) && (a3 >= a5) && (a3 >= a6) && (a3 >= a7))
        return 3;
    else if ((a4 >= a0) && (a4 >= a1) && (a4 >= a2) && (a4 >= a3) && (a4 >= a5) && (a4 >= a6) && (a4 >= a7))
        return 4;
    else if ((a5 >= a0) && (a5 >= a1) && (a5 >= a2) && (a5 >= a3) && (a5 >= a4) && (a5 >= a6) && (a5 >= a7))
        return 5;
    else if ((a6 >= a0) && (a6 >= a1) && (a6 >= a2) && (a6 >= a3) && (a6 >= a4) && (a6 >= a5) && (a6 >= a7))
        return 6;
    else
        return 7;    
}

#endif
