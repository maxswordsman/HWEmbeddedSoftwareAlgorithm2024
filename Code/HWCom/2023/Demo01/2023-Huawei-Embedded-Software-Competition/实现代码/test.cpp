#include <bits/stdc++.h>
using namespace std; 
#define endl '\n'
typedef long long ll; 
int N,M,T,P,D;
//�ڵ�����N,��������M,ҵ������T,����ͨ������P,���˥������D
int si,ti,pi;
struct node{
    int to,next,v,i;
}p[50005];
struct node1{
	int x,y;
	friend bool operator < (node1 a,node1 b){
		return a.y>b.y;
	}
	
};
//��ʼ����
int w=15;
int op,num_new,head[5005],flag,work[20005][2],ee;
int side[25005][3],dis_min[7005][7005];
int channel[25005][85],ans_cha[10005];
//Ѱ��·������
int vis[5005],fa[85][5005][2],dis[5005],cha_find,min_dis_find,check_find,cost[10005];
//�ӱ߱���
int dis_add[5005],vis_add[5005],min_dis=1e9,cha_add;
int fa_add[85][5005][2];
//���
vector<pair<int,int> > vec_side[10005];//��
vector<int> vec_en[10005];//�Ŵ���
void add(int x,int y,int v,int i){
    p[++flag].to=y;
    p[flag].v=v;
    p[flag].i=i;
    p[flag].next=head[x];
    head[x]=flag;
}
//Ѱ�ҿ���·��
void find(int x,int y){
    int x1=x;
	min_dis_find=1e9;
    int time=0;
    for(int j=0;j<P;j++){
        x=x1;
        priority_queue<node1> q;
        for(int i=0;i<M;i++) vis[i]=0,dis[i]=1e9;
        dis[x]=0;
        q.push({x,0});
        while(!q.empty()){
            x=q.top().x;
            q.pop();
            if(x==y){
            	break;
			}
            if(vis[x]) continue;
            vis[x]++;
            for(int i=head[x];i;i=p[i].next){
                if(dis[p[i].to]>dis[x]+p[i].v+w*channel[p[i].i][P]&&channel[p[i].i][j]==0){
                    dis[p[i].to]=dis[x]+p[i].v+w*channel[p[i].i][P];
                 	fa[j][p[i].to][0]=x;
                	fa[j][p[i].to][1]=p[i].i;	
                    q.push({p[i].to,dis[p[i].to]});
                }
            }
        }
        if(dis[y]!=1e9){
            time++;
        }
        if(min_dis_find>dis[y]){
    		min_dis_find=dis[y];
    		cha_find=j;
		} 
        if(time>=54) return;
    }
}
//������ټӱ�
int branch(int x,int y,int si,int x1,int y1){
	int check=0;
	int temp_cha[M+5];
	int i=channel[si][cha_add]-1;
	//����һ��cha_addͨ���±ߵ�ռ�������֮�������ָ�
	for(int i=0;i<M;i++) temp_cha[i]=channel[i][cha_add];
	//��ռ��si�ߵ�ҵ������ͷ�
	for(int t=vec_side[i].size()-1;t>=0;t--){
        channel[vec_side[i][t].first][ans_cha[i]]=0;
	}
	//���ӱ�·������ռ�У���ֹ�ͷŵ�ҵ��ʹ��
	int h=y1;
	while(1){
		channel[fa_add[cha_add][h][1]][cha_add]=1;
        h=fa_add[cha_add][h][0];
        if(h==x1) break;
    }
	//���ͷ�ҵ�����·���·��
	find(work[i][0],work[i][1]);
    if(min_dis_find!=1e9){
		//�ͷ�ԭ��·���ͷŴ���
   	    ans_cha[i]=cha_find;
   	    vec_side[i].clear();
		vec_en[i].clear();
		//��¼·�� 
    	int h=work[i][1];
    	while(1){
   			vec_side[i].push_back({fa[ans_cha[i]][h][1],fa[ans_cha[i]][h][0]});
    	    channel[fa[ans_cha[i]][h][1]][ans_cha[i]]=i+1;
        	channel[fa[ans_cha[i]][h][1]][P]++;
        	cost[i]++;
        	h=fa[ans_cha[i]][h][0];
        	if(h==work[i][0]){
          		break;
			}
    	}
    	//��¼�Ŵ���
    	vec_en[i].push_back(0);
    	h=D;
    	for(int t=vec_side[i].size()-1;t>=0;t--){
        	if(h-side[vec_side[i][t].first][2]<0){
   	    		vec_en[i][0]++,vec_en[i].push_back(vec_side[i][t].second);
           		h=D;
           		cost[i]+=100;
			}
			h-=side[vec_side[i][t].first][2]; 
    	}
   	    check=1;
		//֮ǰ©������������ͷűߣ�
   	    temp_cha[si]=0;
	}
	//����·�������ԭ�����лָ�
	else{
		for(int t=vec_side[i].size()-1;t>=0;t--){
       	 	channel[vec_side[i][t].first][ans_cha[i]]=i+1;
		}
	}
	//��·��ռ�н��лָ�
	h=y1;
	while(1){
		channel[fa_add[cha_add][h][1]][cha_add]=temp_cha[fa_add[cha_add][h][1]];
        h=fa_add[cha_add][h][0];
        if(h==x1) break;
    }
	return check;
}
// �ӱ�
void add_side(int x,int y){
	int x1=x;
	min_dis=1e9;
	int min_sum=1e9;
	for(int j=0;j<P;j++){
		int dis_sum[5005];
    	for(int i=0;i<=5005;i++) dis_sum[i]=0;
		priority_queue<node1> q;
		x=x1;
    	for(int i=0;i<M;i++) vis[i]=0,dis_add[i]=1e9;
    	dis_add[x]=0;
    	q.push({x,0});
    	while(!q.empty()){
        	x=q.top().x;
        	q.pop();
        	if(x==y) break;
        	if(vis[x]) continue;
        	vis[x]++;
        	for(int i=head[x];i;i=p[i].next){
            	if(dis_add[p[i].to]>dis_add[x]+(channel[p[i].i][j]>0)){
                	dis_add[p[i].to]=dis_add[x]+(channel[p[i].i][j]>0);
                	dis_sum[p[i].to]=dis_sum[x]+p[i].v;
                 	fa_add[j][p[i].to][0]=x;
                	fa_add[j][p[i].to][1]=p[i].i;	
                	q.push({p[i].to,dis_add[p[i].to]});
            	}
        	}
    	}
    	if(min_dis>dis_add[y]){
    		min_dis=dis_add[y];
    		cha_add=j;
		} 
		else if(min_dis==dis_add[y]&&min_sum>dis_sum[y]){
			min_dis=dis_add[y];
			min_sum=dis_sum[y];
    		cha_add=j;
		}
	}
	x=x1;
	int h=y;
    while(1){
    	if(channel[fa_add[cha_add][h][1]][cha_add]){
    		int re1=channel[fa_add[cha_add][h][1]][cha_add]-1;
			if(branch(work[re1][0],work[re1][1],fa_add[cha_add][h][1],x,y)==0){
				M++;
    			int i=fa_add[cha_add][h][1]; 
    			num_new++;
    			add(side[i][0],side[i][1],dis_min[side[i][1]][side[i][0]],M-1);
    			add(side[i][1],side[i][0],dis_min[side[i][1]][side[i][0]],M-1);
            	side[M-1][0]=side[i][0];
            	side[M-1][1]=side[i][1];
            	side[M-1][2]=dis_min[side[i][1]][side[i][0]];
			}
    		
		}
        h=fa_add[cha_add][h][0];
        if(h==x) break;
    }
    
}


//���Գ���
//���Ʋ���TEST��
int TEST = 5;
void RandomData(){
	//��¼�ܷ������ܺ�ʱ
	ll totalScore = 0,totalTime = 0;
	for (int h = 0; h < TEST; h++)
	{
		op=0,num_new=0,flag=0;
	for(int i=0;i<25005;i++)
		for(int t=0;t<85;t++)
			channel[i][t]=0;	
	for(int i=0;i<10005;i++){
		cost[i]=0;
		vec_side[i].clear();//��
		vec_en[i].clear();
	} 
	
	 
	//��ʼ��i�β���
	//��������
	int seed = time(0);
	cout<<"���β�������:"<<seed<<endl;
	srand(seed);
    N = 2+rand()%4999;
    M = N-1+rand()%(5000-N+2);
    T = 2+rand()%9999;
    P = 2+rand()%79;
    D = 2+rand()%999;
	for(int i=0;i<N;i++)
        for(int t=0;t<N;t++){
            dis_min[i][t]=1e9;
        }
	//������������ݣ��ұ�֤ͼ����ͨ��
	for(int i=0;i<M;i++){
		if(i<N-1){
			si = i;ti = i+1;pi = rand() % (D-1);
		}else{
			si = rand()%N;ti = rand()%N;
			if(si==ti){
				if(si==N-1)si--;
				else si++;
			}
        	pi = rand() % (D-1);
		}

        dis_min[si][ti]=min(dis_min[si][ti],pi);
        dis_min[ti][si]=dis_min[si][ti];
        side[i][0]=si,side[i][1]=ti,side[i][2]=pi;
        add(si,ti,pi,i);
        add(ti,si,pi,i);
    }
	//�������ҵ������
	for(int i=0;i<T;i++){
		si = rand()%N;ti = rand()%N;
		if(si==ti){
			if(si==N-1)si--;
			else si++;
		}
		work[i][0]=si;
        work[i][1]=ti;
	}
    cout<<"�Ѿ������������"<<N<<"������"<<M<<"����"<<T<<"��ҵ��"<<P<<"��ͨ��"<<D<<"˥������"<<endl;
    clock_t begin = clock(),end;
    for(int i=0;i<T;i++){
		// cout<<"�����"<<i<<"��ҵ��"<<"��������������"<<T<<"��ҵ��"<<endl;
        int check=0;
        find(work[i][0],work[i][1]);
        if(min_dis_find!=1e9){
            check=1,ans_cha[i]=cha_find;
		}
        //���мӱ�
         if(!check){
        	add_side(work[i][0],work[i][1]);
        	find(work[i][0],work[i][1]);
        	ans_cha[i]=cha_find;
        }
        //��¼·�� 
        int h=work[i][1];
        while(1){
            vec_side[i].push_back({fa[ans_cha[i]][h][1],fa[ans_cha[i]][h][0]});
            channel[fa[ans_cha[i]][h][1]][ans_cha[i]]=i+1;
            channel[fa[ans_cha[i]][h][1]][P]++;
            cost[i]++;
            h=fa[ans_cha[i]][h][0];
            if(h==work[i][0]){
            	break;
			}
        }
        //��¼�Ŵ���
        vec_en[i].push_back(0);
        h=D;
        for(int t=vec_side[i].size()-1;t>=0;t--){
            if(h-side[vec_side[i][t].first][2]<0){
            	vec_en[i][0]++,vec_en[i].push_back(vec_side[i][t].second);
            	h=D;
            	cost[i]+=100;
			}
			h-=side[vec_side[i][t].first][2]; 
        }
    }
    //find_better_way();
    //�ݲ�������
    // cout<<num_new<<endl;
    // for(int i=0;i<num_new;i++) cout<<side[M-num_new+i][0]<<" "<<side[M-num_new+i][1]<<endl;
    // for(int i=0;i<T;i++){
    //     cout<<ans_cha[i]<<" "<<vec_side[i].size()<<" "<<vec_en[i][0];
    //     for(int t=vec_side[i].size()-1;t>=0;t--) cout<<" "<<vec_side[i][t].first;
    //     for(int t=1;t<=vec_en[i][0];t++) cout<<" "<<vec_en[i][t];
    //     cout<<endl;
    // }
    end = clock();
    cout<<"���� time used:"<<(double)(end-begin)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
	totalTime+=((double)(end-begin)/CLOCKS_PER_SEC*1000/1000);
	for(int i=0;i<T;i++){
		totalScore+=vec_side[i].size()+vec_en[i][0]*100;
		}
		totalScore+=(num_new*1000000)*1.0;
	}
	cout<<"ƽ��������"<<totalScore/TEST<<endl;
	cout<<"ƽ����ʱ��"<<totalTime<<endl;
}

int main(){
	//��ʼ����
	RandomData();
	return 1;

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
        find(work[i][0],work[i][1]);
        if(min_dis_find!=1e9){
            check=1,ans_cha[i]=cha_find;
		}
        //���мӱ�
         if(!check){
        	add_side(work[i][0],work[i][1]);
        	find(work[i][0],work[i][1]);
        	ans_cha[i]=cha_find;
        }
        //��¼·�� 
        int h=work[i][1];
        while(1){
            vec_side[i].push_back({fa[ans_cha[i]][h][1],fa[ans_cha[i]][h][0]});
            channel[fa[ans_cha[i]][h][1]][ans_cha[i]]=i+1;
            channel[fa[ans_cha[i]][h][1]][P]++;
            cost[i]++;
            h=fa[ans_cha[i]][h][0];
            if(h==work[i][0]){
            	break;
			}
        }
        //��¼�Ŵ���
        vec_en[i].push_back(0);
        h=D;
        for(int t=vec_side[i].size()-1;t>=0;t--){
            if(h-side[vec_side[i][t].first][2]<0){
            	vec_en[i][0]++,vec_en[i].push_back(vec_side[i][t].second);
            	h=D;
            	cost[i]+=100;
			}
			h-=side[vec_side[i][t].first][2]; 
        }
    }
    //find_better_way();
    //������
    cout<<num_new<<endl;
    for(int i=0;i<num_new;i++) cout<<side[M-num_new+i][0]<<" "<<side[M-num_new+i][1]<<endl;
    for(int i=0;i<T;i++){
        cout<<ans_cha[i]<<" "<<vec_side[i].size()<<" "<<vec_en[i][0];
        for(int t=vec_side[i].size()-1;t>=0;t--) cout<<" "<<vec_side[i][t].first;
        for(int t=1;t<=vec_en[i][0];t++) cout<<" "<<vec_en[i][t];
        cout<<endl;
    }
}
