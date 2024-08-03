/*
 *  BaseLine版本 ---> 对应的Debug版本：BaselineDebug.cpp
 *  已解决BUG:
 *      重新规划的业务老路径资源并不会，立即释放，要等到所有受到影响的业务全部都规划完毕之后才会释放资源 (BUG修复)
 *
 *  需要在该版本继续优化:
 *      --> 受到影响的业务按照价值进行排序，逐个进行处理 (已完成)
 * */

#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
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
    Node(int id = 0, int capacity = 0) : id(id), channel_change_capacity(capacity) ,usedChannelCnt(ZERO){}
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
            from(from), to(to),edgeId(CurCntEdgeId),is_faulty(false),freeContinuousChannelsMaxNum(InitChannelNums)
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


    // 构造函数
    Service(int id=0,int start = 0, int end = 0, int width = 0, int value = 0):
            id(id),start(start),end(end),width(width),value(value),edge_nums(ZERO),isDie(false){}
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

    // 计算每一个测试场景开始时，业务总价值(对于同一个测试用例，每一个测试场景开始时，业务总价值相同，因为当一个测试场景结束后，光网络会恢复至初始状态)
    int getBeginValue();
    // 计算每一个测试场景结束后，光网络上存活的业务总价值
    int getEndValue();
    // 计算该测试场景下的得分
    int getScore(int beginValue, int endValue);
    // 计算测试用例总得分
    int getCaseSumScore(vector<int> &vec);
};

/*********************************************类成员函数*******************************************************/
// 构造函数，预置空对象确保从1开始编号
Graph::Graph()
{
    nodes.emplace_back(); // 添加一个空的节点对象
    edges.emplace_back(); // 添加一个空的边对象
}

// 增加节点
void Graph::addNode(int id, int capacity)
{
    nodes.emplace_back(id, capacity);
}

// 增加边
void Graph::addEdge(int from, int to)
{
    edges.emplace_back(from, to);
    // 邻接表
    adjacency_list[from].push_back(CurCntEdgeId);
    adjacency_list[to].push_back(CurCntEdgeId);
}

//增加服务
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
pair<int, int> Graph::removeOverlap(const pair<int, int>& pair1, const pair<int, int>& pair2) {
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
void Graph::removeElements(vector<int>& vec1, const vector<int>& vec2) {
    for (int elem : vec2) {
        // 查找并删除vec1中的elem
        vec1.erase(std::remove(vec1.begin(), vec1.end(), elem), vec1.end());
    }
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
                << graph.services[i].end << " " << graph.services[i].edge_nums << endl;
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
//    if (freopen(redirectPath2, "r", stdin) == NULL)
//    {
//        std::cerr << "Error opening redirectPath file" << std::endl;
//        exit(1);
//    }

    Graph graph;
    graph.initialize();


    // 处理故障和业务重规划
    int T;
    cin >> T;
    while (T--)
    {
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

            // 实现业务重规划逻辑
            vector<int> rePlanned_servicesId; // 记录重新规划成功的业务id
            map<int, Service> oldServices;  // 存储业务老路径占用的资源
            for (int service_id : affected_services)
            {
                Service& service = graph.services[service_id];

                // 业务重新规划成功
                if(graph.bfsRePlanService(service,oldServices))
                {
                    // 将规划成功的业务id 记录至 rePlanned_servicesId中
                    rePlanned_servicesId.push_back(service_id);
                }
            }
            // 输出重新规划的业务详情
            graph.displayReplanServicesInfo(rePlanned_servicesId);
            // 对老路径剩余的资源进行释放
            graph.freeOldPathResources(oldServices);

        }
        // 重置图状态以处理下一个场景
        graph.resetGraphState();
    }

    return 0;
}
