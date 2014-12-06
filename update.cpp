#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <set>
#include <algorithm>
#include <string>
#include <iostream>
#define USER_NUM 2000
#define MOVIE_NUM 2000
#define USER_CANDINATE 30

using namespace std;

int user_num = -1;
int movie_num = -1;
int recommend_num = -1;
int RecommendSum = 0;
double Accuracy;
double Recall;

struct Movie{
	int movieID;
	int rating;
	bool operator<(const Movie &a)const
	{
		return movieID < a.movieID;
	}
};
struct Candinate{
	int userID;
	double similarity;
	bool operator<(const Candinate &a)const
	{
		return similarity < a.similarity;
	}
};
struct ReMoive{
	int movieID;
	double weight;
	bool operator<(const ReMoive &a)const
	{
		return weight < a.weight;
	}
};

set<struct Movie>userHasMovie[USER_NUM];
set<int> UserHasMovie[USER_NUM];
set<struct Movie>TestMoive[USER_NUM];
set<struct Candinate>CandinateUser[USER_NUM];
set<struct Movie>CandindateMoive[USER_NUM];
set<struct ReMoive>RecommendMoive[USER_NUM];

void ReadFromFin(FILE *fin)
{
	int user, movie, rating ,time;
	while(fscanf(fin,"%d%d%d%d", &user, &movie, &rating, &time) != EOF)
	{
		struct Movie tmp;
		tmp.movieID = movie;
		tmp.rating = rating*100;
		userHasMovie[user].insert(tmp);
		UserHasMovie[user].insert(movie);
		user_num = max(user_num, user);
		movie_num = max(movie_num, movie);
	}
	return ;
}
//计算jaccard 相似度
double GetJaccard(set<Movie> &a, set<Movie> &b)
{
	set<Movie>Intersection;
	set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(Intersection,Intersection.begin()));

	set<Movie>Union;
	set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(Union, Union.begin()));

	int IntersectionNum = Intersection.size();
	int UnionNum = Union.size();
	return (double)IntersectionNum/UnionNum;
}
//计算pearson相似度
double GetPearson(set<Movie> &a, set<Movie> &b)
{
	double similarity = 0;
	set<Movie>Intersection;
	set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(Intersection, Intersection.begin()));
	if(Intersection.size() < 3)
		return 0;
	set<Movie>::iterator movieIT = Intersection.begin();
	int ratingA[MOVIE_NUM],ratingB[MOVIE_NUM];
	int num = 0;
	int sumA = 0, sumB = 0;
	while( movieIT != Intersection.end())
	{
		set<Movie>::iterator tmp = a.find(*movieIT);
		ratingA[num] = (*tmp).rating;
		sumA += ratingA[num];

		tmp = b.find(*movieIT);
		ratingB[num] = (*tmp).rating;
		sumB += ratingB[num];
		movieIT++;
		num++;
	}

	double meansA = (double)sumA/num;
	double meansB = (double)sumB/num;
	double frac = 0;
	double frasA = 0;
	double frasB = 0;
	for (int i = 0; i < num; i++)
	{
		frac  += (ratingA[i] - meansA)*(ratingB[i] - meansB);
		frasA += (ratingA[i] - meansA)*(ratingA[i] - meansA);
		frasB += (ratingB[i] - meansB)*(ratingB[i] - meansB);
	}
	double A = frac *frac;
	double B = frasA * frasB;
	if(fabs(B)< 1e-6)
		return -1;
	//printf("====%.3lf\t%.3lf\n\n", frasA, frasB);
	similarity = A/B;
	return similarity;
}

double GetSIM(set<Movie> &a, set<Movie> &b, int type)
{
	if(type == 1)
	{
		return  GetJaccard(a,b);
	}
	else if(type == 2)
	{
		return GetPearson(a,b);
	}
	return 0;
}

//获取候选用户集合
void GetCandindateUser(void)
{
	for (int i = 1; i <= user_num; i++)
	{
		for (int j =1; j <= user_num; j++)
		{
			if(i == j)
				continue;
			struct Candinate tmp_candinate;
			tmp_candinate.similarity= GetSIM(userHasMovie[i],userHasMovie[j],2);
			tmp_candinate.userID = j;
			//if( tmp_candinate.similarity <= 0.4)
			//	continue;
			int candinatenum =CandinateUser[i].size();
			if( candinatenum < USER_CANDINATE )
			{
				CandinateUser[i].insert(tmp_candinate);
			}
			else
			{
				set<struct Candinate>::iterator min = CandinateUser[i].begin();
				if((*min).similarity < tmp_candinate.similarity)
				{
					CandinateUser[i].erase(min);
					CandinateUser[i].insert(tmp_candinate);
				}
			}
		}
	}
}
//获取候选电影集合
void GetCandindateMoive(void)
{
	for (int i = 1; i <= user_num; ++i)
	{
		set<Candinate>::iterator it = CandinateUser[i].begin();

		while (it != CandinateUser[i].end())
		{
			int userID = (*it).userID;
			//printf("%d\t", userID);
			set<Movie>::iterator movieIT = userHasMovie[userID].begin();
			while (movieIT != userHasMovie[userID].end())
			{
				CandindateMoive[i].insert(*movieIT);
				movieIT++;
			}
			it++;
		}
	}
}
//获取推荐电影集合
void GetRecommendMoive(void)
{
	for (int i = 1; i <= user_num; i++)
	{
		//printf("====================%d=====================\n",i);
		set<Movie>::iterator movieIT = CandindateMoive[i].begin();
		while( movieIT != CandindateMoive[i].end())
		{
			double weight = 0;
			int movieID = (*movieIT).movieID;
			struct Movie movie;
			movie.movieID = movieID;
			if (UserHasMovie[i].count(movieID) == (unsigned long)0)
			{
				set<Candinate>::iterator candinateIT = CandinateUser[i].begin();
				while( candinateIT != CandinateUser[i].end())
				{
					int candinateID = (*candinateIT).userID;
					if(UserHasMovie[candinateID].count(movieID))
					{
						set<Movie>::iterator tmp = userHasMovie[candinateID].find(movie);
						weight += (*candinateIT).similarity;// * (*tmp).rating;
					}
					candinateIT++;
				}
				ReMoive recommendmoive;
				recommendmoive.movieID = movieID;
				recommendmoive.weight = weight;
				int RecommendMoiveNum = RecommendMoive[i].size();
				if( RecommendMoiveNum < recommend_num)
				{
					RecommendMoive[i].insert(recommendmoive);
				}
				else
				{
					set<ReMoive>::iterator remoiveIT = RecommendMoive[i].begin();
					if((*remoiveIT).weight < recommendmoive.weight)
					{
						RecommendMoive[i].erase(remoiveIT);
						RecommendMoive[i].insert(recommendmoive);
					}
				}
			}
			movieIT++;
		}
		set<ReMoive>::iterator tt =RecommendMoive[i].begin();
		RecommendSum += RecommendMoive[i].size();
		//printf("%d:\t%d\n",i, RecommendSum);
	}
	return;
}
//计算正确率与召回率
void GetAccracyAndRecall(FILE *fin)
{
	int user, movie, rating, time;
	int testsum = 0;
	int testnum = 0;
	while(fscanf(fin, "%d%d%d%d", &user, &movie, &rating, &time) != EOF)
	{
		testsum++;
		int flag = 0;
		set<ReMoive>::iterator testIT = RecommendMoive[user].begin();
		while( testIT != RecommendMoive[user].end())
		{
			if((*testIT).movieID == movie)
			{
				flag = rating;
				break;
			}
			testIT++;
		}
		testnum += flag/RecommendMoive[user].size();
	}
	printf("%d\t%d\t%d\n", testnum, RecommendSum,testsum);
	Accuracy = 100.0*(double)testnum/RecommendSum;
	Recall = 100.0*(double)testnum/testsum;
}

//主函数 : BaseUser in out recommend_num
int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		printf("error!");
		return 0;
	}
	string fin_path(argv[1]);
	FILE *fin = fopen(fin_path.c_str(),"r+");
	//string fout_path(argv[2]);
	//FILE *fout = fopen(fout_path.c_str(),"w+");
	string ftest_path(argv[2]);
	FILE *ftest = fopen(ftest_path.c_str(),"r+");

	recommend_num = atoi(argv[3]);
	//读取数据  存入userHasmoive
	//格式：moiveID rating 即电影的编号与该用户评分
	ReadFromFin(fin);
	/*ReadFromFin 测试
	set<struct Movie>::iterator user = userHasMovie[1].begin();
	 while( user != userHasMovie[1].end())
	 {
		Movie movietmp = (*user);
		int tmp = movietmp.movieID;
		printf("%d\n", tmp);
		user++;
	 }
	 */
	//获取候选用户
	//计算相应相似度，根据相似度选出前USER_CANDINATE 用户
	GetCandindateUser();

	GetCandindateMoive();
	//测试
	//输出候选电影集合
	//set<Movie>::iterator MovieIT = CandindateMoive[1].begin();
	//while ( MovieIT != CandindateMoive[1].end())
	//{
	//	int movieID = (*MovieIT).movieID;
	//	printf("%d\n", movieID);
	//	MovieIT++;
	//}
	//
	GetRecommendMoive();
	//测试：输出推荐电影
	//set<ReMoive>::iterator remoiveIT = RecommendMoive[1].begin();
	//while ( remoiveIT != RecommendMoive[1].end())
	//{
	//	int movieID = (*remoiveIT).movieID;
	//	printf("%d\n", movieID);
	//	remoiveIT++;
	//}
	GetAccracyAndRecall(ftest);
	//测试正确率与召回率计算
	printf("正确率：%.3lf\t 召回率：%.3lf\n", Accuracy, Recall);
	return 0;
}
