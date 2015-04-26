#include<iostream>
#include<cstdio>
#include<vector>
#include<map>
#include<set>
#include<cstdlib>
#include<algorithm>
#include<fstream>
#include<cstring>
#include<cmath>

using namespace std;

const int MAX = 1000;

class UserCF
{
	private:
		int k,m; // most k similar users; top m brands
		map<int,int> userid_id;
		map<int,int> id_userid;
		map< int,set<int> > user_brands;
		map< int,set<int> > id_brands;
		map< int,set<int> > brand_ids;
		set<int> brand_all;
		map< int,set<int> > user_brand_rec;

		ifstream fin;
		ofstream fout;

		double sim_mat[MAX][MAX];

		typedef struct sim_idx
		{
			double sim;
			int idx;
			bool operator > (const sim_idx &other) const
			{
				return sim > other.sim;
			}
		} sim_idx;
		typedef struct brand_interest 
		{
			double Int; // Int = interest 
			int brand;
			bool operator > (const brand_interest &other) const
			{
				return Int > other.Int;
			}
		} brand_interest;

	public:
		UserCF(int _k, int _m):k(_k),m(_m)
	{
		fin.open("user_brand_m123.txt");
		fout.open("rec_result_by_m123.txt");
		if (!fin || !fout)
		{
			cout << "can not open the file" << endl;
			exit(1);
		}

		//userid_id, id_userid, user_brands, brand_all
		int userid, brandid, i = 0;
		while (fin >> userid >> brandid)
		{
			if (user_brands[userid].empty())
			{
				// 将用户userid映射为0~n-1的id,方便矩阵计算
				userid_id[userid] = i;
				id_userid[i] = userid; 
				++i;
			}
			user_brands[userid].insert(brandid);
			brand_all.insert(brandid);
		}

		// id_brands
		map< int, set<int> >::iterator it = user_brands.begin();
		while ( it != user_brands.end() )
		{
			id_brands[ userid_id[it->first] ] = it->second;
			++it;
		}
		memset(sim_mat,0,sizeof(sim_mat));
	}

		// brand_ids - brand : userid1, userid2...
		void get_reverse_table()
		{
			map< int, set<int> >::iterator it = id_brands.begin();
			while (it != id_brands.end())
			{
				set<int> tmp = it->second;
				set<int>::iterator it2 = tmp.begin();
				while (it2 != tmp.end())
				{
					brand_ids[*it2].insert(it->first);
					++it2;
				}
				++it;
			}
		}

		// 获得用于相似度矩阵
		void get_sim_mat()
		{
			get_reverse_table();
			map< int, set<int> >::iterator it = brand_ids.begin();
			while (it != brand_ids.end())
			{
				// brand下的ids为tmp
				vector<int> tmp( it->second.begin(), it->second.end());
				int len = tmp.size();
				for (int i = 0; i < len; ++i)
					for (int j = i + 1; j < len; ++j)
					{
						sim_mat[tmp[i]][tmp[j]] += 1;
						sim_mat[tmp[j]][tmp[i]] += 1;
					}
				++it;
			}

			int len = id_userid.size();
			for (int i = 0; i < len; ++i)
				for (int j = 0; j < len; ++j)
				{
					// 另存有序id的作用体现在这
					sim_mat[i][j] /= sqrt( user_brands[ id_userid[i] ].size() * user_brands[ id_userid[j] ].size() );
					sim_mat[j][i] = sim_mat[i][j];
				}
			cout << endl;
		}

		set<int> get_rec_brand_set_by_user(int userid)
		{
			set<int> brand_unused;
			// set_difference: find different set between two set
			// function: get brand set that userid has never bought before
			//set_difference(brand_all.begin(), brand_all.end(), user_brands[userid].begin(), user_brands[userid].end(), brand_unused.begin());
			set_difference(brand_all.begin(), brand_all.end(), user_brands[userid].begin(), user_brands[userid].end(), inserter( brand_unused, brand_unused.begin()));

			sim_idx simidx;
			vector<sim_idx> vec_sim_idx;
			int len = userid_id.size();
			int id = userid_id[userid];
			for (int i = 0; i < len; ++i)
			{
				simidx.sim = sim_mat[id][i];
				simidx.idx = i;
				vec_sim_idx.push_back(simidx);
			}
			sort(vec_sim_idx.begin(), vec_sim_idx.end(), greater<sim_idx>()); // desc order

			// 前k个相似的用户
			set<int> rec_ids;
			vector<sim_idx>::iterator it = vec_sim_idx.begin();
			for (int i = 0; i < k; ++i)
			{
				rec_ids.insert( it->idx );
				++it;
			}

			// 通过与用户交集的推荐商品
			set<int> rec_brand;
			set<int>::iterator itt = brand_unused.begin();
			// userid's interest level toward brand
			vector<brand_interest> vec_bi;
			brand_interest bi;
			while ( itt != brand_unused.end() )
			{
				vector<int> newset; 
				set<int> ids = brand_ids[*itt];
				// rec_ids前k个相似用户，没有品牌的所有用户ids
				set_intersection(rec_ids.begin(), rec_ids.end(), ids.begin(), ids.end(), inserter(newset, newset.begin()));
				if (newset.empty())
				{
					++itt;
					continue;
				}
				// itt所指向商品的interest
				double interest = 0.0;
				int len = newset.size();
				for (int i = 0; i< len; ++i)
				{
					interest += sim_mat[ userid_id[userid] ][ newset[i] ];
				}

				bi.brand = *itt;
				bi.Int = interest;
				vec_bi.push_back(bi);
				++itt;
			}
			
			sort(vec_bi.begin(), vec_bi.end(), greater<brand_interest>()); // desc order

			// 推荐前m个interest的品牌
			for (int i = 0; i < m && i < (int)vec_bi.size(); ++i)
			{
				rec_brand.insert(vec_bi[i].brand);
			}

			return rec_brand;
		}

		void recommend()
		{
			map<int, int>::iterator it = userid_id.begin();
			while ( it != userid_id.end() )
			{
				user_brand_rec[it->first] = get_rec_brand_set_by_user(it->first);
				++it;
			}
		}

		void print()
		{
			// write result <user, brand> to file
			map< int, set<int> >::iterator it = user_brand_rec.begin();
			while ( it != user_brand_rec.end() )
			{
				set<int> tmp = it->second;
				set<int>::iterator it2 = tmp.begin();
				while (it2 != tmp.end())
				{
					fout << it->first << " " << *it2 << endl;
					++it2;
				}
				++it;
			}
		}

		~UserCF()
		{
			userid_id.clear();
			user_brands.clear();
			id_brands.clear();
			brand_ids.clear();
			brand_all.clear();
			user_brand_rec.clear();
			fin.close();
			fout.close();
		}
};

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "Usage : ./a.out k m" << endl;
		exit(1);
	}

	int k = atoi(argv[1]);
	int m = atoi(argv[2]);

	UserCF ucf(k,m);
	ucf.get_sim_mat();
	ucf.recommend();
#if 1
	ucf.print();
#endif
	return 0;
}
