#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

int main()
{
	ifstream fin, fin1;
	fin.open("rec_result_by_m123.txt");
	fin1.open("user_brand_m4.txt");
	if (!fin || !fin1)
	{
		cout << "can not open file " << endl;
		exit(1);
	}

	map< int, set<int> > fore_user_brands; // recomendation result
	map< int, set<int> > real_user_brands; // real result

	int user, brand;
	while (fin >> user >> brand)
	{
		fore_user_brands[user].insert(brand);
	}
	while (fin1 >> user >> brand)
	{
		real_user_brands[user].insert(brand);
	}

	double precision = 0, recall = 0;
	double fore_total_brand = 0;
	double real_total_brand = 0;
	double intersection = 0;
	double F = 0;

	map< int, set<int> >::iterator it = fore_user_brands.begin();
	map< int, set<int> >::iterator itt = real_user_brands.begin();
	while ( it != fore_user_brands.end() )
	{
		fore_total_brand += (it->second).size();
		++it;
	}
	while ( itt != real_user_brands.end() )
	{
		real_total_brand += (itt->second).size();
		++itt;
	}

	it = fore_user_brands.begin();
	while (it != fore_user_brands.end())
	{
		set<int> fore, real, newset;
		fore = it->second;
		real = real_user_brands[it->first];
		set_intersection(fore.begin(), fore.end(), real.begin(), real.end(), inserter(newset, newset.begin()));
		intersection += newset.size();
		if (newset.size() != 0)
		{
			set<int>::iterator itnew = newset.begin();
			cout << "user : " << it->first << " brand : ";
			while (itnew != newset.end())
			{
				cout << *itnew << " ";
				++itnew;
			}
			cout << endl;
		}
		++it;
	}

	precision = intersection / fore_total_brand;
	recall = intersection / real_total_brand;
	F = (2*precision*recall) / (precision+recall);
	cout << "fore_total_brand = " << fore_total_brand << endl;
	cout << "real_total_brand = " << real_total_brand << endl;
	cout << "intersection = " << intersection << endl;
	cout << "precision = " << precision << endl;
	cout << "recall = " << recall << endl;
	cout << "F = " << F << endl;

	return 0;
}
