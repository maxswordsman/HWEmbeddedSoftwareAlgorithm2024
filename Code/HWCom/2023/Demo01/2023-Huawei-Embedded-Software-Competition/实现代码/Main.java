
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;
//开始进行Dijstra算法
public class Main {
    public static void main(String[] args) {
        random();
    }
    public static void compete(){
        //获取数据
        Scanner scan = new Scanner(System.in);
        int[] Total = new int[5];
        for (int i = 0; i < 5; i++) {
            Total[i] = scan.nextInt();
        }
        int[][] edges = new int[Total[1]][3];
        int[][] txs = new int[Total[2]][2];
        for (int i = 0; i < Total[1]; i++) {
            edges[i] = new int[]{scan.nextInt(),scan.nextInt(),scan.nextInt()};
        }
        for (int i = 0; i < Total[2]; i++) {
            txs[i] = new int[]{scan.nextInt(),scan.nextInt()};
        }
        algorithm(Total,edges,txs);
    }
    public static void random(){
        //随机生成数据
        int[] Total = new int[5];
        Total[0] = (int)(Math.random()*4998)+2;
        Total[1] = (int)(Math.random()*(5000-Total[0]))+Total[0];
        Total[2] = (int)(Math.random()*9998)+2;
        Total[3] = (int)(Math.random()*78)+2;
        Total[4] = (int)(Math.random()*998)+2;
        int[][] edges = new int[Total[1]][3];
        int[][] txs = new int[Total[2]][2];
        for (int i = 0; i < Total[0]-1; i++) {
            edges[i][0] = i;
            edges[i][1] = i+1;
            edges[i][2] = (int)(Math.random()*(Total[4]-1))+1;
        }
        for (int i = Total[0]-1; i < edges.length; i++) {
            edges[i][0] = (int)(Math.random()*(Total[0]-1));
            edges[i][1] = (int)(Math.random()*(Total[0]-1));
            if (edges[i][0]==edges[i][1]){
                edges[i][1]++;
            }
            edges[i][2] = (int)(Math.random()*(Total[4]-1))+1;
        }
        for (int i = 0; i < txs.length; i++) {
            txs[i][0] = (int)(Math.random()*(Total[0]-1));
            txs[i][1] = (int)(Math.random()*(Total[0]-1));
            if (txs[i][0]==txs[i][1]){
                txs[i][1]++;
            }
        }
        System.out.println("无向连通图已经生成！");
        for (int i = 0; i < 5; i++) {
            System.out.print("\t"+Total[i]);
        }
        System.out.println();
        long startTime=System.currentTimeMillis();   //获取开始时间
        algorithm(Total,edges,txs);
        long endTime=System.currentTimeMillis(); //获取结束时间
        System.out.println("程序运行时间： "+(endTime-startTime)+"ms");
    }
    public static void algorithm(int[] Total,int[][] edges,int[][] txs){
        Graph graph = new Graph(edges,Total,txs);
        //开始执行
        graph.handleTx(txs);
    }
}
//权值计算公式：W = (distance*w1+(used/capacity)*w2+nodes*w3)/edges;
class Graph{
    //图中的属性矩阵：包括两点之间的距离、边数、已经使用的通道数
    HashMap<Integer,HashMap<Integer,Multilateral>> network;
    int[][] W;
    int N,M,T,P,D;
    ArrayList<int[]> addEdge,resPath;
    public Graph(int[][] edges,int[] Total,int[][] txs){
        addEdge = new ArrayList<>();
        resPath = new ArrayList<>();
        network = new HashMap<>();
        N = Total[0];
        M = Total[1];
        T = Total[2];
        P = Total[3];
        D = Total[4];
        //两点间最短距离、已经使用的通道数量、两点间的边数
        W = new int[N][N];
        //初始化图
        for (int i = 0; i < edges.length; i++) {
            int point1,point2;
            point1 = edges[i][0];
            point2 = edges[i][1];
            if(!network.containsKey(point1)){
                HashMap<Integer,Multilateral> end = new HashMap<>();
                network.put(point1,end);
            }
            if(!network.containsKey(point2)){
                HashMap<Integer,Multilateral> end = new HashMap<>();
                network.put(point2,end);
            }
            Multilateral multi;
            if(!network.get(point1).containsKey(point2)){
                multi = new Multilateral(point1,point2);
                network.get(point1).put(point2,multi);
                network.get(point2).put(point1,multi);
            }else{
                multi = network.get(point1).get(point2);
            }
            Edge edge = new Edge(point1,point2,edges[i][2],i,Total[3]);
            //将一条边加入两个数组
            multi.AddEdge(edge);
        }
        initWeight(W);
    }
    public void handleTx(int[][] txs){
        //开始执行业务
        for (int i = 0; i < txs.length; i++) {
            int[] arr = dijkstra(W,txs[i][0],txs[i][1]);
            int[] path = new int[arr[arr.length-1]+1];
            int length = path.length-1;
            for(int node = txs[i][1];node!=txs[i][0]&&length>=0;){
                path[length--] = node;
                node = arr[node];
            }
            path[0] = txs[i][0];
            int[] channels = new int[P];
            int[] pair = new int[2];
            pair[0] = -1;
            pair[1] = -1;
            ArrayList<Edge> tx_path = new ArrayList<>();
            for (int j = 0; j < path.length-1; j++) {
                int start,end;
                start = path[j];
                end = path[j+1];
                //先选择边，再选择通道
                Edge edge = network.get(start).get(end).getPriority();
                tx_path.add(edge);
                for (int k = 0; k < channels.length; k++) {
                    channels[k]+=edge.pass[k]?0:1;
                    if(channels[k]>pair[1]){
                        pair[1] = channels[k];
                        pair[0] = k;
                    }
                }
            }
            //开始进行更新地图
            UpdatePath(tx_path,txs[i][0],pair[0]);
        }
        PrintResult();
    }
    public void initWeight(int[][] W){
        //初始化图的权重矩阵
        for (int i = 0; i < N; i++) {
            for (int j = i+1; j < N; j++) {
                if(i==j){
                    continue;
                }
                if(!network.get(i).containsKey(j)){
                    //说明两个节点不连接
                    W[i][j] = -1;
                    W[j][i] = -1;
                }else{
                    int weight = network.get(i).get(j).getWeight();
                    W[i][j] = weight;
                    W[j][i] = weight;
                }
            }
        }
    }
    public void PrintResult(){
        if (addEdge.size()>0){
            System.out.println(addEdge.size());
        }
        for (int i = 0; i < addEdge.size(); i++) {
            int[] arr = addEdge.get(i);
            System.out.println(arr[0]+" "+arr[1]);
        }
        for (int i = 0; i < resPath.size(); i++) {
            int[] path = resPath.get(i);
            for (int j = 0; j < path.length; j++) {
                System.out.print(path[j]);
                if(j!=path.length-1){
                    System.out.print(" ");
                }
            }
            if(i<resPath.size()-1){
                System.out.println();
            }
        }
    }
    //Dijkstra算法寻找最优路径
    public static int[] dijkstra(int[][] W1, int start, int end) {
        boolean[] isLabel = new boolean[W1[0].length];// 是否标号
        int[] indexs = new int[W1[0].length];// 所有标号的点的下标集合，以标号的先后顺序进行存储，实际上是一个以数组表示的栈
        int i_count = -1;//栈的顶点
        int[] distance = W1[start].clone();// v0到各点的最短距离的初始值
        int index = start;// 从初始点开始
        int presentShortest = 0;//当前临时最短距离
        //记录到达此点的前一个点
        int[] path = new int[W1[0].length+1];
        for (int i = 0; i < distance.length; i++) {
            if(distance[i]!=-1){
                path[i] = start;
            }
        }
        indexs[++i_count] = index;// 把已经标号的下标存入下标集中
        isLabel[index] = true;
        while (i_count<W1[0].length-1) {
            // 第一步：标号v0,即w[0][0]找到距离v0最近的点
            int min = Integer.MAX_VALUE;
            for (int i = 0; i < distance.length; i++) {
                if (!isLabel[i] && distance[i] != -1 && i != index) {
                    // 如果到这个点有边,并且没有被标号
                    if (distance[i] < min) {
                        min = distance[i];
                        index = i;// 把下标改为当前下标
                    }
                }
            }
            if (index == end) {//已经找到当前点了，就结束程序
//                System.out.println("已经搜索到路径！");
                int cnt = 0;
                for(int node = end;node!=start&&cnt<5001;cnt++){
                    node = path[node];
                    path[path.length-1]++;
                }
                if(cnt==5001){
//                    System.out.println("错误！");
                    path[path.length-1] = -1;
                }
                break;
            }
            isLabel[index] = true;//对点进行标号
            indexs[++i_count] = index;// 把已经标号的下标存入下标集中
            if (W1[indexs[i_count - 1]][index] == -1
                    || presentShortest + W1[indexs[i_count - 1]][index] > distance[index]) {
                // 如果两个点没有直接相连，或者两个点的路径大于最短路径
                presentShortest = distance[index];
            } else {
                presentShortest += W1[indexs[i_count - 1]][index];
            }
            // 第二步：将distance中的距离加入vi
            for (int i = 0; i < distance.length; i++) {
                // 如果vi到那个点有边，则v0到后面点的距离加
                if (distance[i] == -1 && W1[index][i] != -1) {// 如果以前不可达，则现在可达了
                    distance[i] = presentShortest + W1[index][i];
                    path[i] = index;
                } else if (W1[index][i] != -1
                        && presentShortest + W1[index][i] < distance[i]) {
                    // 如果以前可达，但现在的路径比以前更短，则更换成更短的路径
                    distance[i] = presentShortest + W1[index][i];
                    path[i] = index;
                }
            }
        }
//        System.out.println(distance[start]);
        //如果全部点都遍历完，则distance中存储的是开始点到各个点的最短路径
        return path;
    }
    //更新路径上的结构
    public void UpdatePath(ArrayList<Edge> path,int start,int pass){
        int previous = D;
        ArrayList<Integer> magnify = new ArrayList<>();
        ArrayList<Integer> passage = new ArrayList<>();
        int point = start;
        for (int i = 0; i < path.size(); i++) {
            Edge edge = path.get(i);
            if (previous<= edge.d){
                previous = D;
                //安装一个放大器
                magnify.add(point);
            }
            previous-=edge.d;
            if(edge.pass[pass]){
                //需要添加边
                int end = point==edge.s?edge.t:edge.s;
                int s,e;
                s = point<end?point:end;
                e = point>end?point:end;
                Edge newEdge = new Edge(s,e,network.get(s).get(e).distance,M,P);
                M++;
                newEdge.pass[pass] = true;
                network.get(s).get(e).AddEdge(newEdge);
                addEdge.add(new int[]{s,e});
                passage.add(newEdge.id);
            }else{
                edge.pass[pass] = true;
                passage.add(edge.id);
            }
            network.get(edge.s).get(edge.t).used++;
            int weight = network.get(edge.s).get(edge.t).getWeight();
            W[edge.s][edge.t] = weight;
            W[edge.t][edge.s] = weight;
            point = edge.s==point?edge.t:edge.s;
        }
        int[]arr = new int[3+magnify.size()+passage.size()];
        arr[0] = pass;
        arr[1] = passage.size();
        arr[2] = magnify.size();
        int k = 3;
        for (int i = 0; i < passage.size(); i++) {
            arr[k] = passage.get(i);
            k++;
        }
        for (int i = 0; i < magnify.size(); i++) {
            arr[k] = magnify.get(i);
            k++;
        }
        resPath.add(arr);
    }
}
class Edge{
    //分别为：起点、终点、距离、ID、通道占用情况
    int s;
    int t;
    int d;
    int id;
    boolean[] pass;
    public Edge(int s_,int t_,int d_,int id_,int p){
        s = s_;
        t = t_;
        d = d_;
        id = id_;
        pass = new boolean[p];
    }
    public void Print(){
        System.out.println("开始打印边的信息：");
        System.out.println(s);
        System.out.println(t);
        System.out.println(d);
        System.out.println(id);
        for (int i = 0; i < pass.length; i++) {
            System.out.println(pass[i]);
        }
    }
    //获取可用channel的数量
    public int getChannelNum(){
        int ans = 0;
        for (int i = 0; i < pass.length; i++) {
            if (!pass[i]){
                ans++;
            }
        }
        return ans;
    }
}

//表示两个顶点间的多条边
class Multilateral{
    //两个顶点间的最小边
    int distance;
    int point1,point2;
    ArrayList<Edge> edges;
    int used,capacity;
    public Multilateral(int p1,int p2){
        capacity = 0;
        distance = 1001;
        point1 = p1;
        point2 = p2;
        edges = new ArrayList<>();
    }
    public void AddEdge(Edge edge){
        if(edge.d<distance){
            distance = edge.d;
        }
        //增加两个顶点间的通道数
        capacity+=edge.pass.length;
        edges.add(edge);
    }
    public void Print(){
        System.out.println(point1+"->"+point2);
        System.out.println("最短距离为："+distance);
        for (int i = 0; i < edges.size(); i++) {
            Edge edge = edges.get(i);
            edge.Print();
        }
    }
    //获取
    public Edge getPriority(){
        Edge ans = null;
        int num = -1;
        for (int i = 0; i < edges.size(); i++) {
            Edge edge = edges.get(i);
            int cnt = edge.getChannelNum();
            if(cnt>num){
                ans = edge;
                num = cnt;
            }
        }
        return ans;
    }
    //获取两个顶点间的权重
    public int getWeight(){
        int weight = (distance*100+(used*10000/capacity)+1*1)/edges.size();
        if(weight==1){
            System.out.println("error");
            System.out.println(distance+"-"+used+"/"+capacity+"-"+edges.size());
        }
        return weight;
    }
}
