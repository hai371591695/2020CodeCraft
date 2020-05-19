#include <bits/stdc++.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>

using namespace std;
using namespace chrono;
#define TEST


#define MaxNodeNum 2097152
#define HashBucketNum 262144
#define TaskSize 1024
#define Max_St 4
#define ThreadsNum 4
typedef unsigned int ui;
struct Path{
    uint32_t path[2];
    uint32_t money[2];
    bool operator<(const Path&rhs)const{
        if(path[0]!=rhs.path[0])
            return path[0]<rhs.path[0];
        return path[1]<rhs.path[1];

    }
};
struct Edge{
    uint32_t edge;
    uint32_t money;
    bool operator<(const Edge&rhs)const{
        return edge<rhs.edge;
    }
};

struct COPY16{
    char t[16];
};
struct COPY8{
    char t[8];
};
inline void Memcpy(void *des,const void *src){
    *(COPY16*)des = *(COPY16*)src;
}

struct Hash_node{
    uint32_t key;
    uint32_t value;
};
struct Node_D{
    uint32_t num;
    int indegree;
    int outdegree;
};
struct NodeInfor{    //输入时通过hash去重唯一记录各个节点的 长度、字符串信息。用于构造答案使用。
    char str[12];
    uint32_t num;
    int len;
    NodeInfor() {}
    NodeInfor(uint32_t a, int b, char* c) :num(a), len(b){memcpy(str,c,len); }
    bool operator<(const NodeInfor&rhs)const{
        return num<rhs.num;
    }
};






struct ARR{
    uint32_t *data;
    int capality;
    int size;
    ARR(){
        capality = 0;
       // data = (uint32_t*)malloc(capality*4);
        size = 0;
    }
    inline void expand(){
        if (capality==0){
            capality = 32;
            data = (uint32_t*)malloc(capality*4);
        }
    }
    inline void put4(uint32_t num0,uint32_t num1,uint32_t num2,uint32_t num3){
        if (size==capality){
            capality*=4;
            uint32_t *p = (uint32_t*)malloc(capality*4);
            memcpy(p,data,size*4);
            free(data);
            data = p;
        }
        data[size++] = num0;
        data[size++] = num1;
        data[size++] = num2;
        data[size++] = num3;
    }
    inline void clear(){
        size = 0;
    }

};

struct MyHash{
    //输入通过该hash去重，uint32_t nodes[4194304] 统计各个线程接收的无重复节点
    //BucketsFisrt  BucketsSecond 记录各个节点入度出度
    vector<Node_D> BucketsSecond[HashBucketNum];//作为备用，映射后碰撞的情况用vector存储。
    Node_D BucketsFisrt[HashBucketNum];      //设置适当的桶数，可以很大提高该数组命中率
    bool BucketsGeshu[HashBucketNum]={0};     //存储时使用，false时存入First..true存入Second


    NodeInfor outNodes[MaxNodeNum];//用于唯一记录节点的字符串信息（数字，字符串形式，长度）
    uint32_t nodes[4194304];       //各个线程读入的转账记录（u,v,m）。
    int outNodesCnt = 0;           //输出节点计数（每个线程节点唯一记录，不重复）
    int nodesCnt = 0;              //转账记录计数
    inline bool put(uint32_t num,int in,int out){//根据转账记录队各个统计各个几点出度入度
        int key = num%HashBucketNum;

        if (!BucketsGeshu[key]){
            BucketsFisrt[key] = Node_D{num,in,out};
            BucketsGeshu[key] = true;
            return true;
        }
        if (BucketsFisrt[key].num ==num){
            BucketsFisrt[key].indegree += in;
            BucketsFisrt[key].outdegree += out;
            return false;
        }
        for(Node_D &item:BucketsSecond[key]){
            if (item.num==num){
                item.indegree+=in;
                item.outdegree+=out;
                return false;
            }
        }
        BucketsSecond[key].emplace_back(Node_D{num,in,out});
        return true;
    }
    inline Node_D get(uint32_t num){
        int key = num%HashBucketNum;

        if (BucketsFisrt[key].num==num){
            return BucketsFisrt[key];
        }
        for(Node_D &item:BucketsSecond[key]){
            if (item.num==num)
                return item;
        }
    }

    void readData(char* buff,int st,int ed){
        int nlen=0;
        uint32_t num=0;
        char* buf = buff+st;
        char* buf_end = buff+ed;

        while (buf<buf_end){
            num = 0;nlen=0;
            while (*buf!=','){
                num*=10;
                num+=(*buf)-'0';
                nlen ++;
                buf++;
            }
            if (put(num,0,1)){
                outNodes[outNodesCnt++] =NodeInfor{num,nlen+1,buf-nlen};
            }
            nodes[nodesCnt++]=num;
            num = 0;nlen = 0;buf++;
            while (*buf!=','){
                num*=10;
                num+=(*buf)-'0';
                nlen ++;
                buf++;
            }
            if (put(num,1,0)){
                outNodes[outNodesCnt++] =NodeInfor{num,nlen+1,buf-nlen};
            }
            nodes[nodesCnt++]=num;
            num = 0;buf++;
            while (*buf!='\n' && *buf!='\r'){
                num*=10;
                num+=(*buf)-'0';
                buf++;
            }
            nodes[nodesCnt++]=num;
            while (*buf++!='\n');
        }

    }
}Read_Task0,Read_Task[ThreadsNum-1];

//多线程读取
void read_task(int pid,char* buff,int st,int ed){
    if (pid==0)
        Read_Task0.readData(buff,st,ed);
    else
        Read_Task[pid-1].readData(buff,st,ed);
}


struct IDS{
//简单的hash映射
    vector<Hash_node> BucketsSecond[HashBucketNum];
    Hash_node BucketsFisrt[HashBucketNum];
    bool BucketsGeshu[HashBucketNum]={0};

    inline void idsPut(uint32_t key,uint32_t value){
        int key_hash = key%HashBucketNum;

        if (!BucketsGeshu[key_hash]){
            BucketsFisrt[key_hash] = Hash_node{key,value};
            BucketsGeshu[key_hash] = true;
        }else{
            for(Hash_node &item:BucketsSecond[key_hash]){
                if (item.key==key){
                    return ;
                }
            }
            BucketsSecond[key_hash].emplace_back(Hash_node{key,value});
        }

    }
    inline uint32_t idsmap(uint32_t key){
        int key_hash = key%HashBucketNum;
        if (BucketsFisrt[key_hash].key==key){
            return BucketsFisrt[key_hash].value;
        }
        for(Hash_node &item:BucketsSecond[key_hash]){
            if (item.key==key){
                return item.value;
            }
        }
    }
}ids;


struct GVec{
//GVec 用于记录  head 邻居的位置 起始：s_index，结束 e_index
    int s_index;
    int e_index;
};

NodeInfor outNodes[MaxNodeNum];       //输出信息数组
NodeInfor outNodesMerge[MaxNodeNum];  //输出信息归并暂存
int nodeCnt=0;

pair<uint32_t, uint32_t> forG[2000000];     //正向图
pair<uint32_t, uint32_t> revG[2000000];     //反向图
GVec forGIndex[MaxNodeNum];           //正向图索引
GVec revGIndex[MaxNodeNum];           //正向图索引



//双向构图节点位置
int forG1[MaxNodeNum];
int revG1[MaxNodeNum];
int forG2[MaxNodeNum];
int revG2[MaxNodeNum];

struct Task_s{
    int len[8];    //环在ans 中的长度
    char *pans[8];
}Task[TaskSize];
mutex mt;
int CurTask = 0;  //任务调度





struct Thread_s{
    uint32_t revPaths0[MaxNodeNum][Max_St];   //1条反向路径快速存储
    ARR revPathsNum[MaxNodeNum];              //多条反向路径存储
    char revPathsGeshu[MaxNodeNum] = {0};     //反向路径标记 0:无路径   1:存在1条反向路径    2:存在多条反向路径且未排序    3：存在多条反向路径且已排序
    int deleteNode[65536];                    //存在反向路径的节点   用于清空
    int deleteNodeCnt = 0;                    //反向路径的节点计数
    int resnum = 0;                           //答案计数
    uint32_t layer[67108864];                 //bfs搜索队列

    char* pans3 ;                            //3环答案
    char* pans4 ;                            //4环答案
    char* pans5 ;
    char* pans6 ;
    char* pans7 ;
    Thread_s(){
        pans7 = (char*)malloc(1<<29);
        pans6 = (char*)malloc(1<<28);
        pans5 = (char*)malloc(1<<27);
        pans4 = (char*)malloc(1<<26);
        pans3 = (char*)malloc(1<<26);

    }

    void forwardSearch(int head){
        int node1,node2,node3,node4,revnode2,revnode1;
        pair<uint32_t, uint32_t> edge1,edge2,edge3,edge4;
        uint64_t money1,money2,money3,money4,moneyhead,moneytail;
        int layer1index=0,layer2index=0,layer3index=0;
        uint32_t * pathvec;
        int Geshu = 0;
        if (revPathsGeshu[head]!=0){//起点存在反向路径，判断3环
            Geshu = revPathsGeshu[head];
            if (Geshu==2){//存在多条路径未排序
                sort((Path*)revPathsNum[head].data,(Path*)revPathsNum[head].data+revPathsNum[head].size/4);
                revPathsGeshu[head]++;
                Geshu++;

            }
            if (Geshu==1){//只有单条反向路径
                revnode2 = revPaths0[head][0];
                revnode1 = revPaths0[head][1];
                moneyhead = revPaths0[head][2];
                moneytail = revPaths0[head][3];
                if(moneyhead<=5*moneytail &&moneytail<=3*moneyhead){
                    Memcpy(pans3,outNodes[head].str);  pans3+=outNodes[head].len;
                    Memcpy(pans3,outNodes[revnode2].str);  pans3+=outNodes[revnode2].len;
                    Memcpy(pans3,outNodes[revnode1].str);  pans3+=outNodes[revnode1].len;
                    *(pans3-1)='\n';
                    resnum ++;
                }
            }
            else{//多条路径已经排序完毕
                pathvec = revPathsNum[head].data;
                for(int i=0;i<revPathsNum[head].size;i+=4){
                    revnode2 = pathvec[i];
                    revnode1 = pathvec[i+1];
                    moneyhead = pathvec[i+2];
                    moneytail = pathvec[i+3];

                    if(moneyhead>5*moneytail ||moneytail>3*moneyhead)
                        continue;
                    Memcpy(pans3,outNodes[head].str);  pans3+=outNodes[head].len;
                    Memcpy(pans3,outNodes[revnode2].str);  pans3+=outNodes[revnode2].len;
                    Memcpy(pans3,outNodes[revnode1].str);  pans3+=outNodes[revnode1].len;
                    *(pans3-1)='\n';
                    resnum ++;
                }
            }
        }


//搜索第一层，判断是否存在4环，并将节点前缀与邻居索引入队
        for (int i1=forGIndex[head].s_index;i1<forGIndex[head].e_index;i1++){
            edge1 = forG[i1];
            node1=edge1.first;money1=edge1.second;
            if (node1<=head)
                continue;
            layer[layer1index] = node1;
            layer[layer1index+1] = money1;
            layer[layer1index+2] = forGIndex[node1].s_index;
            layer[layer1index+3] = forGIndex[node1].e_index;
            layer1index += 4;
            if (revPathsGeshu[node1]!=0){
                Geshu = revPathsGeshu[node1];
                if (Geshu==2){
                    sort((Path*)revPathsNum[node1].data,(Path*)revPathsNum[node1].data+revPathsNum[node1].size/4);
                    revPathsGeshu[node1]++;
                    Geshu++;
                }
                if (Geshu==1){
                    revnode2 = revPaths0[node1][0];
                    revnode1 = revPaths0[node1][1];
                    moneyhead = revPaths0[node1][2];
                    moneytail = revPaths0[node1][3];
                    if(moneyhead<=5*money1&&money1<=5*moneytail &&moneytail<=3*money1&&money1<=3*moneyhead){
                        Memcpy(pans4,outNodes[head].str);  pans4+=outNodes[head].len;
                        Memcpy(pans4,outNodes[node1].str);  pans4+=outNodes[node1].len;
                        Memcpy(pans4,outNodes[revnode2].str);  pans4+=outNodes[revnode2].len;
                        Memcpy(pans4,outNodes[revnode1].str); pans4+=outNodes[revnode1].len;
                        *(pans4-1)='\n';
                        resnum ++;
                    }
                }
                else{
                    char path_str[64];
                    int path_len = 0;
                    Memcpy(path_str,outNodes[head].str);  path_len+=outNodes[head].len;
                    Memcpy(path_str+path_len,outNodes[node1].str);  path_len+=outNodes[node1].len;
                    pathvec = revPathsNum[node1].data;
                    for(int i=0;i<revPathsNum[node1].size;i+=4){
                        revnode2 = pathvec[i];
                        revnode1 = pathvec[i+1];
                        moneyhead = pathvec[i+2];
                        moneytail = pathvec[i+3];
                        if(moneyhead>5*money1 ||money1>3*moneyhead)
                            continue;
                        if(money1>5*moneytail ||moneytail>3*money1)
                            continue;
                        memcpy(pans4,path_str,32);          pans4+=path_len;
                        Memcpy(pans4,outNodes[revnode2].str);  pans4+=outNodes[revnode2].len;
                        Memcpy(pans4,outNodes[revnode1].str); pans4+=outNodes[revnode1].len;
                        *(pans4-1)='\n';
                        resnum ++;
                    }
                }
            }
        }
//搜索第二层，判断是否存在5环，并将节点前缀与邻居索引入队
        layer2index = layer1index;
        for (int i2=0;i2<layer1index;i2+=4){
            node1 = layer[i2];
            money1 = layer[i2+1];
            int s1=layer[i2+2];
            int e1=layer[i2+3];
            for (int i=s1;i<e1;i++){
                edge2 = forG[i];
                node2=edge2.first;money2=edge2.second;
                if (node2<=head)
                    continue;
                if (money1>5*money2 || money2>3*money1)
                    continue;
                layer[layer2index] = node1;
                layer[layer2index+1] = node2;
                layer[layer2index+2] = money1;
                layer[layer2index+3] = money2;
                layer[layer2index+4] = forGIndex[node2].s_index;
                layer[layer2index+5] = forGIndex[node2].e_index;
                layer2index += 6;
                if (revPathsGeshu[node2]!=0){
                    Geshu = revPathsGeshu[node2];
                    if (Geshu==2){
                        sort((Path*)revPathsNum[node2].data,(Path*)revPathsNum[node2].data+revPathsNum[node2].size/4);
                        revPathsGeshu[node2]++;
                        Geshu++;
                    }

                    if (Geshu==1){
                        revnode2 = revPaths0[node2][0];
                        revnode1 = revPaths0[node2][1];
                        moneyhead = revPaths0[node2][2];
                        moneytail = revPaths0[node2][3];
                        if(node1!=revnode1&&node1!=revnode2&&moneyhead<=5*money1 &&money1<=3*moneyhead&&money2<=5*moneytail &&moneytail<=3*money2){
                            Memcpy(pans5,outNodes[head].str);  pans5+=outNodes[head].len;
                            Memcpy(pans5,outNodes[node1].str);  pans5+=outNodes[node1].len;
                            Memcpy(pans5,outNodes[node2].str);  pans5+=outNodes[node2].len;
                            Memcpy(pans5,outNodes[revnode2].str);  pans5+=outNodes[revnode2].len;
                            Memcpy(pans5,outNodes[revnode1].str);  pans5+=outNodes[revnode1].len;
                            *(pans5-1)='\n';
                            resnum ++;
                        }
                    }
                    else{
                        char path_str[64];
                        int path_len = 0;
                        Memcpy(path_str,outNodes[head].str);  path_len+=outNodes[head].len;
                        Memcpy(path_str+path_len,outNodes[node1].str);  path_len+=outNodes[node1].len;
                        Memcpy(path_str+path_len,outNodes[node2].str);  path_len+=outNodes[node2].len;
                        pathvec = revPathsNum[node2].data;
                        for(int i=0;i<revPathsNum[node2].size;i+=4){
                            revnode2 = pathvec[i];
                            revnode1 = pathvec[i+1];
                            moneyhead = pathvec[i+2];
                            moneytail = pathvec[i+3];
                            if(node1==revnode1||node1==revnode2)
                                continue;
                            if(moneyhead>5*money1 ||money1>3*moneyhead)
                                continue;
                            if(money2>5*moneytail ||moneytail>3*money2)
                                continue;
                            memcpy(pans5,path_str,48);          pans5+=path_len;
                            Memcpy(pans5,outNodes[revnode2].str);  pans5+=outNodes[revnode2].len;
                            Memcpy(pans5,outNodes[revnode1].str);  pans5+=outNodes[revnode1].len;
                            *(pans5-1)='\n';
                            resnum ++;

                        }
                    }

                }
            }
        }
//搜索第三层，判断是否存在6环，并将节点前缀与邻居索引入队
        layer3index = layer2index;
        for (int i3=layer1index;i3<layer2index;i3+=6){
            node1 = layer[i3];
            node2 = layer[i3+1];
            money1 = layer[i3+2];
            money2 = layer[i3+3];
            int s2=layer[i3+4];
            int e2=layer[i3+5];
            for (int i=s2;i<e2;i++){
                edge3 = forG[i];
                node3=edge3.first;money3=edge3.second;
                if (node3<=head || node3==node1)
                    continue;
                if (money2>5*money3 || money3>3*money2)
                    continue;
                layer[layer3index] = node1;
                layer[layer3index+1] = node2;
                layer[layer3index+2] = node3;
                layer[layer3index+3] = money1;
                layer[layer3index+4] = money2;
                layer[layer3index+5] = money3;
                layer[layer3index+6] = forGIndex[node3].s_index;
                layer[layer3index+7] = forGIndex[node3].e_index;
                layer3index += 8;
                if (revPathsGeshu[node3]!=0){
                    Geshu = revPathsGeshu[node3];
                    if (Geshu==2){
                        sort((Path*)revPathsNum[node3].data,(Path*)revPathsNum[node3].data+revPathsNum[node3].size/4);
                        revPathsGeshu[node3]++;
                        Geshu++;
                    }
                    if (Geshu==1){
                        revnode2 = revPaths0[node3][0];
                        revnode1 = revPaths0[node3][1];
                        moneyhead = revPaths0[node3][2];
                        moneytail = revPaths0[node3][3];

                        if(node1!=revnode1&&node1!=revnode2&&node2!=revnode1&&node2!=revnode2&&moneyhead<=5*money1 &&money3<=5*moneytail &&moneytail<=3*money3&&money1<=3*moneyhead){
                            Memcpy(pans6,outNodes[head].str);  pans6+=outNodes[head].len;
                            Memcpy(pans6,outNodes[node1].str);  pans6+=outNodes[node1].len;
                            Memcpy(pans6,outNodes[node2].str);  pans6+=outNodes[node2].len;
                            Memcpy(pans6,outNodes[node3].str);  pans6+=outNodes[node3].len;
                            Memcpy(pans6,outNodes[revnode2].str);  pans6+=outNodes[revnode2].len;
                            Memcpy(pans6,outNodes[revnode1].str);  pans6+=outNodes[revnode1].len;
                            *(pans6-1)='\n';
                            resnum ++;
                        }
                    }
                    else{
                        char path_str[64];
                        int path_len = 0;
                        Memcpy(path_str,outNodes[head].str);  path_len+=outNodes[head].len;
                        Memcpy(path_str+path_len,outNodes[node1].str);  path_len+=outNodes[node1].len;
                        Memcpy(path_str+path_len,outNodes[node2].str);  path_len+=outNodes[node2].len;
                        Memcpy(path_str+path_len,outNodes[node3].str);  path_len+=outNodes[node3].len;
                        pathvec = revPathsNum[node3].data;
                        for(int i=0;i<revPathsNum[node3].size;i+=4){
                            revnode2 = pathvec[i];
                            revnode1 = pathvec[i+1];
                            moneyhead = pathvec[i+2];
                            moneytail = pathvec[i+3];
                            if(node1==revnode1||node1==revnode2||node2==revnode1||node2==revnode2)
                                continue;
                            if(moneyhead>5*money1 ||money1>3*moneyhead)
                                continue;
                            if(money3>5*moneytail ||moneytail>3*money3)
                                continue;
                            memcpy(pans6,path_str,48);          pans6+=path_len;
                            Memcpy(pans6,outNodes[revnode2].str);  pans6+=outNodes[revnode2].len;
                            Memcpy(pans6,outNodes[revnode1].str);  pans6+=outNodes[revnode1].len;
                            *(pans6-1)='\n';
                            resnum ++;
                        }
                    }
                }


            }
        }
//搜索第四层，判断是否存在7环，并将节点前缀与邻居索引入队
        for (int i4=layer2index;i4<layer3index;i4+=8){
            node1 = layer[i4];
            node2 = layer[i4+1];
            node3 = layer[i4+2];
            money1 = layer[i4+3];
            money2 = layer[i4+4];
            money3 = layer[i4+5];
            int s3=layer[i4+6];
            int e3=layer[i4+7];
            for (int i=s3;i<e3;i++){
                edge4 = forG[i];
                node4=edge4.first;money4=edge4.second;
//7环判断先通过反向路径是否存在来剪枝可以加速不少
                if (revPathsGeshu[node4]!=0){
                    if (node4<=head || node4==node1||node4==node2)
                        continue;
                    if (money3>5*money4 || money4>3*money3)
                        continue;
                    Geshu = revPathsGeshu[node4];
                    if (Geshu==2){
                        sort((Path*)revPathsNum[node4].data,(Path*)revPathsNum[node4].data+revPathsNum[node4].size/4);
                        revPathsGeshu[node4]++;
                        Geshu++;
                    }
                    if (Geshu==1){
                        revnode2 = revPaths0[node4][0];
                        revnode1 = revPaths0[node4][1];
                        moneyhead = revPaths0[node4][2];
                        moneytail = revPaths0[node4][3];
                        if(node1!=revnode1&&node1!=revnode2&&node2!=revnode1&&node2!=revnode2&&node3!=revnode1&&node3!=revnode2&&moneyhead<=5*money1 &&money4<=5*moneytail &&moneytail<=3*money4&&money1<=3*moneyhead){
                            Memcpy(pans7,outNodes[head].str);  pans7+=outNodes[head].len;
                            Memcpy(pans7,outNodes[node1].str);  pans7+=outNodes[node1].len;
                            Memcpy(pans7,outNodes[node2].str);  pans7+=outNodes[node2].len;
                            Memcpy(pans7,outNodes[node3].str);  pans7+=outNodes[node3].len;
                            Memcpy(pans7,outNodes[node4].str);  pans7+=outNodes[node4].len;
                            Memcpy(pans7,outNodes[revnode2].str);  pans7+=outNodes[revnode2].len;
                            Memcpy(pans7,outNodes[revnode1].str);  pans7+=outNodes[revnode1].len;
                            *(pans7-1)='\n';
                            resnum ++ ;
                        }
                    }
                    else{
                        char path_str[64];
                        int path_len = 0;
                        Memcpy(path_str,outNodes[head].str);  path_len+=outNodes[head].len;
                        Memcpy(path_str+path_len,outNodes[node1].str);  path_len+=outNodes[node1].len;
                        Memcpy(path_str+path_len,outNodes[node2].str);  path_len+=outNodes[node2].len;
                        Memcpy(path_str+path_len,outNodes[node3].str);  path_len+=outNodes[node3].len;
                        Memcpy(path_str+path_len,outNodes[node4].str);  path_len+=outNodes[node4].len;
                        pathvec = revPathsNum[node4].data;
                        for(int i=0;i<revPathsNum[node4].size;i+=4){
                            revnode2 = pathvec[i];
                            revnode1 = pathvec[i+1];
                            moneyhead = pathvec[i+2];
                            moneytail = pathvec[i+3];

                            if(node1==revnode1||node1==revnode2||node2==revnode1||node2==revnode2||node3==revnode1||node3==revnode2){
                                continue;
                            }
                            if(moneyhead>5*money1 ||money1>3*moneyhead){
                                continue;
                            }
                            if(money4>5*moneytail ||moneytail>3*money4){
                                continue;
                            }
                            memcpy(pans7,path_str,64);          pans7+=path_len;
                            Memcpy(pans7,outNodes[revnode2].str);  pans7+=outNodes[revnode2].len;
                            Memcpy(pans7,outNodes[revnode1].str);  pans7+=outNodes[revnode1].len;
                            *(pans7-1)='\n';
                            resnum ++ ;
                        }
                    }
                }
            }
        }
    }



    void reverseSearch(int head){
//反向搜索与正向类似，主要在存反向路径
        int node1,node2,node3;
        int len1,len2,len3;
        pair<uint32_t, uint32_t> edge1,edge2,edge3;
        uint64_t money1,money2,money3;
        int layer1index=0,layer2index=0;

        for (int i1=revGIndex[head].s_index;i1<revGIndex[head].e_index;i1++){
            edge1 = revG[i1];
            node1 = edge1.first;money1 = edge1.second;
            if (node1<=head)
                continue;
            layer[layer1index] = node1;
            layer[layer1index+1] = money1;
            layer[layer1index+2] = revGIndex[node1].s_index;
            layer[layer1index+3] = revGIndex[node1].e_index;
            layer1index += 4;
        }
        layer2index = layer1index;
        for (int i2=0;i2<layer1index;i2+=4){
            node1 = layer[i2];
            money1 = layer[i2+1];
            int s1=layer[i2+2];
            int e1=layer[i2+3];
            for (int i=s1;i<e1;i++){
                edge2 = revG[i];
                node2=edge2.first;money2=edge2.second;

                if (money2>5*money1 || money1>3*money2)
                    continue;
                if (node2<=head )
                    continue;
                layer[layer2index] = node1;
                layer[layer2index+1] = node2;
                layer[layer2index+2] = money1;
                layer[layer2index+3] = money2;
                layer[layer2index+4] = revGIndex[node2].s_index;
                layer[layer2index+5] = revGIndex[node2].e_index;
                layer2index += 6;
            }

        }
        for (int i3=layer1index;i3<layer2index;i3+=6){
            node1 = layer[i3];
            node2 = layer[i3+1];
            money1 = layer[i3+2];
            money2 = layer[i3+3];
            int s2=layer[i3+4];
            int e2=layer[i3+5];
            for (int i=s2;i<e2;i++){
                edge3 = revG[i];
                node3 = edge3.first;money3 = edge3.second;
                if (money3>5*money2 || money2>3*money3)
                    continue;
                if (node3<head||node3==node1)
                    continue;
                if (revPathsGeshu[node3]==0){
//反向路径数为0条，node3为连接节点，将其加入需要情况的数组， 保存路径，并将标志置1
                    deleteNode[deleteNodeCnt++]=node3;
                    revPaths0[node3][0] = node2;
                    revPaths0[node3][1] = node1;
                    revPaths0[node3][2] = money1;
                    revPaths0[node3][3] = money3;
                    revPathsGeshu[node3] = 1;//标志置1
                }else{
//反向路不为0
                    if (revPathsGeshu[node3]==1){
                    //反向路径为1条，将一条反向路径存入动态数组中。标志置2.表示连接节点路径通过动态数组保存
                        revPathsNum[node3].clear();
                        revPathsNum[node3].expand();
                        memcpy(revPathsNum[node3].data,revPaths0[node3],Max_St*4);
                        revPathsNum[node3].size = 4;
                        revPathsGeshu[node3] = 2;
                    }
                    //放入新的路径
                    revPathsNum[node3].put4(node2,node1,money1,money3);
                }
            }
        }
    }

    void myclear(){
        //清空前一次搜索中，记录过反向路径的节点。
        for(int i=0;i<deleteNodeCnt;i++){
            revPathsGeshu[deleteNode[i]]=0;
        }
        deleteNodeCnt=0;

    }


    void solve(){
    #ifdef TEST
    #endif
        int st,ed,taskid;
        int task_len = nodeCnt/TaskSize+1;

        while(true){
            {
                lock_guard<std::mutex> lck(mt);
                if (CurTask<TaskSize){
                    taskid = CurTask++;
                }else{
                    break;
                }
            }
            st =  task_len*taskid;
            ed =  st+task_len;
            if (ed>nodeCnt)
                ed = nodeCnt;
            Task[taskid].pans[3] = pans3;
            Task[taskid].pans[4] = pans4;
            Task[taskid].pans[5] = pans5;
            Task[taskid].pans[6] = pans6;
            Task[taskid].pans[7] = pans7;

            for(int i=st;i<ed;i++){
#ifdef TEST
                if (i%2048==0){
                    cout<<i<<"/"<<nodeCnt<<endl;
                }
#endif // TEST
                myclear();//清空
                reverseSearch(i);//反向
                if (deleteNodeCnt){
                    forwardSearch(i);//正向
                }
            }
            Task[taskid].len[3] = pans3-Task[taskid].pans[3];
            Task[taskid].len[4] = pans4-Task[taskid].pans[4];
            Task[taskid].len[5] = pans5-Task[taskid].pans[5];
            Task[taskid].len[6] = pans6-Task[taskid].pans[6];
            Task[taskid].len[7] = pans7-Task[taskid].pans[7];
        }
    #ifdef TEST
   //     cout<<"TOTAL_LOOPS: "<<resnum<<endl;
    //    cout<<"cal time:"<<double(clock()-ss)/CLOCKS_PER_SEC<<endl;
    #endif

    }


}Thread[ThreadsNum];





int findLF(int len,char* buf){
    while (buf[len]!='\n'){
        len++;
    }
    return len+1;
}
void mmapInput(string &testFile){
    int fd = open(testFile.c_str(), O_RDONLY);
	int len = lseek(fd, 0, SEEK_END);
	char *buf = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    int Div = len/4;
    //多线程输入计算分割行末
    int lens[5] = {0,findLF(Div,buf),findLF(2*Div,buf),findLF(3*Div,buf),len};

    //多线程输入
    thread thread0(read_task,0,buf,lens[0],lens[1]);
    thread thread1(read_task,1,buf,lens[1],lens[2]);
    thread thread2(read_task,2,buf,lens[2],lens[3]);
    thread thread3(read_task,3,buf,lens[3],lens[4]);

    thread0.join();
    thread1.join();
    thread2.join();
    thread3.join();


#ifdef TEST
   // cout<<"record:"<<Read_Task0.nodeCnt/3<<endl;
   // printf("%d Records in Total\n",cnt);
#endif
}









//双向奇偶构图
//1个线程负责一半数据的奇数，通过正向构图
//1个线程负责一半数据的偶数，通过正向构图
//1个线程负责一半数据的奇数，通过反向构图
//1个线程负责一半数据的偶数，通过反向构图


void forwardGraphBuild0(){
    uint32_t u,v,m;
    for (int i=0;i<Read_Task0.nodesCnt;i+=3){
        if (Read_Task0.nodes[i]%2==1&&Read_Task0.nodes[i+1]%2==1)
            continue;
        u=ids.idsmap(Read_Task0.nodes[i]);v=ids.idsmap(Read_Task0.nodes[i+1]);m=Read_Task0.nodes[i+2];
        if (Read_Task0.nodes[i]%2==0)
            forG[forG1[u]++] = make_pair(v,m);
        if (Read_Task0.nodes[i+1]%2==0)
            revG[revG1[v]++] = make_pair(u,m);

    }
    for (int i=0;i<Read_Task[0].nodesCnt;i+=3){
        if (Read_Task[0].nodes[i]%2==1&&Read_Task[0].nodes[i+1]%2==1)
            continue;
        u=ids.idsmap(Read_Task[0].nodes[i]);v=ids.idsmap(Read_Task[0].nodes[i+1]);m=Read_Task[0].nodes[i+2];
        if (Read_Task[0].nodes[i]%2==0)
            forG[forG1[u]++] = make_pair(v,m);
        if (Read_Task[0].nodes[i+1]%2==0)
            revG[revG1[v]++] = make_pair(u,m);

    }
}

void forwardGraphBuild1(){
    uint32_t u,v,m;
    for (int i=0;i<Read_Task0.nodesCnt;i+=3){
        if (Read_Task0.nodes[i]%2==0&&Read_Task0.nodes[i+1]%2==0)
            continue;
        u=ids.idsmap(Read_Task0.nodes[i]);v=ids.idsmap(Read_Task0.nodes[i+1]);m=Read_Task0.nodes[i+2];
        if (Read_Task0.nodes[i]%2==1)
            forG[forG1[u]++] = make_pair(v,m);
        if (Read_Task0.nodes[i+1]%2==1)
            revG[revG1[v]++] = make_pair(u,m);

    }
    for (int i=0;i<Read_Task[0].nodesCnt;i+=3){
        if (Read_Task[0].nodes[i]%2==0&&Read_Task[0].nodes[i+1]%2==0)
            continue;
        u=ids.idsmap(Read_Task[0].nodes[i]);v=ids.idsmap(Read_Task[0].nodes[i+1]);m=Read_Task[0].nodes[i+2];
        if (Read_Task[0].nodes[i]%2==1)
            forG[forG1[u]++] = make_pair(v,m);
        if (Read_Task[0].nodes[i+1]%2==1)
            revG[revG1[v]++] = make_pair(u,m);

    }

}








void reverseGraphBuild0(){
    uint32_t u,v,m;
    for (int i=0;i<Read_Task[1].nodesCnt;i+=3){
        if (Read_Task[1].nodes[i]%2==1&&Read_Task[1].nodes[i+1]%2==1)
            continue;
        u=ids.idsmap(Read_Task[1].nodes[i]);v=ids.idsmap(Read_Task[1].nodes[i+1]);m=Read_Task[1].nodes[i+2];
        if (Read_Task[1].nodes[i]%2==0)
            forG[--forG2[u]] = make_pair(v,m);
        if (Read_Task[1].nodes[i+1]%2==0)
            revG[--revG2[v]] = make_pair(u,m);
    }
    for (int i=0;i<Read_Task[2].nodesCnt;i+=3){
        if (Read_Task[2].nodes[i]%2==1&&Read_Task[2].nodes[i+1]%2==1)
            continue;
        u=ids.idsmap(Read_Task[2].nodes[i]);v=ids.idsmap(Read_Task[2].nodes[i+1]);m=Read_Task[2].nodes[i+2];
        if (Read_Task[2].nodes[i]%2==0)
            forG[--forG2[u]] = make_pair(v,m);
        if (Read_Task[2].nodes[i+1]%2==0)
            revG[--revG2[v]] = make_pair(u,m);
    }
}
void reverseGraphBuild1(){
    uint32_t u,v,m;
    for (int i=0;i<Read_Task[1].nodesCnt;i+=3){
        if (Read_Task[1].nodes[i]%2==0&&Read_Task[1].nodes[i+1]%2==0)
            continue;
        u=ids.idsmap(Read_Task[1].nodes[i]);v=ids.idsmap(Read_Task[1].nodes[i+1]);m=Read_Task[1].nodes[i+2];
        if (Read_Task[1].nodes[i]%2==1)
            forG[--forG2[u]] = make_pair(v,m);
        if (Read_Task[1].nodes[i+1]%2==1)
            revG[--revG2[v]] = make_pair(u,m);
    }
    for (int i=0;i<Read_Task[2].nodesCnt;i+=3){
        if (Read_Task[2].nodes[i]%2==0&&Read_Task[2].nodes[i+1]%2==0)
            continue;
        u=ids.idsmap(Read_Task[2].nodes[i]);v=ids.idsmap(Read_Task[2].nodes[i+1]);m=Read_Task[2].nodes[i+2];
        if (Read_Task[2].nodes[i]%2==1)
            forG[--forG2[u]] = make_pair(v,m);
        if (Read_Task[2].nodes[i+1]%2==1)
            revG[--revG2[v]] = make_pair(u,m);
    }
}

void merge1(int s,int m,int e){
    int i = s;
    int j = m;
    int k = s;
    while(i < m && j < e){
        if (outNodes[i].num<outNodes[j].num)
            outNodesMerge[k++] = outNodes[i++];
        else
            outNodesMerge[k++] = outNodes[j++];
    }
    memcpy(outNodesMerge+k,outNodes+i,sizeof(NodeInfor) *(m - i));
    memcpy(outNodesMerge+k,outNodes+j,sizeof(NodeInfor) *(e - j));
}
void merge2(int s,int m,int e){
    int i = s;
    int j = m;
    int k = s;
    while(i < m && j < e){
        if (outNodesMerge[i].num<outNodesMerge[j].num)
            outNodes[k++] = outNodesMerge[i++];
        else
            outNodes[k++] = outNodesMerge[j++];
    }
    memcpy(outNodes+k,outNodesMerge+i,sizeof(NodeInfor) *(m - i));
    memcpy(outNodes+k,outNodesMerge+j,sizeof(NodeInfor) *(e - j));
}

void sort_thread(int s,int e){
    sort(outNodes+s,outNodes+e);
}

void constructGraph(){
    int Cnt = Read_Task0.outNodesCnt;
    //多线程输入后，归并各个线程的输入结果，累加各个线程统计节点的出度入度。
    memcpy(outNodes,Read_Task0.outNodes,sizeof(NodeInfor)*Read_Task0.outNodesCnt);
    for(int pid=0;pid<ThreadsNum-1;pid++){
        for(int i=0;i<Read_Task[pid].outNodesCnt;i++){
            //依次拿出
            NodeInfor t = Read_Task[pid].outNodes[i];
            uint32_t num = t.num;
            Node_D item = Read_Task[pid].get(num);
            if (Read_Task0.put(item.num,item.indegree,item.outdegree)){
                outNodes[Cnt++] = t;
            }

        }
    }
    nodeCnt = Cnt;

    int div = Cnt/4;


/*
struct NodeInfor{
    char str[12];
    uint32_t num;
    int len;
};
归并完成NodeInfor outNodes[]，记录了节点的字符串，长度，数字等信息
通过对 num 排序，完成建立映射

outNodes的排序通过4个线程分段排序，然后两两归并完成
*/
    thread threada(sort_thread,0,div);
    thread threadb(sort_thread,div,2*div);
    thread threadc(sort_thread,2*div,3*div);
	thread threadd(sort_thread,3*div,Cnt);
	threada.join();
	threadb.join();
    threadc.join();
	threadd.join();


    thread threade(merge1,0,div,2*div);
    thread threadf(merge1,2*div,3*div,Cnt);
	threade.join();
	threadf.join();

    merge2(0,2*div,Cnt);
//完成0 1 2 3……连续值到节点值的映射，

//下面通过简单的hash对节点值到连续值映射，并根据输入统计的出入度，安排forGindex的其实位置和结束位置
    int forGtindex=0,revGtindex=0;
    for(int i=0;i<nodeCnt;i++){
        int num = outNodes[i].num;
        ids.idsPut(num,i);     //排序后，将outNodes[i].num 与i映射

        Node_D nodet = Read_Task0.get(num);  //得到节点num的出入度,分配forG空间


        forG1[i] = forGtindex;      //forG1记录索引起始位置，用于正向构图
        revG1[i] = revGtindex;

        forGtindex += nodet.outdegree;
        revGtindex += nodet.indegree;

        forG2[i] = forGtindex;     //forG2记录索引结束位置，用于反向构图
        revG2[i] = revGtindex;



        forGIndex[i] = GVec{forG1[i],forG2[i]}; //索引确定
        revGIndex[i] = GVec{revG1[i],revG2[i]};
    }
    //开始构图
    thread thread0(forwardGraphBuild0);
	thread thread1(reverseGraphBuild0);
	thread thread2(forwardGraphBuild1);
	thread thread3(reverseGraphBuild1);
	thread0.join();
	thread1.join();
    thread2.join();
	thread3.join();

}

void graphSort(){
/*
对每个节点的邻居排序,这里尝试过多线程+4的方式，调度的方式，已经多邻居的节点多线程归并的方式，
发现线下加速明显而线上均是负优化，单线程的方式反而最快
*/
    for (int i=0;i<nodeCnt;i++)
        sort(forG+forGIndex[i].s_index,forG+forGIndex[i].e_index);
}


void Save(const string &outputFile){
//写入文件
    FILE *fp = fopen(outputFile.c_str(), "wb");
    char buf[64];
    int idx=sprintf(buf,"%d\n",Thread[0].resnum+Thread[1].resnum+Thread[2].resnum+Thread[3].resnum);
    buf[idx]='\0';
    fwrite(buf, idx , sizeof(char), fp );



    for(int i=0;i<TaskSize;i++){
        fwrite(Task[i].pans[3],Task[i].len[3],1,fp);
    }
    for(int i=0;i<TaskSize;i++){
        fwrite(Task[i].pans[4],Task[i].len[4],1,fp);
    }
    for(int i=0;i<TaskSize;i++){
        fwrite(Task[i].pans[5],Task[i].len[5],1,fp);
    }
    for(int i=0;i<TaskSize;i++){
        fwrite(Task[i].pans[6],Task[i].len[6],1,fp);
    }
    for(int i=0;i<TaskSize;i++){
        fwrite(Task[i].pans[7],Task[i].len[7],1,fp);
    }

    fclose(fp);
}
void solve(int pid){
    Thread[pid].solve();
}


int main()
{
#ifdef TEST
//19630345
    string testFile = "/mnt/tmpfs/19630345/test_data.txt";
    string outputFile = "/mnt/tmpfs/output/result.txt";
    string answerFile = "result.txt";
    auto t0=system_clock::now();
#else
    string testFile = "/data/test_data.txt";
    string outputFile = "/projects/student/result.txt";
#endif

    mmapInput(testFile);


#ifdef TEST
    auto t1 = system_clock::now();
#endif // TEST


    constructGraph();


#ifdef TEST
    auto t2 = system_clock::now();
#endif // TEST


    graphSort();


#ifdef TEST
    auto t3 = system_clock::now();
#endif // TEST


    thread thread0(solve,0);
	thread thread1(solve,1);
	thread thread2(solve,2);
	thread thread3(solve,3);
	thread0.join();
	thread1.join();
	thread2.join();
	thread3.join();

#ifdef TEST
    auto t4 = system_clock::now();
#endif


    Save(outputFile);

#ifdef TEST
    auto t5 = system_clock::now();
    cout<<"Total_LOOPS:"<<Thread[0].resnum+Thread[1].resnum+Thread[2].resnum+Thread[3].resnum<<endl;
    auto duration0 = duration_cast<microseconds>(t1 - t0);
    auto duration1 = duration_cast<microseconds>(t2 - t1);
    auto duration2 = duration_cast<microseconds>(t3 - t2);
    auto duration3 = duration_cast<microseconds>(t4 - t3);
    auto duration4 = duration_cast<microseconds>(t5 - t4);
    auto duration5 = duration_cast<microseconds>(t5 - t0);


    cout<<"input_time:"<< double(duration0.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
    cout<<"build_graph:"<< double(duration1.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
    cout<<"sortneighbor_time:"<< double(duration2.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
    cout<<"cal-time:"<< double(duration3.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
    cout<<"write_time:"<< double(duration4.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
    cout<<"total-time:"<< double(duration5.count()) * microseconds::period::num / microseconds::period::den   << "s" << endl;
#endif
    exit(0);
    return 0;
}
