/*
 * AdvanceV2版本 ---> 对应的Debug版本：AdvanceV2Debug.cpp
 *
 *  改进:
 *      1.在AdvanceV1的基础上，在对业务进行重新规划，选择通道时，优先占用连续空间最小的通道
 *      2.在AdvanceV1的基础上，BFS路径寻找失败的时候，反向寻找试一试
 *          上述两点均添加至bfsRePlanService3方法中
 * */

#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <cstdio>

using namespace std;


// 图的节点数以及边数量 (2<=N<=200  1<=M<=1000)
int N,M;
// 第i个节点的允许的最大变通道数 (1<= Pi <=20)
int Pi;
// 图中第i条边的起点，终点 ei = (ui,vi)
int ui,vi;
// 图上初始运行的业务数 (1<=J<=5000)
int J;
// 一条业务的起点，终点，经边数，路径上的边占用的通道范围[L,R]，业务价值
int Src,Snk,S,L,R,V;
// 独立测试场景数量 (1<=T<=100)
int T;
// 发生故障的边编号
int edgeFailed;
// 规划成功的业务数量
int planSuccessServices = 0;
// 规划失败的业务数量
int planFailedServices = 0;
// bfs为同一个业务的调用次数
int bfsTimes;

// 记录当前边的编号
static int CurCntEdgeId = 0;
// 最大单边通道数量
const int K = 40;
// 边初始的通道数量
const int InitChannelNums = K;
const int ZERO = 0;
// 日志文件路径
const std::string logFilePath = "../init_input.log";
const char * redirectPath1 = "../testcase/testcase1.in";
const char * redirectPath2 = "../testcase/testcase2.in";
const char * redirectPath3 = "../testcase/testcase3.in";
const char * redirectPath4 = "../inputMakeCase";
const char * outputLogPath = "../output.log";


// 记录一个测试用例下，每一个测试场景的规划成功业务数量/规划失败业务数量
class TestCaseInfo
{
public:
    TestCaseInfo():successCnt(0),failedCnt(0){}
    int TCCnt;
    int successCnt;
    int failedCnt;
};
vector<TestCaseInfo> TestCase;

// 节点类
class Node {
public:
    int id;  // 节点编号
    int channel_change_capacity;  // 变通道次数--初始化之后就不会变化
    int usedChannelCnt;           // 该节点变通道次数被使用计数

    // 构造函数
    Node(int id = 0, int capacity = 0) : id(id), channel_change_capacity(capacity) , usedChannelCnt(ZERO){}
};

// 光纤类
class Edge{
public:
    // 边的编号 冗余
    int edgeId;
    int from;
    int to;
    vector<bool> channels;  // 通道占用情况，true 表示被占用，false 表示可用
    bool is_faulty; // 边是否故障
    unordered_set<int> passServices; // 经过该边的业务ID

    int freeContinuousChannelsMaxNum;   // 空闲的连续编号最大通道宽度
    vector<pair<int,int>> freeContinuousChannelsRange;  // 空闲的连续编号通道区间

    // 构造函数
    Edge(int from = 0, int to = 0) :
            from(from), to(to),edgeId(CurCntEdgeId), is_faulty(false), freeContinuousChannelsMaxNum(InitChannelNums)
    {
        channels.resize(InitChannelNums+1, false);  // 初始化41个通道，下标0不使用
    }
};

// 业务类
class Service {
public:
    int id;
    int start;
    int end;
    int width; // 业务通道宽度
    int edge_nums;   // 经过边的数量
    vector<int> path;  // 业务路径上的边编号
    vector<pair<int, int>> channels;  // 每条边上的通道区间
    int value;  // 业务价值
    vector<int> changeChannelNodes;  // 记录当前业务在那些节点变换通道了
    bool isDie;
    vector<int> pathNodes;

    // 构造函数
    Service(int id=0,int start = 0, int end = 0, int width = 0, int value = 0):
            id(id), start(start), end(end), width(width), value(value), edge_nums(ZERO), isDie(false){}
};

// 图类
class Graph {
public:
    vector<Node> nodes;
    vector<Edge> edges;
    map<int, vector<int>> adjacency_list;
    map<int, Service> services;

    // 初始状态备份
    vector<Node> initial_nodes;
    vector<Edge> initial_edges;
    map<int, Service> initial_services;

    // 构造函数，预置空对象确保从1开始编号
    Graph();
    // 为图增加节点
    void addNode(int id, int capacity);
    // 为图增加边
    void addEdge(int from, int to);
    // 为图增加服务
    void addService(int id,int start, int end, int width, int value);
    // 初始化图
    void initialize();
    // 重置图状态到初始输入状态
    void resetGraphState();
    // 输出重新规划的业务详情
    void displayReplanServicesInfo(vector<int> &rePlanned_servicesId);

    // 标记故障边
    void markFaultyEdge(int edgeFailedId);
    // 检查通道是否可用
    bool isChannelAvailable(const Edge& edge, int channel_start, int channel_end);
    // 更新通道占用情况
    void updateChannelOccupation(Edge& edge, int channel_start, int channel_end, bool occupy);

    // 找到两个容器中相同的元素，并且返回
    vector<int> findCommonElements(const vector<int>& vec1, const vector<int>& vec2);
    // 找到两个容器中相同的元素的对应的索引
    pair<int, int> findCommonElementIndices(const vector<int>& vec1, const vector<int>& vec2, int commonElement);
    // 去除pair1,中pair1与pair2交集的部分
    pair<int, int> removeOverlap(const pair<int, int>& pair1, const pair<int, int>& pair2);
    // 将vec1中存在的vec2的元素全部去除
    void removeElements(vector<int>& vec1, const vector<int>& vec2);

    // 业务路径重新规划前，释放老路径的通道资源，以至于新路径规划时可以使用
    void bfsPlanBeforeHandOldPath(Service &service);
    // 释放业务老路径上剩余的资源
    void freeOldPathResources(map<int, Service>& oldServices);
    // 业务重新规划失败，还原业务老路径的资源占用，将规划失败的业务标记为死亡
    void restoreOldPathResources(Service &service);

    // 使用BFS重新规划路径
    bool bfsRePlanService(Service& service,map<int, Service>& oldServices);

    // 解决BFS baseLine无法处理重边的问题
    bool bfsRePlanService2(Service& service,map<int, Service>& oldServices);

    // 在bfs2的基础上，通道选择优化，以及初次失败，尝试首尾节点互换，再次搜索
    bool bfsRePlanService3(Service& service,map<int, Service>& oldServices);

    // 使用DFS重新规划路径
    bool dfsRePlanService(Service& service,map<int, Service>& oldServices);

    // 计算每一个测试场景开始时，业务总价值(对于同一个测试用例，每一个测试场景开始时，业务总价值相同，因为当一个测试场景结束后，光网络会恢复至初始状态)
    int getBeginValue();
    // 计算每一个测试场景结束后，光网络上存活的业务总价值
    int getEndValue();
    // 计算该测试场景下的得分
    int getScore(int beginValue, int endValue);
    // 计算测试用例总得分
    int getCaseSumScore(vector<int> &vec);

private:
    // 判断队列中是否已经出现了元素x（bfsRePlanService2方法内部调用）
    bool isQEleRepeat(queue<int> &q, int &neighbor);
    // 递归DFS（dfsRePlanService方法内部调用）
    bool dfs(int node, Service &service, vector<bool> &vis, vector<int> &pathNode, vector<int> &pathEdge, vector<int> &used_channels);
    // 在可处理重边的BFS算法中，得到相关状态后，使用该方法找合适路径，是对得到的状态进行DFS（bfsRePlanService2方法内部调用）
    bool findPathDfs(int node, Service &service, vector<int> &pathNode, vector<int> &pathEdge, vector<int> &pathChan,
                     vector<vector<int>> &parent, vector<vector<int>> &parentEdge, vector<vector<int>> &usedChan, int channel);
    // （findPathDfs方法内部调用）
    bool findPath(int node, Service &service, vector<int> &pathNode, vector<int> &pathEdge, vector<int> &pathChan,
                  vector<vector<int>> &parent, vector<vector<int>> &parentEdge, vector<vector<int>> &usedChan);
    // 寻找最小范围的插入通道(返回通道范围的左边界索引，为-1则没有)
    int findShortestRangeChannel(const vector<bool> &vec, int width);
};

/*********************************************类成员函数*******************************************************/
// 构造函数，预置空对象确保从1开始编号
Graph::Graph()
{
    nodes.emplace_back(); // 添加一个空的节点对象
    edges.emplace_back(); // 添加一个空的边对象
}

// 为图增加节点
void Graph::addNode(int id, int capacity)
{
    nodes.emplace_back(id, capacity);
}

// 为图增加边
void Graph::addEdge(int from, int to)
{
    edges.emplace_back(from, to);
    // 邻接表
    adjacency_list[from].push_back(CurCntEdgeId);
    adjacency_list[to].push_back(CurCntEdgeId);
}

// 为图增加服务
void Graph::addService(int id,int start, int end, int width, int value)
{
    services[id] = Service(id,start,end,width,value);
}

// 初始化图
void Graph::initialize()
{
    cin >> N >> M;  // 节点数和边数
    for (int i = 1; i <= N; i++)
    {
        cin >> Pi;
        addNode(i, Pi);
    }
    for (int i = 1; i <= M; i++)
    {
        cin >> ui >> vi;
        CurCntEdgeId++;
        addEdge(ui, vi);
    }
    // 业务数量
    cin >> J;
    for (int i = 1; i <= J; i++)
    {
        // 业务起点 终点 经边数 通道范围[L,R]--初始不存在变通道 业务价值
        cin >> Src >> Snk >> S >> L >> R >> V;
        int id = i;
        int width = ((R-L) + 1);
        addService(id, Src, Snk, width, V);
        Service& service = services[id];
        // 初始化业务经边数
        service.edge_nums = S;
        // 将起点加入
        service.pathNodes.push_back(Src);
        // 业务路径 初始化
        for (int j = 1; j <= S; j++)
        {
            int edge_id;
            cin >> edge_id;
            // 当前业务添加路径
            service.path.push_back(edge_id);
            // 当前路径中经过边，所占用的通道对
            service.channels.push_back(make_pair(L,R));
            // 记录经过edge_id边的业务  edges[edge_id].passServices.insert(id);
            edges[edge_id].passServices.insert(id);
            // 记录经过的节点
            int backNode = service.pathNodes.back();
            int otherNode = (edges[edge_id].from == backNode) ? edges[edge_id].to : edges[edge_id].from;
            service.pathNodes.push_back(otherNode);

            // 当前边被占用的通道进行标记
            for(int j = L;j<=R;j++)
            {
                edges[edge_id].channels[j] = true;
            }
            // 边edge_id最大连续通道宽度,更新
            edges[edge_id].freeContinuousChannelsMaxNum -= width;
        }
    }

    // 备份初始状态
    initial_nodes = nodes;
    initial_edges = edges;
    initial_services = services;
}

// 重置图状态到初始输入状态
void Graph::resetGraphState()
{
    nodes = initial_nodes;
    edges = initial_edges;
    services = initial_services;
    adjacency_list.clear();
    for (int i = 1; i <edges.size(); ++i)
    {
        int from = edges[i].from;
        int to = edges[i].to;
        adjacency_list[from].push_back(i);
        adjacency_list[to].push_back(i);
    }
}

// 输出重新规划的业务详情
void Graph::displayReplanServicesInfo(vector<int> &rePlanned_servicesId)
{
    cout << rePlanned_servicesId.size() << endl;
    for (const int& serviceId : rePlanned_servicesId)
    {
        cout << services[serviceId].id << " " << services[serviceId].path.size() << endl;
        for (size_t i = 0; i < services[serviceId].path.size(); ++i)
        {
            cout << services[serviceId].path[i] << " " << services[serviceId].channels[i].first << " " << services[serviceId].channels[i].second << " ";
        }
        cout << endl;
    }
}

// 标记故障边
void Graph::markFaultyEdge(int edgeFailedId)
{
    edges[edgeFailedId].is_faulty = true;
}

// 检查通道是否可用
bool Graph::isChannelAvailable(const Edge& edge, int channel_start, int channel_end)
{
    for (int i = channel_start; i <= channel_end; ++i)
    {
        if (edge.channels[i])
        {
            return false;
        }
    }
    return true;
}

// 更新通道占用情况
void Graph::updateChannelOccupation(Edge& edge, int channel_start, int channel_end, bool occupy)
{
    for (int i = channel_start; i <= channel_end; ++i)
    {
        edge.channels[i] = occupy;
    }
}

// 找到两个容器中相同的元素，并且返回
vector<int> Graph::findCommonElements(const vector<int>& vec1, const vector<int>& vec2)
{
    // 使用unordered_set存储vec1的元素以便快速查找
    unordered_set<int> elementsSet(vec1.begin(), vec1.end());
    vector<int> commonElements;

    // 遍历vec2，查找相同元素
    for (const auto& elem : vec2) {
        if (elementsSet.find(elem) != elementsSet.end()) {
            commonElements.push_back(elem);
            elementsSet.erase(elem);  // 防止重复元素被多次添加
        }
    }

    return commonElements;
}

// 找到两个容器中相同的元素的对应的索引
pair<int, int> Graph::findCommonElementIndices(const vector<int>& vec1, const vector<int>& vec2, int commonElement)
{
    std::pair<int, int> indices(-1, -1);
    // 查找元素在vec1中的索引
    auto it1 = std::find(vec1.begin(), vec1.end(), commonElement);
    if (it1 != vec1.end()) {
        indices.first = std::distance(vec1.begin(), it1);
    }

    // 查找元素在vec2中的索引
    auto it2 = std::find(vec2.begin(), vec2.end(), commonElement);
    if (it2 != vec2.end()) {
        indices.second = std::distance(vec2.begin(), it2);
    }
    return indices;
}

// 去除pair1,中pair1与pair2交集的部分
pair<int, int> Graph::removeOverlap(const pair<int, int>& pair1, const pair<int, int>& pair2)
{
    pair<int, int> result(-1,-1);

    // pair1 和 pair2 完全不重叠
    if (pair1.second < pair2.first || pair1.first > pair2.second) {
        result = pair1;
    }
        // pair1 被 pair2 完全覆盖
    else if (pair1.first >= pair2.first && pair1.second <= pair2.second) {
        // 不需要添加任何元素到 result，因为 pair1 完全被覆盖
    }
        // pair1 和 pair2 部分重叠
    else {
        if (pair1.first < pair2.first) {
            result = make_pair(pair1.first, pair2.first-1);
        }
        if (pair1.second > pair2.second) {
            result = make_pair(pair2.second+1, pair1.second);
        }
    }

    return result;
}

// 将vec1中存在的vec2的元素全部去除
void Graph::removeElements(vector<int>& vec1, const vector<int>& vec2)
{
    for (int elem : vec2) {
        // 查找并删除vec1中的elem
        vec1.erase(std::remove(vec1.begin(), vec1.end(), elem), vec1.end());
    }
}

// 释放业务老路径上剩余的资源--该批次受影响业务全部处理后
void Graph::freeOldPathResources(map<int, Service>& oldServices)
{
    for(auto &[key,oService]:oldServices)
    {
        for (int i = 0; i < oService.path.size(); ++i)
        {
            updateChannelOccupation(edges[oService.path[i]], oService.channels[i].first, oService.channels[i].second, false);
        }
        // 释放老路径的节点使用的变通道情况
        for(int i = 0;i<oService.changeChannelNodes.size();i++)
        {
            --nodes[oService.changeChannelNodes[i]].usedChannelCnt;
        }
    }
}

// 业务重新规划失败，还原业务老路径的资源占用，将规划失败的业务标记为死亡
void Graph::restoreOldPathResources(Service &service)
{
    // 还原-释放老路径通道占用情况
    for (int i = 0; i < service.path.size(); ++i)
    {
        updateChannelOccupation(edges[service.path[i]], service.channels[i].first, service.channels[i].second, true);
        // 这条业务死了，不需要将其绑定到路径上
        // edges[service.path[i]].passServices.insert(service.id);  // 该边经过的业务需要添加
    }
    // 还原-释放老路径的节点使用的变通道情况
    for(int i = 0;i<service.changeChannelNodes.size();i++)
    {
        ++nodes[service.changeChannelNodes[i]].usedChannelCnt;
    }
    service.isDie = true;  // 业务作死亡标记
}

// 使用BFS重新规划路径
bool Graph::bfsRePlanService(Service& service,map<int, Service>& oldServices)
{
    queue<int> q;
    vector<int> parent(nodes.size(), -1); // 记录路径上的父节点  记录路径上的父节点。具体来说，parent[node] 表示在到达节点 node 时，从哪个父节点过来的
    vector<int> parent_edge(nodes.size(), -1); // 记录路径上的边编号
    vector<int> used_channels(nodes.size(), -1); // 记录使用的通道编号  用于记录在到达每个节点时所使用的通道编号区间的起点
    q.push(service.start);
    parent[service.start] = service.start;
    used_channels[service.start] = service.channels[0].first; // 初始通道编号

    // 业务重新规划前，释放老路径的通道占用资源
    // 原因：对受影响业务进行重新规划时，它可以使用该业务老路径的相关资源（故障边上的资源无法使用）
    bfsPlanBeforeHandOldPath(service);

    while (!q.empty())
    {
        int node = q.front();
        q.pop();

        if (node == service.end) break; // 到达终点

        for (int edge_id : adjacency_list[node])
        {
            Edge& edge = edges[edge_id];
            if (edge.is_faulty) continue; // 跳过故障边

            int neighbor = (edge.from == node) ? edge.to : (edge.to == node) ? edge.from : -1;
            if (neighbor == -1 || parent[neighbor] != -1) continue; // 跳过已访问节点

            int current_channel = used_channels[node];

            // 判断该边的连续空闲范围 是否满足业务宽度需求
            if (isChannelAvailable(edge, current_channel, current_channel + service.width - 1))
            {
                q.push(neighbor);
                parent[neighbor] = node;
                parent_edge[neighbor] = edge_id;
                used_channels[neighbor] = current_channel; // 继续使用当前通道
            }
            else
            {
                if(node == service.start)
                {
                    for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                    {
                        if (isChannelAvailable(edge, ch, ch + service.width - 1))
                        {
                            q.push(neighbor);
                            parent[neighbor] = node;
                            parent_edge[neighbor] = edge_id;
                            used_channels[neighbor] = ch; // 使用新的通道
                            break;
                        }
                    }
                }
                else
                {
                    if ((nodes[node].channel_change_capacity - nodes[node].usedChannelCnt) > 0)
                    {
                        // 使用当前节点处的变通道次数
                        for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                        {
                            if (isChannelAvailable(edge, ch, ch + service.width - 1))
                            {
                                q.push(neighbor);
                                parent[neighbor] = node;
                                parent_edge[neighbor] = edge_id;
                                used_channels[neighbor] = ch; // 使用新的通道
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // 规划失败
    if (parent[service.end] == -1)  // 无法到达目标
    {
        // 业务重新规划失败，标记为死亡，死亡的业务占用的通道资源以及变节点次数均不会释放，之后也不会对其进行重新规划
        // 还原-在BFS算法开始之前对业务老路径释放的通道资源情况
        // 并且对该规划失败的业务标记死亡
        restoreOldPathResources(service);
        return false;
    }

    // 业务路径重新规划成功，获取新路径的资源情况
    vector<int> new_path;
    vector<pair<int, int>> new_channels;
    vector<int> newChangeChannelNodes;
    vector<int> newPathNodes;
    for (int v = service.end; v != service.start; v = parent[v])
    {
        new_path.push_back(parent_edge[v]);
        newPathNodes.push_back(v);
        new_channels.push_back(make_pair(used_channels[v], used_channels[v] + service.width - 1));
    }
    newPathNodes.push_back(service.start);
    reverse(new_path.begin(), new_path.end());
    reverse(new_channels.begin(), new_channels.end());
    reverse(newPathNodes.begin(),newPathNodes.end());

    // 比对该业务新老路径资源
    // 1.先判断是否使用了相同的边上的，共有的通道资源，若新路径使用了老路径的部分通道资源，老路径需要进行剔除
    // repeatedEdges 存储了相同边的编号
    vector<int> repeatedEdges;
    repeatedEdges = findCommonElements(new_path,service.path);
    for(int i = 0;i<repeatedEdges.size();i++)
    {
        int commonElement = repeatedEdges[i];
        // 获得老路径 与 新路径 共有边分别在各自 path中的索引值
        pair<int, int> indices = findCommonElementIndices(service.path,new_path,commonElement);
        // commonElement --> new_path in indices.first
        // commonElement --> service.path in indices.second
        if (indices.first != -1 && indices.second != -1)
        {
            pair<int,int> result = removeOverlap(service.channels[indices.first],new_channels[indices.second]);
            // 完全不重叠 or 部分重叠
            if(result.first != -1 && result.second != -1)
            {
                // result 即 该业务老路径上 这条边上的 剩余资源
                service.channels[indices.first] = result;
            }
            else
            {
                // 完全覆盖---这条共有边上老路径所使用的通道资源，被新路径完全使用
                // 将这条边，在老路径中剔除
                service.path.erase(service.path.begin() + indices.first);
                service.channels.erase(service.channels.begin() + indices.first);
            }
        }
    }

    // 2.判断是否对相同的节点使用了变通道次数
    // 同时新路径上使用了变通道能力的节点，次数进行更新
    for(int i = 0;i<new_channels.size()-1;i++)
    {
        if(new_channels[i] != new_channels[i+1])
        {
            ++nodes[newPathNodes[i+1]].usedChannelCnt;
            newChangeChannelNodes.push_back(newPathNodes[i+1]);
        }
    }
    // 判断新老路径上是否对同一个节点使用了变通道的次数
    // 若新路径使用了老路径changeChannelNodes中的节点的变通道次数，老路径changeChannelNodes中需要将其剔除
    vector<int> repeatedChangeChannelNodes;
    repeatedChangeChannelNodes = findCommonElements(newChangeChannelNodes,service.changeChannelNodes);
    // 将service.changeChannelNodes 中 被新路径使用的变通道节点能力的节点去除
    if(repeatedChangeChannelNodes.size() != 0)
    {
        removeElements(service.changeChannelNodes,repeatedChangeChannelNodes);
    }

    // 将该业务老路径剩余的资源进行占用
    oldServices[service.id] = Service(service.id,service.start,service.end,service.width,service.value);
    oldServices[service.id].path = service.path;
    oldServices[service.id].channels = service.channels;
    oldServices[service.id].changeChannelNodes = service.changeChannelNodes;
    oldServices[service.id].edge_nums = service.path.size();
    // 将老路径剩余资源占用
    for (int i = 0; i < oldServices[service.id].path.size(); ++i)
    {
        updateChannelOccupation(edges[oldServices[service.id].path[i]], oldServices[service.id].channels[i].first,
                                oldServices[service.id].channels[i].second, true); // 占用的通道资源
    }
    // 将老路径的剩余节点使用的变通道情况进行占用
    for(int i = 0;i<oldServices[service.id].changeChannelNodes.size();i++)
    {
        ++nodes[oldServices[service.id].changeChannelNodes[i]].usedChannelCnt;
    }

    // 对重新规划的新路径通道进行占用
    for (int i = 0; i < new_path.size(); ++i)
    {
        updateChannelOccupation(edges[new_path[i]], new_channels[i].first, new_channels[i].second, true);
        edges[new_path[i]].passServices.insert(service.id);
    }


    service.path = new_path;
    service.channels = new_channels;
    // 更新路径上那些节点使用了变通道
    service.changeChannelNodes = newChangeChannelNodes;

    // 更新业务的经边数量
    service.edge_nums = new_path.size();

    return true;
}

// 使用BFS重新规划路径---可以处理重边
bool Graph::bfsRePlanService2(Service& service,map<int, Service>& oldServices)
{
    queue<int> q;
    queue<int> qAll;
    vector<vector<int>> parent(nodes.size(), vector<int>(1, -1)); // 记录路径上的父节点  记录路径上的父节点（同时也用于判断某节点是否已经访问过）。具体来说，parent[node] 表示在到达节点 node 时，从哪个父节点过来的
    vector<vector<int>> parent_edge(nodes.size(), vector<int>(1, -1)); // 记录路径上的边编号
    vector<vector<int>> used_channels(nodes.size(), vector<int>(1, -1)); // 记录使用的通道编号  用于记录在到达每个节点时所使用的通道编号区间的起点
    vector<int> vis_edge(edges.size(),-1); // 判断边是否访问

    q.push(service.start);
    qAll.push(service.start);
    parent[service.start][0] = service.start;
    used_channels[service.start][0] = service.channels[0].first; // 初始通道编号

    // 业务重新规划前，释放老路径的通道占用资源
    // 原因：对受影响业务进行重新规划时，它可以使用该业务老路径的相关资源（故障边上的资源无法使用）
    bfsPlanBeforeHandOldPath(service);

    while (!q.empty())
    {
        int node = q.front();
        q.pop();

        if (node == service.end) break; // 到达终点

        for (int edge_id : adjacency_list[node])
        {
            Edge& edge = edges[edge_id];
            if (edge.is_faulty) continue; // 跳过故障边

            int neighbor = (edge.from == node) ? edge.to : (edge.to == node) ? edge.from : -1;
            if ((neighbor == -1 || parent[neighbor][0] != -1) && (vis_edge[edge_id]!=-1)) continue; // 跳过已访问节点与边
            // 若neighbor为node的父节点，也跳过
            if(std::find(parent[node].begin(), parent[node].end(), neighbor) != parent[node].end()) continue;


            vis_edge[edge_id] = edge_id;

            int usedChanSize = used_channels[node].size();
            for(int i=0;i<usedChanSize;i++)
            {
                int current_channel = used_channels[node][i];

                // 判断该边的连续空闲范围 是否满足业务宽度需求
                if (isChannelAvailable(edge, current_channel, current_channel + service.width - 1))
                {
                    if(isQEleRepeat(qAll, neighbor) == false)
                    {
                        qAll.push(neighbor);
                        q.push(neighbor);
                    }

                    if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                        parent[neighbor][0] = node;
                    else
                        parent[neighbor].push_back(node);

                    if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                        parent_edge[neighbor][0] = edge_id;
                    else
                        parent_edge[neighbor].push_back(edge_id);

                    if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                        used_channels[neighbor][0] = current_channel;
                    else
                        used_channels[neighbor].push_back(current_channel);
                }
                else
                {
                    if(node == service.start)
                    {
                        for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                        {
                            if (isChannelAvailable(edge, ch, ch + service.width - 1))
                            {
                                if(isQEleRepeat(qAll, neighbor) == false)
                                {
                                    qAll.push(neighbor);
                                    q.push(neighbor);
                                }

                                if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                                    parent[neighbor][0] = node;
                                else
                                    parent[neighbor].push_back(node);

                                if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                                    parent_edge[neighbor][0] = edge_id;
                                else
                                    parent_edge[neighbor].push_back(edge_id);

                                // 选择新通道
                                if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                                    used_channels[neighbor][0] = ch;
                                else
                                    used_channels[neighbor].push_back(ch);
                                break;
                            }
                        }
                    }
                    else
                    {
                        if ((nodes[node].channel_change_capacity - nodes[node].usedChannelCnt) > 0)
                        {
                            // 使用当前节点处的变通道次数
                            for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                            {
                                if (isChannelAvailable(edge, ch, ch + service.width - 1))
                                {
                                    if(isQEleRepeat(qAll, neighbor) == false)
                                    {
                                        qAll.push(neighbor);
                                        q.push(neighbor);
                                    }

                                    if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                                        parent[neighbor][0] = node;
                                    else
                                        parent[neighbor].push_back(node);

                                    if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                                        parent_edge[neighbor][0] = edge_id;
                                    else
                                        parent_edge[neighbor].push_back(edge_id);

                                    // 选择新通道
                                    if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                                        used_channels[neighbor][0] = ch;
                                    else
                                        used_channels[neighbor].push_back(ch);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 规划失败
    if (parent[service.end][parent[service.end].size()-1] == -1)  // 无法到达目标
    {
        // 业务重新规划失败，标记为死亡，死亡的业务占用的通道资源以及变节点次数均不会释放，之后也不会对其进行重新规划
        // 还原-在BFS算法开始之前对业务老路径释放的通道资源情况
        // 并且对该规划失败的业务标记死亡
        restoreOldPathResources(service);
        return false;
    }


    // 新路径情况
    vector<int> new_path;
    vector<pair<int, int>> new_channels;
    vector<int> newChangeChannelNodes;
    vector<int> newPathNodes;

    vector<int> pathNodes;
    vector<int> pathEdges;
    vector<int> pathChan;
    // 找到路径了
    if(findPath(service.end, service, pathNodes, pathEdges, pathChan, parent, parent_edge, used_channels))
    {
        new_path = pathEdges;
        newPathNodes = pathNodes;
        reverse(new_path.begin(), new_path.end());
        reverse(newPathNodes.begin(), newPathNodes.end());
        for(int i = pathChan.size()-1; i>=0; i--)
        {
            new_channels.push_back(make_pair(pathChan[i], pathChan[i] + service.width - 1));
        }
    }
    else
    {
        // 没有找到路径
        // 还原-释放老路径通道占用情况
        restoreOldPathResources(service);
        return false;
    }


    // 比对该业务新老路径资源
    // 1.先判断是否使用了相同的边上的，共有的资源通道
    // repeatedEdges 存储了相同边的编号
    vector<int> repeatedEdges;
    repeatedEdges = findCommonElements(new_path,service.path);
    for(int i = 0;i<repeatedEdges.size();i++)
    {
        int commonElement = repeatedEdges[i];
        // 获得老路径 与 新路径 共有边分别在各自 path中的索引值
        pair<int, int> indices = findCommonElementIndices(service.path,new_path,commonElement);
        // commonElement --> new_path in indices.first
        // commonElement --> service.path in indices.second
        if (indices.first != -1 && indices.second != -1)
        {
            pair<int,int> result = removeOverlap(service.channels[indices.first],new_channels[indices.second]);
            // 完全不重叠 or 部分重叠
            if(result.first != -1 && result.second != -1)
            {
                // result 即 该业务老路径上 这条边上的 剩余资源
                service.channels[indices.first] = result;
            }
            else
            {
                // 完全覆盖---这条共有边上老路径所使用的通道资源，被新路径完全使用
                // 将这条边，在老路径中剔除
                service.path.erase(service.path.begin() + indices.first);
                service.channels.erase(service.channels.begin() + indices.first);
            }
        }
    }
    // 2.判断是否对相同的节点使用了变通道次数
    // 同时新路径上使用了变通道能力的节点，次数进行更新
    for(int i = 0;i<new_channels.size()-1;i++)
    {
        if(new_channels[i] != new_channels[i+1])
        {
            ++nodes[newPathNodes[i+1]].usedChannelCnt;
            newChangeChannelNodes.push_back(newPathNodes[i+1]);
        }
    }
    // 判断新老路径上是否对同一个节点使用了变通道的次数
    vector<int> repeatedChangeChannelNodes;
    repeatedChangeChannelNodes = findCommonElements(newChangeChannelNodes,service.changeChannelNodes);
    // 将service.changeChannelNodes 中 被新路径使用的变通道节点能力的节点去除
    if(repeatedChangeChannelNodes.size() != 0)
    {
        removeElements(service.changeChannelNodes,repeatedChangeChannelNodes);
    }
    // 将该业务老路径剩余的资源进行占用
    oldServices[service.id] = Service(service.id,service.start,service.end,service.width,service.value);
    oldServices[service.id].path = service.path;
    oldServices[service.id].channels = service.channels;
    oldServices[service.id].changeChannelNodes = service.changeChannelNodes;
    oldServices[service.id].edge_nums = service.path.size();
    // 将老路径剩余资源占用
    for (int i = 0; i < oldServices[service.id].path.size(); ++i)
    {
        updateChannelOccupation(edges[oldServices[service.id].path[i]], oldServices[service.id].channels[i].first,
                                oldServices[service.id].channels[i].second, true); // 占用的通道资源
    }
    // 占用老路径的剩余节点使用的变通道情况
    for(int i = 0;i<oldServices[service.id].changeChannelNodes.size();i++)
    {
        ++nodes[oldServices[service.id].changeChannelNodes[i]].usedChannelCnt;
    }

    //-------------------------------
    // 对重新规划的新路径通道进行占用
    for (int i = 0; i < new_path.size(); ++i)
    {
        updateChannelOccupation(edges[new_path[i]], new_channels[i].first, new_channels[i].second, true);
        edges[new_path[i]].passServices.insert(service.id);
    }


    service.path = new_path;
    service.channels = new_channels;
    // 更新路径上那些节点使用了变通道
    service.changeChannelNodes = newChangeChannelNodes;
    // 更新经过的节点
    service.pathNodes = newPathNodes;

    // 更新业务的经边数量
    service.edge_nums = new_path.size();


    return true;
}

// 判断队列中是否已经出现了元素x
bool Graph::isQEleRepeat(queue<int> &q, int &neighbor)
{
    queue<int> tempQ = q;
    while (!tempQ.empty())
    {
        if(tempQ.front() == neighbor)
        {
            return true;
        }
        tempQ.pop();
    }
    return false;
}

bool Graph::findPathDfs(int node, Service &service, vector<int> &pathNode,vector<int> &pathEdge, vector<int> &pathChan,
                        vector<vector<int>> &parent, vector<vector<int>> &parentEdge, vector<vector<int>> &usedChan,int channel)
{
    pathNode.push_back(node);
    if(node == service.start)
        return true;

    for(int index = 0; index < parent[node].size(); index++)
    {
        if(node == service.start)
        {
            pathEdge.push_back(parentEdge[node][index]);
            pathChan.push_back(usedChan[node][index]);
            if(findPathDfs(parent[node][index], service, pathNode,pathEdge, pathChan,parent, parentEdge, usedChan,usedChan[node][index]))
            {
                return true;
            }
        }
        else
        {
            if(channel == usedChan[node][index])
            {
                pathEdge.push_back(parentEdge[node][index]);
                pathChan.push_back(usedChan[node][index]);
                if(findPathDfs(parent[node][index], service, pathNode,pathEdge, pathChan,parent, parentEdge, usedChan,usedChan[node][index]))
                {
                    return true;
                }
            }
            else
            {
                if(nodes[node].channel_change_capacity - nodes[node].usedChannelCnt > 0)
                {
                    pathEdge.push_back(parentEdge[node][index]);
                    pathChan.push_back(usedChan[node][index]);
                    if(findPathDfs(parent[node][index], service, pathNode,pathEdge, pathChan,parent, parentEdge, usedChan,usedChan[node][index]))
                    {
                        return true;
                    }
                }
            }
        }
    }

    pathNode.pop_back();
    pathEdge.pop_back();
    pathChan.pop_back();
    return false;
}

bool Graph::findPath(int node, Service &service, vector<int> &pathNode,vector<int> &pathEdge, vector<int> &pathChan,
                     vector<vector<int>> &parent, vector<vector<int>> &parentEdge, vector<vector<int>> &usedChan)
{
    for(int index = 0; index < parent[node].size(); index++)
    {
        if(findPathDfs(node, service, pathNode, pathEdge, pathChan, parent, parentEdge, usedChan, usedChan[node][index]))
        {
            return true;
        }
    }
    return false;
}

// 使用DFS重新规划路径
bool Graph::dfsRePlanService(Service& service,map<int, Service>& oldServices)
{
    vector<bool> vis(nodes.size(), false);
    vector<int> pathNode;
    vector<int> pathEdge;
    vector<int> used_channels;
    used_channels.push_back(service.channels[0].first);

    // 业务重新规划前，释放老路径的通道占用资源
    // 原因：对受影响业务进行重新规划时，它可以使用该业务老路径的相关资源（故障边上的资源无法使用）
    bfsPlanBeforeHandOldPath(service);

    /****************规划失败****************/
    if(!dfs(service.start, service, vis, pathNode,pathEdge, used_channels))
    {
        // 业务重新规划失败，标记为死亡，死亡的业务占用的通道资源以及变节点次数均不会释放，之后也不会对其进行重新规划
        // 还原-在BFS算法开始之前对业务老路径释放的通道资源情况
        // 并且对该规划失败的业务标记死亡
        restoreOldPathResources(service);
        return false;
    }
    /****************规划失败****************/

    /****************规划成功****************/
    // 根据 pathNode pathEdge used_Channels得到新路径
    vector<pair<int, int>> new_channels;
    vector<int> newChangeChannelNodes;
    // 获取路径上的通道范围
    for(int i = 1; i < used_channels.size(); i++)
    {
        // used_channels 会将业务原来路径 起点通道也添加进入，因此从1开始
        new_channels.push_back(make_pair(used_channels[i], used_channels[i] + service.width - 1));
    }

    // 比对该业务新老路径资源
    // 1.先判断是否使用了相同的边上的，共有的资源通道
    // repeatedEdges 存储了相同边的编号
    vector<int> repeatedEdges;
    repeatedEdges = findCommonElements(pathEdge,service.path);
    for(int i = 0;i<repeatedEdges.size();i++)
    {
        int commonElement = repeatedEdges[i];
        // 获得老路径 与 新路径 共有边分别在各自 path中的索引值
        pair<int, int> indices = findCommonElementIndices(service.path,pathEdge,commonElement);
        // commonElement --> new_path in indices.first
        // commonElement --> service.path in indices.second
        if (indices.first != -1 && indices.second != -1)
        {
            pair<int,int> result = removeOverlap(service.channels[indices.first],new_channels[indices.second]);
            // 完全不重叠 or 部分重叠
            if(result.first != -1 && result.second != -1)
            {
                // result 即 该业务老路径上 这条边上的 剩余资源
                service.channels[indices.first] = result;
            }
            else
            {
                // 完全覆盖---这条共有边上老路径所使用的通道资源，被新路径完全使用
                // 将这条边，在老路径中剔除
                service.path.erase(service.path.begin() + indices.first);
                service.channels.erase(service.channels.begin() + indices.first);
            }
        }
    }
    // 2.判断是否对相同的节点使用了变通道次数
    // 同时新路径上使用了变通道能力的节点，次数进行更新
    for(int i = 0;i<new_channels.size()-1;i++)
    {
        if(new_channels[i] != new_channels[i+1])
        {
            ++nodes[pathNode[i+1]].usedChannelCnt;
            newChangeChannelNodes.push_back(pathNode[i+1]);
        }
    }

    // 判断新老路径上是否对同一个节点使用了变通道的次数
    vector<int> repeatedChangeChannelNodes;
    repeatedChangeChannelNodes = findCommonElements(newChangeChannelNodes,service.changeChannelNodes);
    // 将service.changeChannelNodes 中 被新路径使用的变通道节点能力的节点去除
    if(repeatedChangeChannelNodes.size() != 0)
    {
        removeElements(service.changeChannelNodes,repeatedChangeChannelNodes);
    }
    // 将该业务老路径剩余的资源进行占用
    oldServices[service.id] = Service(service.id,service.start,service.end,service.width,service.value);
    oldServices[service.id].path = service.path;
    oldServices[service.id].channels = service.channels;
    oldServices[service.id].changeChannelNodes = service.changeChannelNodes;
    oldServices[service.id].edge_nums = service.path.size();
    // 将老路径剩余资源占用
    for (int i = 0; i < oldServices[service.id].path.size(); ++i)
    {
        updateChannelOccupation(edges[oldServices[service.id].path[i]], oldServices[service.id].channels[i].first,
                                oldServices[service.id].channels[i].second, true); // 占用的通道资源
    }
    // 释放老路径的节点使用的变通道情况
    for(int i = 0;i<oldServices[service.id].changeChannelNodes.size();i++)
    {
        ++nodes[oldServices[service.id].changeChannelNodes[i]].usedChannelCnt;
    }

    // 对重新规划的新路径通道进行占用
    for (int i = 0; i < pathEdge.size(); ++i)
    {
        updateChannelOccupation(edges[pathEdge[i]], new_channels[i].first, new_channels[i].second, true);
        edges[pathEdge[i]].passServices.insert(service.id);
    }

    service.path = pathEdge;
    service.channels = new_channels;
    // 更新路径上那些节点使用了变通道
    service.changeChannelNodes = newChangeChannelNodes;

    // 更新业务的经边数量
    service.edge_nums = pathEdge.size();

    return true;
    /****************规划成功****************/
}

// 递归DFS
bool Graph::dfs(int node, Service &service, vector<bool> &vis, vector<int> &pathNode,
                vector<int> &pathEdge, vector<int> &used_channels)
{
    vis[node] = true;
    pathNode.push_back(node);

    // 到达终点
    if(node == service.end)
        return true;

    for(int edge_id: adjacency_list[node])
    {
        Edge& edge = edges[edge_id];
        if (edge.is_faulty) continue; // 跳过故障边

        int neighbor = (edge.from == node) ? edge.to : (edge.to == node) ? edge.from : -1;
        if (neighbor == -1 || vis[neighbor] != false) continue; // 跳过已访问节点

        int current_channel = used_channels.back();

        // 判断该边的连续空闲范围 是否满足业务宽度需求
        if (isChannelAvailable(edge, current_channel, current_channel + service.width - 1))
        {
            pathEdge.push_back(edge_id);
            used_channels.push_back(current_channel);
            if(dfs(neighbor, service, vis, pathNode, pathEdge, used_channels))
                return true;
        }
        else
        {
            if(node == service.start)
            {
                for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                {
                    if (isChannelAvailable(edge, ch, ch + service.width - 1))
                    {
                        pathEdge.push_back(edge_id);
                        used_channels.push_back(ch);
                        if(dfs(neighbor, service, vis, pathNode, pathEdge, used_channels))
                            return true;
                    }
                }
            }
            else
            {
                if ((nodes[node].channel_change_capacity - nodes[node].usedChannelCnt) > 0)
                {
                    // 使用当前节点处的变通道次数
                    for (int ch = 1; ch <= 40 - service.width + 1; ++ch)
                    {
                        if (isChannelAvailable(edge, ch, ch + service.width - 1))
                        {
                            pathEdge.push_back(edge_id);
                            used_channels.push_back(ch);
                            if(dfs(neighbor, service, vis, pathNode, pathEdge, used_channels))
                                return true;
                        }
                    }
                }
            }
        }
    }

    pathNode.pop_back();
    pathEdge.pop_back();
    used_channels.pop_back();
    return false;
}

// 业务路径重新规划前，释放老路径的通道资源，以至于新路径规划时可以使用
void Graph::bfsPlanBeforeHandOldPath(Service &service)
{
    for (int i = 0; i < service.path.size(); ++i)
    {
        updateChannelOccupation(edges[service.path[i]], service.channels[i].first, service.channels[i].second, false); // 释放该边占用的通道资源
        edges[service.path[i]].passServices.erase(service.id);  // 该边经过的业务也需要去除
    }
    // 释放老路径的节点使用的变通道情况
    for(int i = 0;i<service.changeChannelNodes.size();i++)
    {
        --nodes[service.changeChannelNodes[i]].usedChannelCnt;
    }
}

// 使用BFS3进行以业务重新规划（对BFS2进行优化）
bool Graph::bfsRePlanService3(Service& service,map<int, Service>& oldServices)
{
    queue<int> q;
    queue<int> qAll;
    vector<vector<int>> parent(nodes.size(), vector<int>(1, -1)); // 记录路径上的父节点  记录路径上的父节点（同时也用于判断某节点是否已经访问过）。具体来说，parent[node] 表示在到达节点 node 时，从哪个父节点过来的
    vector<vector<int>> parent_edge(nodes.size(), vector<int>(1, -1)); // 记录路径上的边编号
    vector<vector<int>> used_channels(nodes.size(), vector<int>(1, -1)); // 记录使用的通道编号  用于记录在到达每个节点时所使用的通道编号区间的起点
    vector<int> vis_edge(edges.size(),-1); // 判断边是否访问


    q.push(service.start);
    qAll.push(service.start);
    parent[service.start][0] = service.start;
    used_channels[service.start][0] = service.channels[0].first; // 初始通道编号

    // 业务重新规划前，释放老路径的通道占用资源
    // 原因：对受影响业务进行重新规划时，它可以使用该业务老路径的相关资源（故障边上的资源无法使用）
    bfsPlanBeforeHandOldPath(service);

    while (!q.empty())
    {
        int node = q.front();
        q.pop();

        if (node == service.end) break; // 到达终点

        for (int edge_id : adjacency_list[node])
        {
            Edge& edge = edges[edge_id];
            if (edge.is_faulty) continue; // 跳过故障边

            int neighbor = (edge.from == node) ? edge.to : (edge.to == node) ? edge.from : -1;
            if ((neighbor == -1 || parent[neighbor][0] != -1) && (vis_edge[edge_id]!=-1)) continue; // 跳过已访问节点与边
            // 若neighbor为node的父节点，也跳过
            if(std::find(parent[node].begin(), parent[node].end(), neighbor) != parent[node].end()) continue;


            vis_edge[edge_id] = edge_id;

            int usedChanSize = used_channels[node].size();
            for(int i=0;i<usedChanSize;i++)
            {
                int current_channel = used_channels[node][i];

                // 判断该边的连续空闲范围 是否满足业务宽度需求
                if (isChannelAvailable(edge, current_channel, current_channel + service.width - 1))
                {
                    if(isQEleRepeat(qAll, neighbor) == false)
                    {
                        qAll.push(neighbor);
                        q.push(neighbor);
                    }

                    if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                        parent[neighbor][0] = node;
                    else
                        parent[neighbor].push_back(node);

                    if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                        parent_edge[neighbor][0] = edge_id;
                    else
                        parent_edge[neighbor].push_back(edge_id);

                    if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                        used_channels[neighbor][0] = current_channel;
                    else
                        used_channels[neighbor].push_back(current_channel);
                }
                else
                {
                    if(node == service.start)
                    {
                        // 找到可选择的最窄的范围通道
                        int ch = findShortestRangeChannel(edge.channels, service.width);
                        if(ch != -1)
                        {
                            if(isQEleRepeat(qAll, neighbor) == false)
                            {
                                qAll.push(neighbor);
                                q.push(neighbor);
                            }

                            if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                                parent[neighbor][0] = node;
                            else
                                parent[neighbor].push_back(node);

                            if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                                parent_edge[neighbor][0] = edge_id;
                            else
                                parent_edge[neighbor].push_back(edge_id);

                            // 选择新通道
                            if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                                used_channels[neighbor][0] = ch;
                            else
                                used_channels[neighbor].push_back(ch);
                            break;
                        }
                    }
                    else
                    {
                        if ((nodes[node].channel_change_capacity - nodes[node].usedChannelCnt) > 0)
                        {
                            // 找到可选择的最窄的范围通道
                            int ch = findShortestRangeChannel(edge.channels, service.width);
                            if(ch != -1)
                            {
                                if (isChannelAvailable(edge, ch, ch + service.width - 1))
                                {
                                    if(isQEleRepeat(qAll, neighbor) == false)
                                    {
                                        qAll.push(neighbor);
                                        q.push(neighbor);
                                    }

                                    if (parent[neighbor].size()==1 && parent[neighbor][0]==-1)
                                        parent[neighbor][0] = node;
                                    else
                                        parent[neighbor].push_back(node);

                                    if (parent_edge[neighbor].size()==1 && parent_edge[neighbor][0]==-1)
                                        parent_edge[neighbor][0] = edge_id;
                                    else
                                        parent_edge[neighbor].push_back(edge_id);

                                    // 选择新通道
                                    if (used_channels[neighbor].size()==1 && used_channels[neighbor][0]==-1)
                                        used_channels[neighbor][0] = ch;
                                    else
                                        used_channels[neighbor].push_back(ch);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 规划失败
    if (parent[service.end][parent[service.end].size()-1] == -1)  // 无法到达目标
    {
        // 业务重新规划失败，标记为死亡，死亡的业务占用的通道资源以及变节点次数均不会释放，之后也不会对其进行重新规划
        // 还原-在BFS算法开始之前对业务老路径释放的通道资源情况
        // 并且对该规划失败的业务标记死亡
        restoreOldPathResources(service);

        // 尝试反向的二次规划
        if(bfsTimes == 0)
        {
            bfsTimes = 1;
            service.isDie = false;
            swap(service.start, service.end);
            if(bfsRePlanService3(service, oldServices))
            {
                // 二次规划成功
                swap(service.start, service.end);
                return true;
            }
            swap(service.start, service.end);
        }
        return false;
    }


    // 新路径情况
    vector<int> new_path;
    vector<pair<int, int>> new_channels;
    vector<int> newChangeChannelNodes;
    vector<int> newPathNodes;

    vector<int> pathNodes;
    vector<int> pathEdges;
    vector<int> pathChan;
    // 找到路径了
    if(findPath(service.end, service, pathNodes, pathEdges, pathChan, parent, parent_edge, used_channels))
    {
        new_path = pathEdges;
        newPathNodes = pathNodes;
        if(bfsTimes==0)
        {
            reverse(new_path.begin(), new_path.end());
            reverse(newPathNodes.begin(), newPathNodes.end());
            for(int i = pathChan.size()-1; i>=0; i--)
            {
                new_channels.push_back(make_pair(pathChan[i], pathChan[i] + service.width - 1));
            }
        }
        else
        {
            // 当前是业务首尾反向得到的状态，不需要进行reverse等相关操作
            for(int i = 0; i<=pathChan.size()-1; i++)
            {
                new_channels.push_back(make_pair(pathChan[i], pathChan[i] + service.width - 1));
            }
        }
    }
    else
    {
        // 没有找到路径
        // 还原-释放老路径通道占用情况
        restoreOldPathResources(service);
        return false;
    }


    // 比对该业务新老路径资源
    // 1.先判断是否使用了相同的边上的，共有的资源通道
    // repeatedEdges 存储了相同边的编号
    vector<int> repeatedEdges;
    repeatedEdges = findCommonElements(new_path,service.path);
    for(int i = 0;i<repeatedEdges.size();i++)
    {
        int commonElement = repeatedEdges[i];
        // 获得老路径 与 新路径 共有边分别在各自 path中的索引值
        pair<int, int> indices = findCommonElementIndices(service.path,new_path,commonElement);
        // commonElement --> new_path in indices.first
        // commonElement --> service.path in indices.second
        if (indices.first != -1 && indices.second != -1)
        {
            pair<int,int> result = removeOverlap(service.channels[indices.first],new_channels[indices.second]);
            // 完全不重叠 or 部分重叠
            if(result.first != -1 && result.second != -1)
            {
                // result 即 该业务老路径上 这条边上的 剩余资源
                service.channels[indices.first] = result;
            }
            else
            {
                // 完全覆盖---这条共有边上老路径所使用的通道资源，被新路径完全使用
                // 将这条边，在老路径中剔除
                service.path.erase(service.path.begin() + indices.first);
                service.channels.erase(service.channels.begin() + indices.first);
            }
        }
    }
    // 2.判断是否对相同的节点使用了变通道次数
    // 同时新路径上使用了变通道能力的节点，次数进行更新
    for(int i = 0;i<new_channels.size()-1;i++)
    {
        if(new_channels[i] != new_channels[i+1])
        {
            ++nodes[newPathNodes[i+1]].usedChannelCnt;
            newChangeChannelNodes.push_back(newPathNodes[i+1]);
        }
    }
    // 判断新老路径上是否对同一个节点使用了变通道的次数
    vector<int> repeatedChangeChannelNodes;
    repeatedChangeChannelNodes = findCommonElements(newChangeChannelNodes,service.changeChannelNodes);
    // 将service.changeChannelNodes 中 被新路径使用的变通道节点能力的节点去除
    if(repeatedChangeChannelNodes.size() != 0)
    {
        removeElements(service.changeChannelNodes,repeatedChangeChannelNodes);
    }
    // 将该业务老路径剩余的资源进行占用
    oldServices[service.id] = Service(service.id,service.start,service.end,service.width,service.value);
    oldServices[service.id].path = service.path;
    oldServices[service.id].channels = service.channels;
    oldServices[service.id].changeChannelNodes = service.changeChannelNodes;
    oldServices[service.id].edge_nums = service.path.size();
    // 将老路径剩余资源占用
    for (int i = 0; i < oldServices[service.id].path.size(); ++i)
    {
        updateChannelOccupation(edges[oldServices[service.id].path[i]], oldServices[service.id].channels[i].first,
                                oldServices[service.id].channels[i].second, true); // 占用的通道资源
    }
    // 占用老路径的剩余节点使用的变通道情况
    for(int i = 0;i<oldServices[service.id].changeChannelNodes.size();i++)
    {
        ++nodes[oldServices[service.id].changeChannelNodes[i]].usedChannelCnt;
    }

    //-------------------------------
    // 对重新规划的新路径通道进行占用
    for (int i = 0; i < new_path.size(); ++i)
    {
        updateChannelOccupation(edges[new_path[i]], new_channels[i].first, new_channels[i].second, true);
        edges[new_path[i]].passServices.insert(service.id);
    }


    service.path = new_path;
    service.channels = new_channels;
    // 更新路径上那些节点使用了变通道
    service.changeChannelNodes = newChangeChannelNodes;
    // 更新经过的节点
    service.pathNodes = newPathNodes;

    // 更新业务的经边数量
    service.edge_nums = new_path.size();


    return true;
}

// 寻找最小范围的插入通道(返回通道范围的左边界索引，为-1则没有)
int Graph::findShortestRangeChannel(const vector<bool> &vec, int width)
{
    int n = vec.size() - 1;
    int min_range_index = -1;
    vector<pair<int, int>> rangVec;

    for (int l = 1; l <= n - width + 1; ++l)
    {
        if (vec[l] == false)
        {
            int r = l;
            while (r <=n && vec[r] == false)
            {
                ++r;
            }
            int length = r - l;
            if(length >= width)
            {
                rangVec.push_back(make_pair(l,r-1));
            }
            l = r;
        }
    }

    if(rangVec.size() > 0)
    {
        // 排序
        sort(rangVec.begin(), rangVec.end(), [](auto a, auto b){
            return (a.second - a.first) < (b.second - b.first);
        });
        min_range_index = rangVec[0].first;
    }

    return min_range_index;
}

// 计算每一个测试场景开始时，业务总价值(对于同一个测试用例，每一个测试场景开始时，业务总价值相同，因为当一个测试场景结束后，光网络会恢复至初始状态)
int Graph::getBeginValue()
{
    set<int> serversIDs;
    // 遍历所有边
    for(int i = 1; i < edges.size(); i++)
    {
        for(int serId : edges[i].passServices)
        {
            // 判断该业务是否是活的
            if(services[serId].isDie == false)
            {
                serversIDs.insert(serId);
            }
        }
    }

    int valueSum = 0;
    // 遍历此时光网络上存活的业务ID
    for(int Id : serversIDs)
    {
        valueSum += services[Id].value;
    }
    return valueSum;
}

// 计算每一个测试场景结束后，光网络上存活的业务总价值
int Graph::getEndValue()
{
    set<int> serversIDs;
    // 遍历所有边
    for(int i = 1; i < edges.size(); i++)
    {
        for(int serId : edges[i].passServices)
        {
            // 判断该业务是否是活的
            if(services[serId].isDie == false)
            {
                serversIDs.insert(serId);
            }
        }
    }

    int valueSum = 0;
    // 遍历此时光网络上存活的业务ID
    for(int Id : serversIDs)
    {
        valueSum += services[Id].value;
    }
    return valueSum;
}

// 计算该测试场景下的得分
int Graph::getScore(int beginValue, int endValue)
{
    int score = 0;
    score = (endValue * 10000) / beginValue;
    return score;
}

// 计算测试用例总得分
int Graph::getCaseSumScore(vector<int> &vec)
{
    int caseSumScore = 0;
    for(int score: vec)
    {
        caseSumScore += score;
    }
    return caseSumScore;
}

/*********************************************类成员函数*******************************************************/


/*********************************************其他函数*******************************************************/
// 显示初始化输入的业务情况
void displayInitInput(Graph &graph,ofstream &logFile,int &count)
{
    logFile << "--------------初始输入信息情况: "<< count << "---------------" << endl;
    logFile << "-----------------------------" << endl;
    logFile << "业务数量: " << J << endl;
    for(int i = 1;i<=J;i++)
    {
        logFile << "业务编号: "<< graph.services[i].id << " : " << graph.services[i].start << " "
                << graph.services[i].end << " " << graph.services[i].edge_nums << " " << graph.services[i].value <<  endl;
        for(int j = 0;j<graph.services[i].edge_nums;j++)
        {
            logFile << "边的编号: " <<graph.services[i].path[j] << " : " << "[" << graph.services[i].channels[j].first <<" , " << graph.services[i].channels[j].second << "]" << endl;
        }
        logFile << "当前业务变通道节点: ";
        if(graph.services[i].changeChannelNodes.size() == 0)
        {
            logFile << "无"<< endl;
        }
        else
        {
            for(int k = 0;k<graph.services[i].changeChannelNodes.size();k++)
            {
                logFile << graph.services[i].changeChannelNodes[k] << " ";
            }
            logFile <<endl;
        }
        logFile << "--------" << endl;
    }

    logFile << "-----------------------------" << endl;
    logFile << "边上的业务情况: " << endl;
    for(int i = 1;i<=CurCntEdgeId;i++)
    {
        // 该边上存在业务
        if(graph.edges[i].passServices.size() != 0)
        {
            logFile << "边的编号: " << i << " 边上业务数量: " << graph.edges[i].passServices.size() << endl;
            logFile << "该边的业务情况: " << endl;
            for(auto &e : graph.edges[i].passServices)
            {
                logFile << e << " ";
            }
            logFile << endl;
            logFile << "该边剩余的连续通道最大宽度: " << graph.edges[i].freeContinuousChannelsMaxNum << endl;
            logFile << "--------" << endl;
        }
    }
    logFile << "-----------------------------" << endl;
    logFile << "图的初始信息: " << endl;
    logFile << "邻接表尺寸: " << graph.adjacency_list.size() << "x" << graph.adjacency_list.size() <<endl;
    logFile << "连接信息: " <<endl;
    logFile << "--------" << endl;
    for(int i = 1;i<=graph.adjacency_list.size();i++)
    {
        logFile << "节点 " << i << " : ";
        for(int j = 0;j<graph.adjacency_list[i].size();j++)
        {
            logFile << graph.adjacency_list[i][j] << " ";
        }
        logFile << endl;
    }
    logFile << "--------" << endl;

    logFile << "-----------------------------" << endl;
    logFile << "节点的信息情况: " <<endl;
    for(int i = 1;i<=N;i++)
    {
        logFile << "节点编号: " <<  i << endl;
        logFile << "初始变通道数: " << graph.nodes[i].channel_change_capacity << endl;
        logFile << "该节点的变通道使用次数: " << graph.nodes[i].usedChannelCnt <<endl;
        logFile << "--------" << endl;
    }

    logFile << "-----------------------------" << endl;
    logFile << "边上的空闲通道情况: " << endl;
    for(int i = 1;i<=CurCntEdgeId;i++)
    {
        logFile << "边 "<< i << " 空闲通道: ";
        int start = -1;
        for (int j = 1; j < graph.edges[i].channels.size(); ++j)
        {
            if (graph.edges[i].channels[j] == false)
            {
                if (start == -1) {
                    start = j; // 开始遇到1，记录起始索引
                }
            }
            else if (start != -1)
            {
                // 遇到非1，输出从start到i-1的连续1区间
                logFile << "(" << start << ", " << j - 1 << ")" << " ";
                start = -1; // 重置起始索引
            }
        }
        if (start != -1)
        {
            // 处理数组结尾为1的情况
            logFile << "(" << start << ", " << graph.edges[i].channels.size() - 1 << ")" << " ";
        }
        logFile << endl;
    }
    logFile << endl;
}

// 显示处理某个业务之后资源更新情况(变通道节点/边的通道)
void displayAfterHandlingServiceInput(Graph &graph,ofstream &logFile,int &count)
{
    logFile << "-->节点的信息情况: " <<endl;
    for(int i = 1;i<=N;i++)
    {
        logFile << "---->节点编号: " <<  i << endl;
        logFile << "------>初始变通道数: " << graph.nodes[i].channel_change_capacity << endl;
        logFile << "------>该节点的变通道使用次数: " << graph.nodes[i].usedChannelCnt <<endl;
    }

    logFile << "-->边上的空闲通道情况: " << endl;
    for(int i = 1;i<=CurCntEdgeId;i++)
    {
        logFile << "---->边 "<< i << " 空闲通道: ";
        int start = -1;
        for (int j = 1; j < graph.edges[i].channels.size(); ++j)
        {
            if (graph.edges[i].channels[j] == false)
            {
                if (start == -1) {
                    start = j; // 开始遇到1，记录起始索引
                }
            }
            else if (start != -1)
            {
                // 遇到非1，输出从start到i-1的连续1区间
                logFile << "(" << start << ", " << j - 1 << ")" << " ";
                start = -1; // 重置起始索引
            }
        }
        if (start != -1)
        {
            // 处理数组结尾为1的情况
            logFile << "(" << start << ", " << graph.edges[i].channels.size() - 1 << ")" << " ";
        }
        logFile << endl;
    }
    logFile << "-->边上的经由的业务情况: " << endl;
    for(int i = 1;i<=CurCntEdgeId;i++)
    {
        logFile << "---->边 "<< i << " 经由业务: ";
        for (auto &elem: graph.edges[i].passServices)
        {
            logFile << elem << " ";
        }
        logFile << endl;
    }
    logFile << endl;
}

// 获取当前时刻时间
stringstream getTime()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为时间戳
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 转换为tm结构
    std::tm now_tm = *std::localtime(&now_c);

    // 创建一个字符串流
    std::stringstream ss;

    // 设置时间格式
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    return ss;
}
/*********************************************其他函数*******************************************************/


int main()
{
    // 将标准输入重定向到文件
    if (freopen(redirectPath2, "r", stdin) == NULL)
    {
        std::cerr << "Error opening redirectPath file" << std::endl;
        exit(1);
    }

    // 将标准输出重定向
    if (freopen(outputLogPath, "w", stdout) == NULL)
    {
        std::cerr << "Error opening outputLog file" << std::endl;
        exit(1);
    }

    // 打开日志文件
    ofstream logFile(logFilePath, std::ios::trunc);
    if (!logFile.is_open())
    {
        std::cerr << "Failed to open log file." << std::endl;
        exit(1);
    }
    stringstream timeStr = getTime();
    logFile << timeStr.str() << endl;

    Graph graph;
    graph.initialize();
    int count = 0;

    cout << "重定向输入..." << endl;

    int BeginValueBegin = graph.getBeginValue();
    logFile << "测试用例开始业务总价值: " << BeginValueBegin << endl;
    int beginValue = 0;
    int endValue = 0;
    // 用于记录单个测试场景下得分---其和为整个测试用例下所有测试场景得分之和
    vector<int> vecScoresSingleScenario;

    // 处理故障和业务重规划
    int T;
    cin >> T;
    while (T--)
    {
        count++;

        beginValue = graph.getBeginValue();
        logFile << "测试场景 "<< count <<" 开始业务总价值: " << beginValue << endl;
        logFile << endl;

//        displayInitInput(graph,logFile,count);

        TestCaseInfo TCI;
        TCI.TCCnt = T+1;

        int edgeFailed;  // 之前使用全局变量代替
        while (cin >> edgeFailed && edgeFailed != -1)
        {
            // 标记业务故障边
            graph.markFaultyEdge(edgeFailed);

            // 分析受影响的业务编号
            unordered_set<int> affected_services_set = graph.edges[edgeFailed].passServices;
            // 业务按照价值进行降序排序---------------------------
            vector<int> affected_services(affected_services_set.begin(),affected_services_set.end());
            sort(affected_services.begin(), affected_services.end(), [&graph](int a, int b) {
                return graph.services[a].value > graph.services[b].value;  // 更改比较逻辑以实现降序
            });

            /***************Debug: 此次受到影响的业务****************/
            cout << "******************************************************************************" << endl;
            if(affected_services.size() == 0)
            {
                cout << "故障边: " << edgeFailed << ": " << "无受到影响的业务" << endl;
            }
            else
            {
                cout << "故障边: " << edgeFailed << ": ";
                for(auto &e : affected_services)
                {
                    cout << e << " ";
                }
                cout << endl;
            }
            /***************Debug: 此次受到影响的业务****************/

            logFile << "******************************************************************************" << endl;
            // 实现业务重规划逻辑
            vector<int> rePlanned_servicesId; // 记录重新规划成功的业务id
            map<int, Service> oldServices;  // 存储业务老路径占用的资源
            for (int service_id : affected_services)
            {
                Service& service = graph.services[service_id];

                /***************Debug: 处理业务原资源情况****************/
                cout << "************************************" << endl;
                cout << "重新规划业务: " << service.id << " start: " << service.start << " end: " <<service.end <<endl;
                cout << "业务路由节点: ";
                for(int i = 0;i<service.pathNodes.size()-1;i++)
                {
                    cout << service.pathNodes[i] << "->";
                }
                cout << service.end << endl;
                cout << "业务经边数: " << service.edge_nums << "-->路径情况: " << endl;
                for(int i = 0;i<service.path.size();i++)
                {
                    cout << "-->边: " << service.path[i] << " 通道: " << "("<<service.channels[i].first ;
                    cout << " ," << service.channels[i].second << ")" << endl;
                }
                cout << "业务变节点通道情况: " << endl;
                if(service.changeChannelNodes.size() == 0)
                {
                    cout << "-->该业务无变通道的节点使用情况" << endl;
                }
                else
                {
                    for(int i = 0;i<service.changeChannelNodes.size();i++)
                    {
                        cout << "-->节点: "<< service.changeChannelNodes[i] << " " << "容量: " ;
                        cout << graph.nodes[service.changeChannelNodes[i]].channel_change_capacity ;
                        cout << " 已使用次数: " << graph.nodes[service.changeChannelNodes[i]].usedChannelCnt << endl;
                    }
                }
                /***************Debug: 处理业务原资源情况****************/

                bfsTimes = 0;
                // 业务重新规划成功
                if(graph.bfsRePlanService3(service,oldServices))
                {
                    // 将规划成功的业务id 记录至 rePlanned_servicesId中
                    rePlanned_servicesId.push_back(service_id);

                    /***************Debug: 处理业务重新规划资源情况****************/
                    cout << "业务重新规划路由节点: ";
                    for(int i = 0;i<service.pathNodes.size()-1;i++)
                    {
                        cout << service.pathNodes[i] << "->";
                    }
                    cout << service.end << endl;
                    cout << "业务重规划经边数: " << service.edge_nums << "-->路径情况: " << endl;
                    for(int i = 0;i<service.path.size();i++)
                    {
                        cout << "-->边: " << service.path[i] << " 通道: " << "("<<service.channels[i].first ;
                        cout << " ," << service.channels[i].second << ")" << endl;
                    }
                    cout << "业务重新规划变节点通道情况: " << endl;
                    if(service.changeChannelNodes.size() == 0)
                    {
                        cout << "-->该业务重新规划无变通道的节点使用情况" << endl;
                    }
                    else
                    {
                        for(int i = 0;i<service.changeChannelNodes.size();i++)
                        {
                            cout << "-->节点: "<< service.changeChannelNodes[i] << " " << "容量: " ;
                            cout << graph.nodes[service.changeChannelNodes[i]].channel_change_capacity ;
                            cout << " 已使用次数: " << graph.nodes[service.changeChannelNodes[i]].usedChannelCnt << endl;
                        }
                    }
                    /***************Debug: 处理业务重新规划资源情况****************/
                }
                    /***************Debug: ***************************************/
                else
                {
                    // bfs规划失败
                    // 使用dfs进行二次规划
                    if(graph.dfsRePlanService(service,oldServices))
                    {
                        service.isDie = false;
                        rePlanned_servicesId.push_back(service_id);
                    }
                    else
                    {
                        cout << "-->业务: " << service.id << " 规划失败" << endl;
                    }
                }
                /***************Debug: ***************************************/

                /***************Debug: 故障边下处理业务x之后资源情况***************/
//                logFile << "故障边: " << edgeFailed << " 处理业务: " << service.id << " -->资源更新情况: " << endl;
//                displayAfterHandlingServiceInput(graph,logFile,count);
                /***************Debug: 故障边下处理业务x之后资源情况***************/
            }
            /***************Debug: 故障边下处理完所有受影响业务资源情况***************/
//            logFile << "故障边: " << edgeFailed << " 业务全部重新规划完之后" << " -->资源更新情况: " << endl;
//            displayAfterHandlingServiceInput(graph,logFile,count);
            /***************Debug: 故障边下处理完所有受影响业务资源情况***************/

            // 输出重新规划的业务详情
            graph.displayReplanServicesInfo(rePlanned_servicesId);
            // 对老路径剩余的资源进行释放
            graph.freeOldPathResources(oldServices);

            /***************Debug: 故障边下处理完受影响业务资源情况***************/
            logFile << "故障边: " << edgeFailed << " 业务全部重新规划完之后" << " -->并且将老路径剩余资源释放之后资源更新情况: " << endl;
            displayAfterHandlingServiceInput(graph,logFile,count);
            logFile << "故障边: " << edgeFailed << " 受影响的业务: ";
            for(auto &e : affected_services)
            {
                logFile << e << " ";
            }
            logFile << endl;
            logFile << "规划成功的业务: ";
            int successfulCnt1 = 0;
            int failedCnt1 = 0;
            for(auto &e : affected_services)
            {
                if(graph.services[e].isDie == false)
                {
                    successfulCnt1++;
                    logFile << e << " ";
                }
            }
            if(successfulCnt1 == 0)
            {
                logFile << "无";
            }
            logFile << endl;
            logFile << "规划失败的业务: ";
            for(auto &e : affected_services)
            {
                if(graph.services[e].isDie == true)
                {
                    failedCnt1++;
                    logFile << e << " ";
                }
            }
            if(failedCnt1 == 0)
            {
                logFile << "无";
            }
            logFile << endl;
            logFile << endl;
            logFile << endl;
            /***************Debug: 故障边下处理业务x之后资源情况***************/
            TCI.successCnt += successfulCnt1;
            TCI.failedCnt += failedCnt1;
        }
        planSuccessServices += TCI.successCnt;
        planFailedServices += TCI.failedCnt;
        TestCase.push_back(TCI);

        endValue = graph.getEndValue();
        logFile << "测试场景 "<< count <<" 结束业务总价值: " << endValue << endl;
        logFile << endl;
        vecScoresSingleScenario.push_back(graph.getScore(beginValue, endValue));

        // 重置图状态以处理下一个场景
        graph.resetGraphState();
    }

    logFile << "该测试用例总得分: " << graph.getCaseSumScore(vecScoresSingleScenario) << endl;
    logFile << endl;

    logFile << "总规划业务数量: " << planFailedServices + planSuccessServices << endl;
    logFile << "规划成功的业务数量: " << planSuccessServices << endl;
    logFile << "规划失败的业务数量: " << planFailedServices << endl;

    logFile << "----------------------------------" << endl;
    logFile << "每个测试用例场景业务规划情况: " << endl;
    for(auto &e : TestCase)
    {
        logFile << "-----------------" << endl;
        logFile << "测试场景: " << e.TCCnt << endl;
        logFile << "该场景下成功规划业务数量: " << e.successCnt << endl;
        logFile << "该场景下失败规划业务数量: " << e.failedCnt << endl;
    }

    return 0;
}
