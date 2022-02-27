#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<list>
#include<sys/time.h>
#include<string.h>
#include<queue>

#include "MBitTree.cpp"
#include "cluster.cpp"

using namespace std;

FILE *fpr = fopen("./acl1_10k", "r");             //ruleset
FILE *fpt = fopen("./acl1_10k_trace", "r");       //packet header
int bucketSize = 8;                                // leaf threashold

map<int,int> pri_id;


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  loadrule
 *  Description:  load rules from file
 * =====================================================================================
 */
vector<Rule> loadrule(FILE *fp)
{
    unsigned int tmp;
    unsigned sip1,sip2,sip3,sip4,smask;
    unsigned dip1,dip2,dip3,dip4,dmask;
    unsigned sport1,sport2;
    unsigned dport1,dport2;
    unsigned protocal,protocol_mask;
    unsigned ht, htmask;
    int number_rule=0; //number of rules

    vector<Rule> rule;

    while(1){

        Rule r;
        std::array<Point,2> points;

        if(fscanf(fp,"@%d.%d.%d.%d/%d\t%d.%d.%d.%d/%d\t%d : %d\t%d : %d\t%x/%x\t%x/%x\n",
                  &sip1, &sip2, &sip3, &sip4, &smask, &dip1, &dip2, &dip3, &dip4, &dmask, &sport1, &sport2,
                  &dport1, &dport2,&protocal, &protocol_mask, &ht, &htmask)!= 18) break;

        if(smask == 0){
            points[0] = 0;
            points[1] = 0xFFFFFFFF;
        }else if(smask > 0 && smask <= 8){
            tmp = sip1<<24;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-smask)) - 1;
        }else if(smask > 8 && smask <= 16){
            tmp = sip1<<24; tmp += sip2<<16;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-smask)) - 1;
        }else if(smask > 16 && smask <= 24){
            tmp = sip1<<24; tmp += sip2<<16; tmp +=sip3<<8;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-smask)) - 1;
        }else if(smask > 24 && smask <= 32){
            tmp = sip1<<24; tmp += sip2<<16; tmp += sip3<<8; tmp += sip4;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-smask)) - 1;
        }else{
            printf("Src IP length exceeds 32\n");
            exit(-1);
        }
        r.range[0] = points;

        if(dmask == 0){
            points[0] = 0;
            points[1] = 0xFFFFFFFF;
        }else if(dmask > 0 && dmask <= 8){
            tmp = dip1<<24;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-dmask)) - 1;
        }else if(dmask > 8 && dmask <= 16){
            tmp = dip1<<24; tmp +=dip2<<16;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-dmask)) - 1;
        }else if(dmask > 16 && dmask <= 24){
            tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-dmask)) - 1;
        }else if(dmask > 24 && dmask <= 32){
            tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8; tmp +=dip4;
            points[0] = tmp;
            points[1] = points[0] + (1<<(32-dmask)) - 1;
        }else{
            printf("Dest IP length exceeds 32\n");
            exit(-1);
        }
        r.range[1] = points;

        points[0] = sport1;
        points[1] = sport2;
        r.range[2] = points;

        points[0] = dport1;
        points[1] = dport2;
        r.range[3] = points;

        if(protocol_mask == 0xFF){
            points[0] = protocal;
            points[1] = protocal;
            r.prefix_length[4] = 32;
        }else if(protocol_mask== 0){
            points[0] = 0;
            points[1] = 0xFF;
            r.prefix_length[4] = 24;
        }else{
            printf("Protocol mask error\n");
            exit(-1);
        }
        r.range[4] = points;

        r.prefix_length[0] = smask;
        r.prefix_length[1] = dmask;
        r.id = number_rule;
        //extend range match to perfix match
        if(r.range[2][0] == r.range[2][1])
            r.prefix_length[2] = 32;
        else if(r.range[2][0] == 0 && r.range[2][1] == 65535)
            r.prefix_length[2] = 16;
        else
            r.prefix_length[2] = CommonPrefix(r.range[2][0], r.range[2][1]);

        if(r.range[3][0] == r.range[3][1])
            r.prefix_length[3] = 32;
        else if(r.range[3][0] == 0 && r.range[3][1] == 65535)
            r.prefix_length[3] = 16;
        else
            r.prefix_length[3] = CommonPrefix(r.range[3][0], r.range[3][1]);

        rule.push_back(r);

        number_rule++;
    }

    //printf("the number of rules = %d\n", number_rule);
    int max_pri = number_rule-1;
    for(int i=0;i<number_rule;i++){
        rule[i].priority = max_pri - i;
        pri_id.insert(pair<int,int>(rule[i].priority,rule[i].id));
    }
    pri_id.insert(pair<int,int>(-1,-1));
    /* test range to prefxi
    printf("%d, %d, %d, %d \n", rule[375].prefix_length[0], rule[375].prefix_length[1], rule[375].prefix_length[2], rule[375].prefix_length[3]);
    printf("%d, %d, %d, %d \n", rule[376].prefix_length[0], rule[376].prefix_length[1], rule[376].prefix_length[2], rule[376].prefix_length[3]);
    printf("%d, %d, %d, %d \n", rule[377].prefix_length[0], rule[377].prefix_length[1], rule[377].prefix_length[2], rule[377].prefix_length[3]);
    */
    return rule;
}

unsigned int CommonPrefix(unsigned int a, unsigned int b)
{
    unsigned int temp = a ^ b;
    unsigned int count=0;
    for(count=0; count<31; count++)
    {
        if(((temp >> (31-count)) && 1) == 0)
            ;
        else
            break;
    }
    return count;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  loadpacket
 *  Description:  load packet from file
 * =====================================================================================
 */
vector<Packet> loadpacket(FILE *fp)
{
    unsigned int header[MAXDIMENSIONS];
    unsigned int proto_mask, fid;
    int number_pkt=0; //number of packets
    std::vector<Packet> packets;

    while(1){
        if(fscanf(fp,"%u %u %u %u %u %u %u\n",
                     &header[0], &header[1], &header[2], &header[3], &header[4], &proto_mask, &fid) == Null) break;
        Packet p;
        p.push_back(header[0]);
        p.push_back(header[1]);
        p.push_back(header[2]);
        p.push_back(header[3]);
        p.push_back(header[4]);
        p.push_back(fid);

        packets.push_back(p);
        number_pkt++;
    }

    return packets;
}

void parseargs(int argc, char *argv[])  
{
  int	c;
  bool	ok = 1;
  while ((c = getopt(argc, argv, "b:r:e:h")) != -1){
    switch (c) {
	case 'b':
	  bucketSize = atoi(optarg);
	  break;
	case 'r':
	  fpr = fopen(optarg, "r");
      break;
    case 'e':
      fpt = fopen(optarg, "r");
      break;
	case 'h':
	  printf("MBitTree [-b bucketSize][-r ruleset][-e trace]\n");
	  exit(1);
	  break;
	default:
	  ok = 0;
        }
     }
  
  if(bucketSize <= 0 || bucketSize > MAXBUCKETS){
    printf("bucketSize should be greater than 0 and less than %d\n", MAXBUCKETS);
    ok = 0;
     }	
  if(fpr == NULL){
    printf("can't open ruleset file\n");
    ok = 0;
     }
  if (!ok || optind < argc){
    fprintf (stderr, "MBitTree [-b bucketSize][-r ruleset][-e trace]\n");
    fprintf (stderr, "Type \"MBitTree -h\" for help\n");
    exit(1);
     }

}


int main(int argc, char* argv[])
{
    
    parseargs(argc, argv);
    vector<Rule> rule;
    vector<Packet> packets;

    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    std::chrono::duration<double,std::milli> elapsed_milliseconds;

    if(fpr == NULL || fpt == NULL)
    {
    	printf("Error: Cannot open file!\n");
        exit(-1);
    }

    rule = loadrule(fpr);
    int number_rule = rule.size();
    printf("Number of rules = %d\n", number_rule);
    fclose(fpr);

    vector<Rule> subset[4];     //subset 0-4: (0,0), (24,0), (0,24), (24,24)
    int number_subset[4] = {0,0,0,0};
    start = std::chrono::steady_clock::now();
	partition(rule, number_rule, number_subset, subset);

    end = std::chrono::steady_clock::now();
	elapsed_milliseconds = end - start;
    printf("***clustering time: %f(ms)\n", elapsed_milliseconds.count());
    printf("The max priority of subset-0, 1, 2, 3 = %d, %d, %d, %d\n", max_pri[0], max_pri[1], max_pri[2], max_pri[3]);

    printf("************Construct Tree************\n");
	start = std::chrono::steady_clock::now();

    MBitTree Tree_SA(1, subset[1]);
    MBitTree Tree_DA(2, subset[2]);
    MBitTree Tree_SA_DA(3, subset[3]);
    PriorityTupleSpaceSearch ptss;

    if(number_subset[1] > 0){
//        printf("\n------------The SA subtree------------\n");
        Tree_SA.ConstructClassifier(subset[1]);    
    }
    else    printf("subset-1 is empty!\n\n");
    if(number_subset[2] > 0){
//        printf("\n------------The DA subtree------------\n");
        Tree_DA.ConstructClassifier(subset[2]);    
    }
    else    printf("subset-2 is empty!\n\n");
    if(number_subset[3] > 0){
//        printf("\n------------The SA+DA subtree------------\n");
        Tree_SA_DA.ConstructClassifier(subset[3]); 
    }
//    else    printf("subset-3 is empty!\n\n");
    if(number_subset[0] > 0)    ptss.ConstructClassifier(subset[0]);
//    else    printf("subset-0 is empty!\n");

	end = std::chrono::steady_clock::now();
	elapsed_milliseconds = end - start;
    printf("***total processing time: %f(ms)\n", elapsed_milliseconds.count());

    
    Tree_SA.prints();
    Tree_DA.prints();
    Tree_SA_DA.prints();
    
    printf("------PTSS for big subset------\n");
    ptss.prints();
    printf("\n\ttotal memeroy for decision tree: %f(KB), %f(MB)\n", Tree_SA.MemoryQueried()+Tree_DA.MemoryQueried()+Tree_SA_DA.MemoryQueried()+ptss.MemoryQuery(),
    (Tree_SA.MemoryQueried()+Tree_DA.MemoryQueried()+Tree_SA_DA.MemoryQueried()+ptss.MemoryQuery())/1024);
    printf("\tmemeroy per rules: %f(B), %f(KB)\n", 1024*(Tree_SA.MemoryQueried()+Tree_DA.MemoryQueried()+Tree_SA_DA.MemoryQueried()+ptss.MemoryQuery())/(double)number_rule,
    (Tree_SA.MemoryQueried()+Tree_DA.MemoryQueried()+Tree_SA_DA.MemoryQueried()+ptss.MemoryQuery())/(double)number_rule);
    
    printf("**************** Classification ****************\n");
    packets = loadpacket(fpt);
    unsigned int number_pkt = packets.size();
    printf("\tnumber of packets = %u\n", number_pkt);

    int match_miss = 0;
    int match_pri = -1;
    vector<int> results;
    Packet p;
    match_pri = -1;
    results.clear();

	start = std::chrono::steady_clock::now();
    for (unsigned int i=0; i<number_pkt; i++) {
//        p = packets[i];
        match_pri = -1;
        
        if( (number_subset[3] > 0) && (match_pri < max_pri[3]) )    match_pri = max(match_pri, Tree_SA_DA.ClassifyAPacket(packets[i]));
        if( (number_subset[2] > 0) && (match_pri < max_pri[2]) )    match_pri = max(match_pri, Tree_DA.ClassifyAPacket(packets[i]));
        if( (number_subset[1] > 0) && (match_pri < max_pri[1]) )    match_pri = max(match_pri, Tree_SA.ClassifyAPacket(packets[i]));
        if( (number_subset[0] > 0) && (match_pri < max_pri[0]) )    match_pri = max(match_pri, ptss.ClassifyAPacket(packets[i]));           
//        results.push_back(pri_id[match_pri]);
    }
	end = std::chrono::steady_clock::now();
	elapsed_milliseconds = end - start;

    match_miss = 0;
    /* for test
    for(int i = 0;i < number_pkt;i++){
        if(results[i] == -1) match_miss++;
        else if(packets[i][5] < results[i]) match_miss++;
    }
    
    printf("\tRule ID of are: %d, %d, %d, %d, %d\n", results[0], results[1], results[2], results[3], results[4]);
    printf("\tRule ID of are: %d, %d, %d, %d, %d\n", results[5], results[6], results[7], results[8], results[9]);
*/
    printf("\t%d packets are classified, %d of them are misclassified\n", number_pkt, match_miss);

    unsigned int total_query = Tree_SA.TablesQueried()+Tree_DA.TablesQueried()+Tree_SA_DA.TablesQueried()+ptss.TablesQueried();
    printf("\tTotal memory access: %u\n", total_query);
    printf("\tAverage memory access: %lf\n\n", total_query /  (double)packets.size());
    printf("\tTotal classification time : %f(ms)\n", elapsed_milliseconds.count());
    printf("\tAverage classification time : %f(us)\n", 1000*elapsed_milliseconds.count()/(double)packets.size());
    printf("\tThroughput is : %f(MPPS)\n", 1/(1000*elapsed_milliseconds.count()/(double)packets.size()));
    
   return 0;

}
