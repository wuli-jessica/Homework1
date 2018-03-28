#pragma comment(linker, "/HEAP:1024000000,1024000000")
#pragma comment(linker, "/STACK:1024000000,1024000000")
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include <string>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <ctime>

using namespace std;
using vec=vector<string>;
using loco=unordered_map<string,vec>;
using freq=unordered_map<string,int>;

struct wrong //自定义异常类
{
	string what; //异常信息
	explicit wrong(const string s=""):what(s){} //异常构造函数
};

bool parse(string,vec&);	//解析用户输入的拼音串
void read();	//读取各个文件
void split();	//切分拼音串并接受用户选词
void save_user_wd();	//保存用户词
template <class Type,class Func> void input(Type&,const string&,Func&);	//判断用户输入的选项是否符合条件


freq* wd_fq;	//字-字频
loco* py_wd;	//拼音-字列表
loco* py_volc;	//拼音-词列表
loco* wd_py;	//字-拼音列表
loco* py_uservolc;	//拼音-用户词列表


int main()
{
	string choice;
	cout << string(10, '-') << "拼音输入法开始菜单" << string(10, '-') << "\n您想要做什么？\na.启动程序\nb.退出程序\n请输入操作符：";
	input(choice, "仅接收a或b，请重新输入:", [](string& ch) { return ch == "a" || ch == "b"; });
	if (choice == "b")
	{
		system("pause");
		return 0;
	}
	cout << string(10, '-') << "读入文本" << string(10, '-') << endl;
	try { read(); }
	catch (wrong e)
	{
		cout << e.what << endl;
		system("pause");
		return 0;
	}
	while (true)
	{
		cout << string(10, '-') << "主菜单" << string(10, '-') << endl;
		cout << "您想要做什么？\na.使用输入法\nb.退出程序\n请输入操作符：";
		input(choice, "仅接收a或b，请重新输入:", [](string& ch) { return ch == "a" || ch == "b"; });
		if (choice == "b")
			break;
		cout << string(10, '-') << "使用输入法" << string(10, '-') << endl;
		split();
	}
	save_user_wd();
	delete wd_fq;
	delete py_wd;
	delete wd_py;
	delete py_uservolc;
	system("pause");
	return 0;
}

bool parse(string str,vec& arr)
{
	auto lp = str.begin(), rp = str.begin(), ed = str.begin();
	if (ed == str.end())
		return true;
	while (*lp == '\'')
		ed = rp = ++lp;
	if (lp == ed)
		while (ed != str.end() && *ed != '\'')
			++ed;
	size_t&& dis = ed - lp;
	rp = lp + (dis >= 6 ? 6 : dis);
	while (rp - lp >= 1)
	{
		if (py_wd->find(string(lp, rp)) != py_wd->end() && parse(string(rp, str.end()), arr))
		{
			arr.emplace_back(lp, rp);
			break;
		}
		--rp;
	}
	if (lp == rp)
		return false;
	return true;
}

void read()
{
	auto read_in_file = [](const string& filename,function<void(char*&)> func)-> bool
	{
		int t0, t1;
		size_t length;
		char* buff;
		string addr = "d:\\" + filename + ".txt";
		ifstream fin(addr);
		if (!fin.is_open())
		{
			fin.close();
			if (filename != "用户词库")
				throw wrong(addr + "打开失败！\n请检查文件是否放入D盘根目录！");
			else
			{
				py_uservolc = new loco;
				return true;
			}
		}
		t0 = clock();
		fin.seekg(0, ios::end); // go to the end  
		length = fin.tellg(); // report location (this is the length)  
		fin.seekg(0, ios::beg); // go back to the beginning  
		buff = new char[length + 1];
		memset(buff, '\0', length + 1);
		fin.read(buff, length);
		fin.close(); //关闭文件
		t1 = clock();
		cout << "读入" << addr << "用时：" << t1 - t0 << "ms" << string(5, ' ') << "长度：" << length << "字节" << endl;
		func(buff);
		delete[] buff;
		return true;
	};
	int t0 = clock();
	read_in_file("一字串", [](char*& buff)
	{
		wd_fq = new freq;
		freq& freq = *wd_fq;
		stringstream sstrm(buff);
		string a;
		int b;
		while (sstrm >> a >> b)
			freq.emplace(a, b);
	});
	int t1 = clock();
	cout << "字-字频汇总用时：" << t1 - t0 << "ms" << endl;
	t0 = clock();
	read_in_file("pinyin", [](char*& buff)
	{
		py_wd = new loco;
		loco& link = *py_wd;
		string pinyin;
		string word;
		string str(buff);
		for (auto it = str.begin(); it < str.end(); ++it)
		{
			if (*it != ',')
			{
				pinyin.push_back(*it);
				continue;
			}
			++it;
			vec& vec = link[pinyin];
			while (it < str.end() && *it != '\n')
			{
				word.push_back(*it);
				if (word.size() == 2)
				{
					vec.emplace_back(word);
					word.clear();
				}
				++it;
			}
			pinyin.clear();
			if (it == str.end())
				break;
		}
	});
	t1 = clock();
	cout << "拼音-字列表汇总用时：" << t1 - t0 << "ms" << endl;
	t0 = clock();
	read_in_file("多音字", [](char*& buff)
	{
		vec pys;
		int count = 0;
		stringstream sstrm(buff);
		string a;
		while (sstrm >> a)
		{
			++count;
			if (count % 2 == 1)
			{
				pys.emplace_back(a);
				string str;
				getline(sstrm, str);
				stringstream ln(str);
				while (ln >> str)
					pys.emplace_back(str);
			}
			else
			{
				for (auto& x : pys)
				{
					vec& vec = py_wd->find(x)->second;
					if (find(vec.begin(), vec.end(), a) == vec.end())
						vec.emplace_back(a);
				}
				pys.clear();
			}
		}
	});
	for (auto& x : *py_wd)
	{
		vec& y = x.second;
		sort(y.begin(), y.end(), [](string& a,string& b) { return (*wd_fq)[a] > (*wd_fq)[b]; });
	}
	t1 = clock();
	cout << "多音字汇总用时：" << t1 - t0 << "ms" << endl;
	wd_py = new loco;
	t0 = clock();
	for (auto& x : *py_wd)
		for (auto& y : x.second)
			(*wd_py)[y].emplace_back(x.first);
	t1 = clock();
	cout << "字-拼音列表汇总用时：" << t1 - t0 << "ms" << endl;
	t0 = clock();
	read_in_file("word", [](char*& buff)
	{
		py_volc = new loco;
		stringstream sstrm(buff);
		string volc;
		//word中的不辨菽麦的“菽”在pinyin中查不到  全角半角/ 菽 鸹 珮 緒 豊 忒 镕 鎔 猁 镗
		while (sstrm >> volc)
		{
			if (volc.size() == 2)
				continue;
			string wd;
			vec pinyin;
			pinyin.reserve(100);
			for (auto& ch : volc)
			{
				wd.push_back(ch);
				if (wd.size() != 2)
					continue;
				vec oldpyvec(pinyin);
				size_t&& divi = oldpyvec.size();
				size_t&& newsize = (*wd_py)[wd].size() * divi;
				for (int i = 0; i < newsize - divi; ++i)
					pinyin.emplace_back(pinyin[0]);
				if (divi == 0)
					for (int j = 0; j < (*wd_py)[wd].size(); ++j)
						pinyin.emplace_back((*wd_py)[wd][j]);
				else
				{
					int k = 0;
					for (int i = 0; i < divi; ++i)
						for (int j = 0; j < newsize / divi; ++j)
							pinyin[k++] = oldpyvec[i] + '\'' + (*wd_py)[wd][j];
				}
				wd.clear();
			}
			for (auto& x : pinyin)
				(*py_volc)[x].emplace_back(volc);
		}
	});
	t1 = clock();
	cout << "拼音-词汇列表汇总用时：" << t1 - t0 << "ms" << endl;
	t0 = clock();
	read_in_file("用户词库", [](char*& buff)
	{
		py_uservolc = new loco;
		stringstream sstrm(buff);
		string a, b;
		while (sstrm >> a >> b)
			(*py_uservolc)[b].emplace_back(a);
	});
	t1 = clock();
	cout << "用户词库汇总用时：" << t1 - t0 << "ms" << endl;
}

void split()
{
	cout << "帮助：\n先输入拼音串，再进行选词\n1.\n拼音串须合法\n有歧义的拼音需要手动加入\'以划分\n"
		<< "2.\n选词时，按下\'-\'向前翻页，按下\'+\'向后翻页\n键入对应词的序号以选择\n"
		<< "3.（重要！）\n如果不想输入拼音了，可以输入0以中途退出\n如果想退出选词，可以键入0以中途退出\n"
		<< "4.（重要！）\n只有正常退出程序才能保存您自己的词汇\n";
	while (true)
	{
		string choice, splitres, res_str;
		vec arr, words, res;
		size_t&& count = 0;
		cout << string(10, '-') << "拼音输入" << string(10, '-') << endl;
		while (true)
		{
			string str;
			cout << "拼音串：";
			getline(cin, str);
			if (str == "0")
				goto quit;
			if (!str.empty() && parse(str, arr))
				break;
			cout << "无法解析您的拼音串，请重新输入！" << endl;
		}
		reverse(arr.begin(), arr.end());
		for (auto& x : arr)
			splitres += x + '\'';
		splitres.pop_back();
		cout << "切分结果：\n" << splitres << endl;
		cout << string(10, '-') << "词汇选择" << string(10, '-') << endl;
		while (count < arr.size()) //选词总循环
		{
			vec vec(arr.begin() + count, arr.end()), now_words;
			while (!vec.empty())
			{
				string pinyinstr;
				for (auto& x : vec)
					pinyinstr += x + '\'';
				pinyinstr.pop_back();
				auto it = py_uservolc->find(pinyinstr);
				if (it != py_uservolc->end())
					for (auto& x : it->second)
						now_words.emplace_back(x);
				loco* loco = vec.size() == 1 ? py_wd : py_volc;
				it = loco->find(pinyinstr);
				if (it != loco->end())
					for (auto& x : it->second)
						now_words.emplace_back(x);
				vec.pop_back();
			}
			int page = 0;
			int step;
			for (int k = 0; k < arr.size(); ++k)
				cout << (k == count ? "[" + arr[k] + "]" : arr[k] + "\'");
			cout << (arr.size() - 1 != count ? "\b \n" : "\n");
			for (int i = 0; i < 5 && page + i < now_words.size(); ++i)
				cout << i + 1 << "." << now_words[page + i] << " ";
			while (true)
			{
				const unsigned char ch = _getch();
				if (ch == '0')
					goto quit;
				if ('1' <= ch && ch <= '5' && page + (ch - '0') - 1 < now_words.size())
				{
					step = ch - '0' - 1;
					break;
				}
				bool&& get = false;
				if (ch == '-' && page > 0)
				{
					page -= 5;
					get = true;
				}
				else if (ch == '=' && page + 5 < now_words.size())
				{
					page += 5;
					get = true;
				}
				if (get)
				{
					cout << "\r" << string(59, ' ') << "\r";
					for (int i = 0; i < 5 && page + i < now_words.size(); ++i)
						cout << i + 1 << "." << now_words[page + i] << " ";
				}
			}
			res.emplace_back(now_words[page + step]);
			count += now_words[page + step].size() / 2;
			cout << endl << string(10, '-') << endl;
		}
		for (auto& x : res)
			res_str += x;
		cout << res_str;
		vec& volc_vec = py_volc->find(splitres)->second;
		bool&& not_in_volc = find(volc_vec.begin(), volc_vec.end(), res_str) == volc_vec.end();
		bool&& not_in_wd = py_wd->find(splitres) == py_wd->end();
		//存入用户词库
		if (not_in_volc && not_in_wd)
		{
			auto it = py_uservolc->find(splitres);
			if (it == py_uservolc->end() || find(it->second.begin(), it->second.end(), res_str) == it->second.end())
				(*py_uservolc)[splitres].emplace_back(res_str);
		}
	quit:
		cout << "\n是否继续输入拼音？(y/n)：";
		input(choice, "仅接收y或n，请重新输入:", [](string& ch) { return ch == "y" || ch == "n"; });
		if (choice == "n")
			break;
	}
}

void save_user_wd()
{
	ofstream fout("d://用户词库.txt");
	for (auto& x : *py_uservolc)
		for (auto& y : x.second)
			fout << setiosflags(ios::left) << setw(20) << y << string(10, ' ') << x.first << endl;
	fout.close();
}

template <class Type,class Func> void input(Type& para,const string& str,Func& f)
{
	while (true)
	{
		getline(cin, para);
		if (f(para))
			break;
		cout << str;
	}
}
