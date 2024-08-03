#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <set>
#include <ctime>
#define INF 2147483647
using namespace std;

//储存常数
const int maxM = 50000; //边的最大数目
const int maxN = 5000;  //节点的最大数目
const int maxBus = 10000;   //业务的最大数目
const int maxP = 80;    //最大单边通道数量P

//规则常数
const int maxNewEdgeNum = 20000;  //最多加边数
const int newEdgeCost = 1000000;  //加边成本
const int multiplierCost = 100;  //放大器成本
const int edgeCost = 1;  //途经一条边的成本

int N, M, T, P, D, Y; //节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D、加边数量Y

//边是单向边，如果要存双向边的话是2条，则边编号需要处理
class Edge {
public:
    int from, to, d;    //起点，终点，边的距离
    int Pile[maxP]; //该边上存在的通道，记录的是当前承载的业务的编号，不承载业务时值为-1
    Edge()
    {
        from = -1;
        to = -1;
        d = 0;
        for (int i = 0; i < maxP; ++i)
            Pile[i] = -1;
    }
}edge[maxM];    //边集数组

class Business {
public:
    int start;  //业务起点
    int end;    //业务终点
    int busId;  //业务Id
    int pileId; //业务所占据的通道Id
    Business()
    {
        start = -1;
        end = -1;
        busId = -1;
        pileId = -1;
    }
    vector<int> path;   //存储路径所经过的边
    vector<int> multiplierNodeId;  //存储所经过的放大器所在节点的编号
}buses[maxBus];

struct HashFunc_t {
    size_t operator() (const pair<int, int>& key) const {
        return hash<int>()(key.first) ^ hash<int>()(key.second);;
    }
};

struct Equalfunc_t {
    bool operator() (pair<int, int> const& a, pair<int, int> const& b) const {
        return a.first == b.first && a.second == b.second;
    }
};

unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   //记录两个节点间的最短边的距离
vector<pair<int, int>> addEdgeTemp;

clock_t startTime, endTime;
const char* programName;


void addEdge(int s, int t, int d);
void addBus(int s, int t);
bool init(int argc, char** argv);
void readTestCase();
void getResult();
void judgeResult();

int main(int argc, char** argv)
{
    if (init(argc, argv))
    {
        cout << "开始输入测试用例" << endl;
        readTestCase();
        getResult();
        cout << endl << "----判题结果----" << endl;
        judgeResult();
    }

    return 0;
}

/**
  * @brief	加边函数，向边集数组中添加单向边
  * @param	s: 起点
  * @param  t: 终点
  * @param  d: 距离
  * @retval	无
  */
void addEdge(int s, int t, int d)
{
    static int cntEdge = 0;
    edge[cntEdge].from = s; //起点
    edge[cntEdge].to = t;   //终点
    edge[cntEdge].d = d;    //距离
    ++cntEdge;
}

/**
  * @brief	加业务函数
  * @param	s: 起点
  * @param  t: 终点
  * @retval	无
  */
void addBus(int s, int t)
{
    static int cntBus = 0;
    buses[cntBus].start = s;
    buses[cntBus].end = t;
    buses[cntBus].busId = cntBus;
    ++cntBus;
}

/**
  * @brief	判题器初始化函数，用于处理命令行启动参数
  * @param	argc: 参数个数
  * @param  **argv: 参数字符串数组地址
  * @retval	false：初始化失败，true：初始化成功
  */
bool init(int argc, char** argv)
{
    //无输入参数，打印参数列表
    if (argc == 1)
    {
        cout << "Usage:" << endl;
        cout << "  Judger.exe <program>" << endl;
        cout << "Details:" << endl;
        cout << "  <program>  解题程序可执行文件" << endl;
        cout << "Samples:" << endl;
        cout << "  Judger.exe main.exe" << endl;
        return false;
    }
    //输入参数不足
    else if (argc < 2)
    {
        cout << "判题器启动参数错误：参数不足" << endl;
        return false;
    }
    //输入参数数量正常
    else if (argc == 2)
    {
        //检验参数合法性
        if (strstr(argv[1], ".exe"))
        {
            //验证文件是否存在
            ifstream f(argv[1]);
            if (f.good())
                programName = argv[1];
            else
            {
                cout << "判题器启动参数错误：指定的解题程序不存在" << endl;
                return false;
            }
        }
        else
        {
            cout << "判题器启动参数错误：指定的解题程序文件不是可执行文件" << endl;
            return false;
        }
    }
    //输入参数数量过多
    else
    {
        cout << "判题器启动参数错误：参数过多" << endl;
        return false;
    }
    return true;
}

/**
  * @brief	读取测试用例
  * @param	无
  * @retval	无
  */
void readTestCase()
{
    int s, t, d;
    //读取基础数据
    cin >> N;
    cin >> M;
    cin >> T;
    cin >> P;
    cin >> D;
    //读取边数据
    for (int i = 0; i < M; ++i)
    {
        cin >> s;
        cin >> t;
        cin >> d;
        //首次加某两节点之间的边时键不存在，最短距离置INF
        if (minDist.find(make_pair(s, t)) == minDist.end())
        {
            minDist[make_pair(s, t)] = INF;
            minDist[make_pair(t, s)] = INF;
        }
        if (d < minDist[make_pair(s, t)])
        {
            minDist[make_pair(s, t)] = d;
            minDist[make_pair(t, s)] = d;
        }
        addEdge(s, t, d);  //双向边
        addEdge(t, s, d);
    }
    //读取业务数据
    for (int i = 0; i < T; ++i)
    {
        cin >> s;
        cin >> t;
        addBus(s, t);
    }
}

/**
  * @brief	运行解题程序并获得结果
  * @param	无
  * @retval	无
  */
void getResult()
{
    int s, t, p, m, n, temp;
    //运行解题程序并打开管道读端
    FILE* pipe = _popen(programName, "r");  //目前较为简单的建立管道的方法不能使用C++fstream类，暂时采用C方法
    startTime = clock();  //启动程序后开始计时，因为会算上输入时间所以需要尽量减少输入的时间
    if (!pipe) {
        cout << "判题器异常：打开管道失败" << endl;
        return;
    }
    //读取结果
    fscanf(pipe, "%d", &Y);  //加边数
    //加边
    for (int i = 0; i < Y; ++i)
    {
        fscanf(pipe, "%d %d", &s, &t);
        addEdgeTemp.push_back(make_pair(s, t));
    }
    //业务结果
    for (int i = 0; i < T; ++i)
    {
        fscanf(pipe, "%d %d %d", &p, &m, &n);
        buses[i].pileId = p;
        for (int j = 0; j < m; ++j)
        {
            fscanf(pipe, "%d", &temp);
            buses[i].path.push_back(temp);
        }
        for (int j = 0; j < n; ++j)
        {
            fscanf(pipe, "%d", &temp);
            buses[i].multiplierNodeId.push_back(temp);
        }
    }
    endTime = clock();  //读取结束停止计时，会受到判题器读取速度的影响，不过影响非常小
    //关闭管道
    _pclose(pipe);
}

/**
  * @brief	对结果进行评判
  * @param	无
  * @retval	无
  */
void judgeResult()
{
    int cost = 0;
    cout << "运行时间：" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;  //输出运行时间
    //判断结果合法性
    //加边数量
    if (Y > maxNewEdgeNum)
    {
        cout << "逻辑错误：【超过加边限制】共加了" << Y << "条边" << endl;
        return;
    }
    cost += Y * newEdgeCost;  //加边成本
    //加边
    for (int i = 0; i < Y; ++i)
    {
        int s = addEdgeTemp[i].first;
        int t = addEdgeTemp[i].second;
        //点编号是否合法
        if (s < 0 || s >= N || t < 0 || t >= N)
        {
            cout << "逻辑错误：【点编号非法】试图在点" << s << "和点" << t << "之间加边，而点编号范围为0~" << N - 1 << endl;
            return;
        }
        //如果新边找不到键，则证明原来这两个节点之间没有边
        if (minDist.find(make_pair(s, t)) == minDist.end())
        {
            cout << "逻辑错误：【新增边非法】节点" << s << "和节点" << t << "之间原本没有边，但进行了加边" << endl;
            return;
        }
        addEdge(s, t, minDist[make_pair(s, t)]);  //加双向边
        addEdge(t, s, minDist[make_pair(s, t)]);
    }
    //业务分析
    for (int i = 0; i < T; ++i)
    {
        Business bus = buses[i];
        //通道编号是否非法
        if (bus.pileId < 0 || bus.pileId >= P)
        {
            cout << "逻辑错误：【通道编号非法】业务" << bus.busId << "使用了通道" << bus.pileId << "，而通道编号范围为0~" << P - 1 << endl;
            return;
        }
        //路径遍历
        int curNode = bus.start;  //当前遍历路径的起点
        set<int> vis;  //路径途径的节点集合
        for (int j = 0; j < bus.path.size(); ++j)
        {
            vis.insert(curNode);  //记录途径点（不包含终点）
            //边编号是否非法
            if (bus.path[j] < 0 || bus.path[j] >= M + Y)
            {
                cout << "逻辑错误：【边编号非法】业务" << bus.busId << "路径上有边" << bus.path[j] << "，而边编号范围为0~" << M + Y - 1 << endl;
                return;
            }
            //通道是否可用
            if (edge[bus.path[j] * 2].Pile[bus.pileId] == -1)
            {
                edge[bus.path[j] * 2].Pile[bus.pileId] = bus.busId;
                edge[bus.path[j] * 2 + 1].Pile[bus.pileId] = bus.busId;  //双向边
            }
            else
            {
                cout << "逻辑错误：【通道冲突】业务" << bus.busId << "和业务" << edge[bus.path[j] * 2].Pile[bus.pileId] << "都使用了边" << bus.path[j] << "上的通道" << bus.pileId << endl;
                return;
            }
            //路径是否连续
            if (edge[bus.path[j] * 2].from == curNode)
                curNode = edge[bus.path[j] * 2].to;
            else if (edge[bus.path[j] * 2 + 1].from == curNode)
                curNode = edge[bus.path[j] * 2 + 1].to;
            else
            {
                cout << "逻辑错误：【路径不连续】业务" << bus.busId << "的路径不连续" << endl;
                return;
            }
            //是否到达终点
            if (j == bus.path.size() - 1 && curNode != bus.end)
            {
                cout << "逻辑错误：【路径不连续】业务" << bus.busId << "的路径不连续" << endl;
                return;
            }
        }
        //路径边数是否合理
        if (bus.path.size() != vis.size())
        {
            cout << "逻辑错误：【路径边数非法】业务" << bus.busId << "的路径边数非法，途经" << vis.size() + 1 << "个节点却走了" << bus.path.size() << "条边" << endl;
            return;
        }
        //放大器数是否合理
        if (bus.multiplierNodeId.size() > vis.size() - 1)
        {
            cout << "逻辑错误：【路径放大器数非法】业务" << bus.busId << "的路径放大器数非法，途经" << vis.size() + 1 << "个节点却有" << bus.multiplierNodeId.size() << "个放大器" << endl;
            return;
        }
        //遍历放大器
        for (int j = 0; j < bus.multiplierNodeId.size(); ++j)
        {
            //放大器编号是否合法
            if (bus.multiplierNodeId[j] < 0 || bus.multiplierNodeId[j] >= N)
            {
                cout << "逻辑错误：【点编号非法】业务" << bus.busId << "放大器加在了点" << bus.path[j] << "上，而点编号范围为0~" << N - 1 << endl;
                return;
            }
            //如果途径点集合找不到放大器所在节点
            if (vis.find(bus.multiplierNodeId[j]) == vis.end())
            {
                cout << "逻辑错误：【放大器不在路径上】业务" << i << "有放大器在节点" << bus.multiplierNodeId[j] << "上，该点不在路径上" << endl;
                return;
            }
        }
        //遍历路径点（除了终点），不在上面一起遍历是因为要先确定放大器是否都在路径上
        curNode = bus.start;  //当前遍历路径的起点
        int curA = D;  //信号强度
        int pm = 0;  //放大器指针
        for (int j = 0; j < bus.path.size(); ++j)
        {
            //该节点有放大器
            if (pm < bus.multiplierNodeId.size())  //避免越界访问
            {
                if (curNode == bus.multiplierNodeId[pm])
                {
                    curA = D;  //重置信号强度
                    ++pm;  //移动信号放大器指针
                }
            }
            //更新指向下一节点
            if (edge[bus.path[j] * 2].from == curNode)
                curNode = edge[bus.path[j] * 2].to;
            else if (edge[bus.path[j] * 2 + 1].from == curNode)
                curNode = edge[bus.path[j] * 2 + 1].to;
            curA -= edge[bus.path[j] * 2].d;  //下一节点信号强度
            //到下一个点时信号已衰减到不可用
            if (curA < 0)
            {
                cout << "逻辑错误：【光信号未及时放大】业务" << bus.busId << "信号在节点" << curNode << "已衰减到不可用" << endl;
                return;
            }
        }
        //都正常则计算该业务成本
        cost += (bus.path.size() * edgeCost + bus.multiplierNodeId.size() * multiplierCost);
    }
    //一切正常，输出分数
    cout << "成本：" << cost << endl;
}

//从文件中读取测试样例并使用管道通信实现，已废弃，因为管道不能双工通信
///**
//  * @brief	读取测试用例
//  * @param	无
//  * @retval	无
//  */
//void readTestCase()
//{
//    int s, t, d;
//    // 打开测试用例文件
//    ifstream TestCase("TestCase.txt");
//    if (TestCase.is_open())
//    {
//        //读取基础数据
//        TestCase >> N;
//        TestCase >> M;
//        TestCase >> T;
//        TestCase >> P;
//        TestCase >> D;
//        //读取边数据
//        for (int i = 0; i < M; ++i)
//        {
//            TestCase >> s;
//            TestCase >> t;
//            TestCase >> d;
//            //首次加某两节点之间的边时键不存在，最短距离置INF
//            if (minDist.find(make_pair(s, t)) == minDist.end())
//            {
//                minDist[make_pair(s, t)] = INF;
//                minDist[make_pair(t, s)] = INF;
//            }
//            if (d < minDist[make_pair(s, t)])
//            {
//                minDist[make_pair(s, t)] = d;
//                minDist[make_pair(t, s)] = d;
//            }
//            addEdge(s, t, d);  //双向边
//            addEdge(t, s, d);
//        }
//        //读取业务数据
//        for (int i = 0; i < T; ++i)
//        {
//            TestCase >> s;
//            TestCase >> t;
//            addBus(s, t);
//        }
//        // 关闭文件
//        TestCase.close();
//    }
//    else
//    {
//        cout << "判题器错误：打开TestCase.txt失败" << endl;
//        system("pause");
//        exit(0);
//    }
//}
//
///**
//  * @brief	向解题程序发送测试用例
//  * @param	无
//  * @retval	无
//  */
//void sendTestCase()
//{
//    //暂未了解到C++方便写管道的方法，所以使用C方法
//    //打开管道写端并执行解题程序
//    FILE* pipe = _popen("main.exe", "w");
//    if (!pipe) {
//        cout << "判题器异常：打开管道失败" << endl;
//        system("pause");
//        exit(0);
//    }
//    //开始计时
//    startTime = clock();
//    //发送测试用例
//    fprintf(pipe, "%d %d %d %d %d\n", N, M, T, P, D);
//    //边数据
//    for (int i = 0; i < M; ++i)
//    {
//        fprintf(pipe, "%d %d %d\n", edge[i * 2].from, edge[i * 2].to, edge[i * 2].d);
//    }
//    //业务数据
//    for (int i = 0; i < T; ++i)
//    {
//        fprintf(pipe, "%d %d\n", buses[i].start, buses[i].end);
//    }
//}
//
///**
//  * @brief	接收解题程序输出的结果
//  * @param	无
//  * @retval	无
//  */
//void receiveResult()
//{
//    //暂未了解到C++方便读管道的方法，所以使用C方法
//    int s, t, p, m, n, temp;
//    //打开管道读端
//    FILE* pipe = _popen("main.exe", "r");
//    if (!pipe) {
//        cout << "判题器异常：打开管道失败" << endl;
//        system("pause");
//        exit(0);
//    }
//    //读取结果
//    //加边数
//    fscanf(pipe, "%d", &Y);
//    //加边
//    for (int i = 0; i < Y; ++i)
//    {
//        fscanf(pipe, "%d %d", &s, &t);
//        addEdge(s, t, minDist[make_pair(s, t)]);  //新边双向边
//        addEdge(t, s, minDist[make_pair(t, s)]);
//    }
//    //业务结果
//    for (int i = 0; i < T; ++i)
//    {
//        fscanf(pipe, "%d %d %d", &p, &m, &n);
//        buses[i].pileId = p;
//        for (int j = 0; j < m; ++j)
//        {
//            fscanf(pipe, "%d", &temp);
//            buses[i].path.push_back(temp);
//        }
//        for (int j = 0; j < n; ++j)
//        {
//            fscanf(pipe, "%d", &temp);
//            buses[i].multiplierNodeId.push_back(temp);
//        }
//    }
//    // 关闭管道
//    _pclose(pipe);
//}
