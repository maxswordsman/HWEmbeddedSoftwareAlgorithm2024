#include <bits/stdc++.h>

using namespace std;

/*
 * bfs2中DFS找路径的方法
 * */
/*
class Node {
public:
    int id;  // 节点编号
    int channel_change_capacity;  // 变通道次数--初始化之后就不会变化
    int usedChannelCnt;           // 该节点变通道次数被使用计数

    // 构造函数
    Node(int id = 0, int capacity = 0) : id(id), channel_change_capacity(capacity) ,usedChannelCnt(0){}
};

vector<Node> nodes;

void addNode(vector<Node> &nodes, int id, int capacity)
{
    nodes.emplace_back(id, capacity);
}

int serviceStart = 1;
int serviceEnd = 5;
int node = serviceEnd;

vector<int> pathNode;
vector<int> pathEdge;
vector<int> pathChan;


vector<vector<int>> parent = {{-1},{1},{1},{2,2},{3,3},{4},{4}};
vector<vector<int>> parentEdge = {{-1},{-1},{1},{2,3},{4,4},{5},{6}};;
vector<vector<int>> usedChan = {{-1},{35},{35},{35,37},{35,37},{37},{37}};;


bool dfs(int node, int &serviceStart, int &serviceEnd, vector<int> &pathNode,
         vector<int> &pathEdge, vector<int> &pathChan,
         vector<vector<int>> &parent, vector<vector<int>> &parentEdge, vector<vector<int>> &usedChan,
         int channel)
{
    pathNode.push_back(node);
    if(node == serviceStart)
        return true;

    for(int index = 0; index < parent[node].size(); index++)
    {
        if(node == serviceStart)
        {
            pathEdge.push_back(parentEdge[node][index]);
            pathChan.push_back(usedChan[node][index]);
            dfs(parent[node][index], serviceStart, serviceEnd, pathNode,
                pathEdge, pathChan,parent, parentEdge, usedChan,
            usedChan[node][index]);
        }
        else
        {
            if(channel == usedChan[node][index])
            {
                pathEdge.push_back(parentEdge[node][index]);
                pathChan.push_back(usedChan[node][index]);
                if(dfs(parent[node][index], serviceStart, serviceEnd, pathNode,
                       pathEdge, pathChan,parent, parentEdge, usedChan,
                    usedChan[node][index]))
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
                    if(dfs(parent[node][index], serviceStart, serviceEnd, pathNode,
                           pathEdge, pathChan,parent, parentEdge, usedChan,
                        usedChan[node][index]))
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


int main()
{
    nodes.emplace_back();
    addNode(nodes, 1, 0);
    addNode(nodes, 2, 1);
    addNode(nodes, 3, 0);
    addNode(nodes, 4, 0);
    addNode(nodes, 5, 0);
    addNode(nodes, 6, 0);

    for(int index = 0; index < parent[node].size(); index++)
    {
        if(dfs(node, serviceStart, serviceEnd, pathNode,
               pathEdge, pathChan, parent, parentEdge, usedChan,
               usedChan[node][index]))
        {
            cout << "path Nodes: ";
            for(auto n: pathNode)
            {
                cout << n << " ";
            }
            cout << endl;

            cout << "path Edges: ";
            for(auto e: pathEdge)
            {
                cout << e << " ";
            }
            cout << endl;

            cout << "channels: ";
            for(auto c: pathChan)
            {
                cout << c << " ";
            }
            cout << endl;
            break;
        }
    }
}
*/

int find_min_false_range(const std::vector<bool>& vec, int width) {
    int n = vec.size();
    int min_length = -1;
    int start_index = -1;

    for (int i = 0; i <= n - width; ++i)
    {
        if (vec[i] == false)
        {
            int j = i;
            while (j < n && vec[j] == false)
            {
                ++j;
            }
            int length = j - i;
            if (length >= width && length < min_length)
            {
                min_length = length;
                start_index = i;
            }
            i = j;
        }
    }

    return start_index;
}

int find_min(const std::vector<bool>& vec, int width)
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



int main() {
    std::vector<bool> vec = {true, true};
    int width = 1;
    int result = find_min(vec, width);
    cout << result <<endl;
    return 0;
}