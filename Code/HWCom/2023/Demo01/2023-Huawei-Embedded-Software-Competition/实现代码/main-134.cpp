#include <bits/stdc++.h>
using namespace std; 
#define endl '\n'
typedef long long ll; 
int N,M,T,P,D;
//节点数量N,连边数量M,业务数量T,单边通道数量P,最大衰减距离D
int si,ti,pi;
struct node{
    int to,next,v,i;
}p[200005];
struct node1{
	int x,y;
	friend bool operator < (node1 a,node1 b){
		return a.y>b.y;
	}
	
};
//初始变量
double w=0;
int op,num_new,head[200005],flag;
int side[7005][3],work[20005][2],dis_min[5005][5005];
int channel[8005][100],ans_cha[80005],num_channel[80005];
//寻找路径变量
int vis[15005],fa[15005][2],dis[15005];
//加边变量
int dis_add[15005],vis_add[15005],min_dis=1e9,cha_new;
int fa_add[85][15005][2];
//结果
vector<pair<int,int> > vec_side[10005];//边
vector<int> vec_en[10005];//放大器
void add(int x,int y,int v,int i){
    p[++flag].to=y;
    p[flag].v=v;
    p[flag].i=i;
    p[flag].next=head[x];
    head[x]=flag;
}

void find(int x,int y,int cha){
    priority_queue<node1> q;
    for(int i=0;i<M;i++) vis[i]=0,dis[i]=1e9;
    dis[x]=0;
    q.push({x,0});
    while(!q.empty()){
        x=q.top().x;
        q.pop();
        if(vis[x]) continue;
        vis[x]++;
        for(int i=head[x];i;i=p[i].next){
            if(dis[p[i].to]>dis[x]+p[i].v+w*num_channel[p[i].i]&&channel[p[i].i][cha]==0){
                dis[p[i].to]=dis[x]+p[i].v+w*num_channel[p[i].i];
                if(op==0){
                 	fa[p[i].to][0]=x;
                	fa[p[i].to][1]=p[i].i;	
				}
                q.push({p[i].to,dis[p[i].to]});
            }
        }
    }
}
// 加边
void add_side(int x,int y){
	int x1=x;
	min_dis=1e9;
	for(int j=0;j<P;j++){
		priority_queue<node1> q;
		x=x1;
    	for(int i=0;i<M;i++) vis[i]=0,dis_add[i]=1e9;
    	dis_add[x]=0;
    	q.push({x,0});
    	while(!q.empty()){
        	x=q.top().x;
        	q.pop();
        	if(vis[x]) continue;
        	vis[x]++;
        	for(int i=head[x];i;i=p[i].next){
            	if(dis_add[p[i].to]>dis_add[x]+(channel[p[i].i][j]>0)){
                	dis_add[p[i].to]=dis_add[x]+(channel[p[i].i][j]>0);
                 	fa_add[j][p[i].to][0]=x;
                	fa_add[j][p[i].to][1]=p[i].i;	
                	q.push({p[i].to,dis_add[p[i].to]});
            	}
        	}
    	}
    	if(min_dis>dis_add[y]){
    		min_dis=dis_add[y];
    		cha_new=j;
		} 
	}
	x=x1;
	int h=y;
    while(1){
    	if(channel[fa_add[cha_new][h][1]][cha_new]==1){
    		M++;
    		int i=fa_add[cha_new][h][1]; 
    		num_new++;
    		add(side[i][0],side[i][1],dis_min[side[i][1]][side[i][0]],M-1);
    		add(side[i][1],side[i][0],dis_min[side[i][1]][side[i][0]],M-1);
            side[M-1][0]=side[i][0];
            side[M-1][1]=side[i][1];
            side[M-1][2]=dis_min[side[i][1]][side[i][0]];
		}
        h=fa_add[cha_new][h][0];
        if(h==x) break;
    }
    
}
int main(){
    cin>>N>>M>>T>>P>>D;
    for(int i=0;i<N;i++)
        for(int t=0;t<N;t++){
            dis_min[i][t]=1e9;
        }
    for(int i=0;i<M;i++){
        cin>>si>>ti>>pi;
        dis_min[si][ti]=min(dis_min[si][ti],pi);
        dis_min[ti][si]=dis_min[si][ti];
        side[i][0]=si,side[i][1]=ti,side[i][2]=pi;
        add(si,ti,pi,i);
        add(ti,si,pi,i);
    }
    for(int i=0;i<T;i++){
        cin>>si>>ti;
        work[i][0]=si;
        work[i][1]=ti;
    }
    for(int i=0;i<T;i++){
        int check=0;
        for(int t=0;t<P;t++){
            find(work[i][0],work[i][1],t);
            if(dis[work[i][1]]!=1e9){
            	check=1,ans_cha[i]=t;
				break;	
			}
        }
        //进行加边
        if(!check){
        	add_side(work[i][0],work[i][1]);
        	find(work[i][0],work[i][1],cha_new);
        	ans_cha[i]=cha_new;
        }
        //记录路径 
        int h=work[i][1];
        while(1){
            vec_side[i].push_back({fa[h][1],fa[h][0]});
            channel[fa[h][1]][ans_cha[i]]=1;
            h=fa[h][0];
            if(h==work[i][0]){
            	break;
			}
        }
        
        //记录放大器
        vec_en[i].push_back(0);
        h=D;
        for(int t=vec_side[i].size()-1;t>=0;t--){
            if(h-side[vec_side[i][t].first][2]<0){
            	vec_en[i][0]++,vec_en[i].push_back(vec_side[i][t].second);
            	h=D;
			}
			h-=side[vec_side[i][t].first][2]; 
        }
    }
    //输出结果
    cout<<num_new<<endl;
    for(int i=0;i<num_new;i++) cout<<side[M-num_new+i][0]<<" "<<side[M-num_new+i][1]<<endl;
    for(int i=0;i<T;i++){
        cout<<ans_cha[i]<<" "<<vec_side[i].size()<<" "<<vec_en[i][0];
        for(int t=vec_side[i].size()-1;t>=0;t--) cout<<" "<<vec_side[i][t].first;
        for(int t=1;t<=vec_en[i][0];t++) cout<<" "<<vec_en[i][t];
        cout<<endl;
    }
    

    
}
