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

//���泣��
const int maxM = 50000; //�ߵ������Ŀ
const int maxN = 5000;  //�ڵ�������Ŀ
const int maxBus = 10000;   //ҵ��������Ŀ
const int maxP = 80;    //��󵥱�ͨ������P

//������
const int maxNewEdgeNum = 20000;  //���ӱ���
const int newEdgeCost = 1000000;  //�ӱ߳ɱ�
const int multiplierCost = 100;  //�Ŵ����ɱ�
const int edgeCost = 1;  //;��һ���ߵĳɱ�

int N, M, T, P, D, Y; //�ڵ�����N����������M��ҵ������T������ͨ������P�����˥������D���ӱ�����Y

//���ǵ���ߣ����Ҫ��˫��ߵĻ���2������߱����Ҫ����
class Edge {
public:
    int from, to, d;    //��㣬�յ㣬�ߵľ���
    int Pile[maxP]; //�ñ��ϴ��ڵ�ͨ������¼���ǵ�ǰ���ص�ҵ��ı�ţ�������ҵ��ʱֵΪ-1
    Edge()
    {
        from = -1;
        to = -1;
        d = 0;
        for (int i = 0; i < maxP; ++i)
            Pile[i] = -1;
    }
}edge[maxM];    //�߼�����

class Business {
public:
    int start;  //ҵ�����
    int end;    //ҵ���յ�
    int busId;  //ҵ��Id
    int pileId; //ҵ����ռ�ݵ�ͨ��Id
    Business()
    {
        start = -1;
        end = -1;
        busId = -1;
        pileId = -1;
    }
    vector<int> path;   //�洢·���������ı�
    vector<int> multiplierNodeId;  //�洢�������ķŴ������ڽڵ�ı��
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

unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   //��¼�����ڵ�����̱ߵľ���
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
        cout << "��ʼ�����������" << endl;
        readTestCase();
        getResult();
        cout << endl << "----������----" << endl;
        judgeResult();
    }

    return 0;
}

/**
  * @brief	�ӱߺ�������߼���������ӵ����
  * @param	s: ���
  * @param  t: �յ�
  * @param  d: ����
  * @retval	��
  */
void addEdge(int s, int t, int d)
{
    static int cntEdge = 0;
    edge[cntEdge].from = s; //���
    edge[cntEdge].to = t;   //�յ�
    edge[cntEdge].d = d;    //����
    ++cntEdge;
}

/**
  * @brief	��ҵ����
  * @param	s: ���
  * @param  t: �յ�
  * @retval	��
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
  * @brief	��������ʼ�����������ڴ�����������������
  * @param	argc: ��������
  * @param  **argv: �����ַ��������ַ
  * @retval	false����ʼ��ʧ�ܣ�true����ʼ���ɹ�
  */
bool init(int argc, char** argv)
{
    //�������������ӡ�����б�
    if (argc == 1)
    {
        cout << "Usage:" << endl;
        cout << "  Judger.exe <program>" << endl;
        cout << "Details:" << endl;
        cout << "  <program>  ��������ִ���ļ�" << endl;
        cout << "Samples:" << endl;
        cout << "  Judger.exe main.exe" << endl;
        return false;
    }
    //�����������
    else if (argc < 2)
    {
        cout << "�����������������󣺲�������" << endl;
        return false;
    }
    //���������������
    else if (argc == 2)
    {
        //��������Ϸ���
        if (strstr(argv[1], ".exe"))
        {
            //��֤�ļ��Ƿ����
            ifstream f(argv[1]);
            if (f.good())
                programName = argv[1];
            else
            {
                cout << "������������������ָ���Ľ�����򲻴���" << endl;
                return false;
            }
        }
        else
        {
            cout << "������������������ָ���Ľ�������ļ����ǿ�ִ���ļ�" << endl;
            return false;
        }
    }
    //���������������
    else
    {
        cout << "�����������������󣺲�������" << endl;
        return false;
    }
    return true;
}

/**
  * @brief	��ȡ��������
  * @param	��
  * @retval	��
  */
void readTestCase()
{
    int s, t, d;
    //��ȡ��������
    cin >> N;
    cin >> M;
    cin >> T;
    cin >> P;
    cin >> D;
    //��ȡ������
    for (int i = 0; i < M; ++i)
    {
        cin >> s;
        cin >> t;
        cin >> d;
        //�״μ�ĳ���ڵ�֮��ı�ʱ�������ڣ���̾�����INF
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
        addEdge(s, t, d);  //˫���
        addEdge(t, s, d);
    }
    //��ȡҵ������
    for (int i = 0; i < T; ++i)
    {
        cin >> s;
        cin >> t;
        addBus(s, t);
    }
}

/**
  * @brief	���н�����򲢻�ý��
  * @param	��
  * @retval	��
  */
void getResult()
{
    int s, t, p, m, n, temp;
    //���н�����򲢴򿪹ܵ�����
    FILE* pipe = _popen(programName, "r");  //Ŀǰ��Ϊ�򵥵Ľ����ܵ��ķ�������ʹ��C++fstream�࣬��ʱ����C����
    startTime = clock();  //���������ʼ��ʱ����Ϊ����������ʱ��������Ҫ�������������ʱ��
    if (!pipe) {
        cout << "�������쳣���򿪹ܵ�ʧ��" << endl;
        return;
    }
    //��ȡ���
    fscanf(pipe, "%d", &Y);  //�ӱ���
    //�ӱ�
    for (int i = 0; i < Y; ++i)
    {
        fscanf(pipe, "%d %d", &s, &t);
        addEdgeTemp.push_back(make_pair(s, t));
    }
    //ҵ����
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
    endTime = clock();  //��ȡ����ֹͣ��ʱ�����ܵ���������ȡ�ٶȵ�Ӱ�죬����Ӱ��ǳ�С
    //�رչܵ�
    _pclose(pipe);
}

/**
  * @brief	�Խ����������
  * @param	��
  * @retval	��
  */
void judgeResult()
{
    int cost = 0;
    cout << "����ʱ�䣺" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;  //�������ʱ��
    //�жϽ���Ϸ���
    //�ӱ�����
    if (Y > maxNewEdgeNum)
    {
        cout << "�߼����󣺡������ӱ����ơ�������" << Y << "����" << endl;
        return;
    }
    cost += Y * newEdgeCost;  //�ӱ߳ɱ�
    //�ӱ�
    for (int i = 0; i < Y; ++i)
    {
        int s = addEdgeTemp[i].first;
        int t = addEdgeTemp[i].second;
        //�����Ƿ�Ϸ�
        if (s < 0 || s >= N || t < 0 || t >= N)
        {
            cout << "�߼����󣺡����ŷǷ�����ͼ�ڵ�" << s << "�͵�" << t << "֮��ӱߣ������ŷ�ΧΪ0~" << N - 1 << endl;
            return;
        }
        //����±��Ҳ���������֤��ԭ���������ڵ�֮��û�б�
        if (minDist.find(make_pair(s, t)) == minDist.end())
        {
            cout << "�߼����󣺡������߷Ƿ����ڵ�" << s << "�ͽڵ�" << t << "֮��ԭ��û�бߣ��������˼ӱ�" << endl;
            return;
        }
        addEdge(s, t, minDist[make_pair(s, t)]);  //��˫���
        addEdge(t, s, minDist[make_pair(s, t)]);
    }
    //ҵ�����
    for (int i = 0; i < T; ++i)
    {
        Business bus = buses[i];
        //ͨ������Ƿ�Ƿ�
        if (bus.pileId < 0 || bus.pileId >= P)
        {
            cout << "�߼����󣺡�ͨ����ŷǷ���ҵ��" << bus.busId << "ʹ����ͨ��" << bus.pileId << "����ͨ����ŷ�ΧΪ0~" << P - 1 << endl;
            return;
        }
        //·������
        int curNode = bus.start;  //��ǰ����·�������
        set<int> vis;  //·��;���Ľڵ㼯��
        for (int j = 0; j < bus.path.size(); ++j)
        {
            vis.insert(curNode);  //��¼;���㣨�������յ㣩
            //�߱���Ƿ�Ƿ�
            if (bus.path[j] < 0 || bus.path[j] >= M + Y)
            {
                cout << "�߼����󣺡��߱�ŷǷ���ҵ��" << bus.busId << "·�����б�" << bus.path[j] << "�����߱�ŷ�ΧΪ0~" << M + Y - 1 << endl;
                return;
            }
            //ͨ���Ƿ����
            if (edge[bus.path[j] * 2].Pile[bus.pileId] == -1)
            {
                edge[bus.path[j] * 2].Pile[bus.pileId] = bus.busId;
                edge[bus.path[j] * 2 + 1].Pile[bus.pileId] = bus.busId;  //˫���
            }
            else
            {
                cout << "�߼����󣺡�ͨ����ͻ��ҵ��" << bus.busId << "��ҵ��" << edge[bus.path[j] * 2].Pile[bus.pileId] << "��ʹ���˱�" << bus.path[j] << "�ϵ�ͨ��" << bus.pileId << endl;
                return;
            }
            //·���Ƿ�����
            if (edge[bus.path[j] * 2].from == curNode)
                curNode = edge[bus.path[j] * 2].to;
            else if (edge[bus.path[j] * 2 + 1].from == curNode)
                curNode = edge[bus.path[j] * 2 + 1].to;
            else
            {
                cout << "�߼����󣺡�·����������ҵ��" << bus.busId << "��·��������" << endl;
                return;
            }
            //�Ƿ񵽴��յ�
            if (j == bus.path.size() - 1 && curNode != bus.end)
            {
                cout << "�߼����󣺡�·����������ҵ��" << bus.busId << "��·��������" << endl;
                return;
            }
        }
        //·�������Ƿ����
        if (bus.path.size() != vis.size())
        {
            cout << "�߼����󣺡�·�������Ƿ���ҵ��" << bus.busId << "��·�������Ƿ���;��" << vis.size() + 1 << "���ڵ�ȴ����" << bus.path.size() << "����" << endl;
            return;
        }
        //�Ŵ������Ƿ����
        if (bus.multiplierNodeId.size() > vis.size() - 1)
        {
            cout << "�߼����󣺡�·���Ŵ������Ƿ���ҵ��" << bus.busId << "��·���Ŵ������Ƿ���;��" << vis.size() + 1 << "���ڵ�ȴ��" << bus.multiplierNodeId.size() << "���Ŵ���" << endl;
            return;
        }
        //�����Ŵ���
        for (int j = 0; j < bus.multiplierNodeId.size(); ++j)
        {
            //�Ŵ�������Ƿ�Ϸ�
            if (bus.multiplierNodeId[j] < 0 || bus.multiplierNodeId[j] >= N)
            {
                cout << "�߼����󣺡����ŷǷ���ҵ��" << bus.busId << "�Ŵ��������˵�" << bus.path[j] << "�ϣ������ŷ�ΧΪ0~" << N - 1 << endl;
                return;
            }
            //���;���㼯���Ҳ����Ŵ������ڽڵ�
            if (vis.find(bus.multiplierNodeId[j]) == vis.end())
            {
                cout << "�߼����󣺡��Ŵ�������·���ϡ�ҵ��" << i << "�зŴ����ڽڵ�" << bus.multiplierNodeId[j] << "�ϣ��õ㲻��·����" << endl;
                return;
            }
        }
        //����·���㣨�����յ㣩����������һ���������ΪҪ��ȷ���Ŵ����Ƿ���·����
        curNode = bus.start;  //��ǰ����·�������
        int curA = D;  //�ź�ǿ��
        int pm = 0;  //�Ŵ���ָ��
        for (int j = 0; j < bus.path.size(); ++j)
        {
            //�ýڵ��зŴ���
            if (pm < bus.multiplierNodeId.size())  //����Խ�����
            {
                if (curNode == bus.multiplierNodeId[pm])
                {
                    curA = D;  //�����ź�ǿ��
                    ++pm;  //�ƶ��źŷŴ���ָ��
                }
            }
            //����ָ����һ�ڵ�
            if (edge[bus.path[j] * 2].from == curNode)
                curNode = edge[bus.path[j] * 2].to;
            else if (edge[bus.path[j] * 2 + 1].from == curNode)
                curNode = edge[bus.path[j] * 2 + 1].to;
            curA -= edge[bus.path[j] * 2].d;  //��һ�ڵ��ź�ǿ��
            //����һ����ʱ�ź���˥����������
            if (curA < 0)
            {
                cout << "�߼����󣺡����ź�δ��ʱ�Ŵ�ҵ��" << bus.busId << "�ź��ڽڵ�" << curNode << "��˥����������" << endl;
                return;
            }
        }
        //������������ҵ��ɱ�
        cost += (bus.path.size() * edgeCost + bus.multiplierNodeId.size() * multiplierCost);
    }
    //һ���������������
    cout << "�ɱ���" << cost << endl;
}

//���ļ��ж�ȡ����������ʹ�ùܵ�ͨ��ʵ�֣��ѷ�������Ϊ�ܵ�����˫��ͨ��
///**
//  * @brief	��ȡ��������
//  * @param	��
//  * @retval	��
//  */
//void readTestCase()
//{
//    int s, t, d;
//    // �򿪲��������ļ�
//    ifstream TestCase("TestCase.txt");
//    if (TestCase.is_open())
//    {
//        //��ȡ��������
//        TestCase >> N;
//        TestCase >> M;
//        TestCase >> T;
//        TestCase >> P;
//        TestCase >> D;
//        //��ȡ������
//        for (int i = 0; i < M; ++i)
//        {
//            TestCase >> s;
//            TestCase >> t;
//            TestCase >> d;
//            //�״μ�ĳ���ڵ�֮��ı�ʱ�������ڣ���̾�����INF
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
//            addEdge(s, t, d);  //˫���
//            addEdge(t, s, d);
//        }
//        //��ȡҵ������
//        for (int i = 0; i < T; ++i)
//        {
//            TestCase >> s;
//            TestCase >> t;
//            addBus(s, t);
//        }
//        // �ر��ļ�
//        TestCase.close();
//    }
//    else
//    {
//        cout << "���������󣺴�TestCase.txtʧ��" << endl;
//        system("pause");
//        exit(0);
//    }
//}
//
///**
//  * @brief	���������Ͳ�������
//  * @param	��
//  * @retval	��
//  */
//void sendTestCase()
//{
//    //��δ�˽⵽C++����д�ܵ��ķ���������ʹ��C����
//    //�򿪹ܵ�д�˲�ִ�н������
//    FILE* pipe = _popen("main.exe", "w");
//    if (!pipe) {
//        cout << "�������쳣���򿪹ܵ�ʧ��" << endl;
//        system("pause");
//        exit(0);
//    }
//    //��ʼ��ʱ
//    startTime = clock();
//    //���Ͳ�������
//    fprintf(pipe, "%d %d %d %d %d\n", N, M, T, P, D);
//    //������
//    for (int i = 0; i < M; ++i)
//    {
//        fprintf(pipe, "%d %d %d\n", edge[i * 2].from, edge[i * 2].to, edge[i * 2].d);
//    }
//    //ҵ������
//    for (int i = 0; i < T; ++i)
//    {
//        fprintf(pipe, "%d %d\n", buses[i].start, buses[i].end);
//    }
//}
//
///**
//  * @brief	���ս����������Ľ��
//  * @param	��
//  * @retval	��
//  */
//void receiveResult()
//{
//    //��δ�˽⵽C++������ܵ��ķ���������ʹ��C����
//    int s, t, p, m, n, temp;
//    //�򿪹ܵ�����
//    FILE* pipe = _popen("main.exe", "r");
//    if (!pipe) {
//        cout << "�������쳣���򿪹ܵ�ʧ��" << endl;
//        system("pause");
//        exit(0);
//    }
//    //��ȡ���
//    //�ӱ���
//    fscanf(pipe, "%d", &Y);
//    //�ӱ�
//    for (int i = 0; i < Y; ++i)
//    {
//        fscanf(pipe, "%d %d", &s, &t);
//        addEdge(s, t, minDist[make_pair(s, t)]);  //�±�˫���
//        addEdge(t, s, minDist[make_pair(t, s)]);
//    }
//    //ҵ����
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
//    // �رչܵ�
//    _pclose(pipe);
//}
