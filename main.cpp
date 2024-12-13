//大作业1——使用模拟退火算法解决TSP问题
//关键点：模拟退火算法、蒙特卡洛算法、Fisher-Yates洗牌算法、链表
//制作者：2024303798 颜语恩



/* ———— 头文件包含以及初始准备 ———— */

#include <iostream>
#include <cmath>
#include <random>
#include <ctime>
#include <iomanip>
#include <thread>
#include <chrono>
using namespace std;
#define M_PI 3.14159265358979323846
#define EARTH_R 6371.4

//定义城市结构体
struct CITY {
	int id;
	char name[20];
	double x;
	double y;
};

//加载动画
bool loading = false;
void loading_animation() {
	const char spinner[] = { '|', '/', '—', '\\' }; // 动画帧
	int index = 0; // 动画帧索引
	while (loading) { // 在任务未完成前持续输出动画
		cout << "\r规划中 " << spinner[index] << flush; // 使用回车符覆盖行
		index = (index + 1) % 4; // 循环动画帧
		this_thread::sleep_for(chrono::milliseconds(100)); // 控制动画速度
	}
	cout << "\r规划完成!" << endl; // 输出完成状态
}



/* ———— 链表的使用 ———— */

// 定义链表节点结构
struct Node {
	double distance;        // 路径长度
	int* city_order;        // 路径城市顺序
	Node* next;             // 指向下一个节点
	Node(double dist, int* order, int size) : distance(dist), next(nullptr) {
		city_order = new int[size];
		for (int i = 0; i < size; i++) {
			city_order[i] = order[i];
		}
	}
	~Node() {
		delete[] city_order; // 释放路径内存
	}
};

// 插入节点到链表尾部
void insert_node(Node*& head, double distance, int* city_order, int size) {
	Node* new_node = new Node(distance, city_order, size);
	if (!head) {//链表为空
		head = new_node;//新节点为头节点
	}
	else {
		Node* temp = head;
		while (temp->next) {
			temp = temp->next;
		}
		temp->next = new_node;
	}
}



/* ———— 距离的计算 ———— */

//依据Haversine公式计算地球上两点之间的距离
double hav(double x)
{
	double result;
	result = (1 - cos(x)) / 2;
	return result;
}
double toRadians(double degree)
{
	return degree * (M_PI / 180.0);
}
double hav_distance(CITY city1, CITY city2)
{
	double lat1 = toRadians(city1.x);
	double lat2 = toRadians(city2.x);
	double lon1 = toRadians(city1.y);
	double lon2 = toRadians(city2.y);
	double dlat = lat2 - lat1;
	double dlon = lon2 - lon1;
	double a = hav(dlat) + cos(lat1) * cos(lat2) * hav(dlon);
	double c = 2 * asin(sqrt(a));
	return EARTH_R * c;
}

//计算一整条路径的长度
double path_length(int* city_order, int num, CITY* city_passed) {//num包括尾部城市,city_order的最大下标为num-1
	double distance = 0;
	for (int i = 0; i < num-1 ; i++) {
		distance += hav_distance(city_passed[city_order[i]], city_passed[city_order[i + 1]]);
	}
	return distance;
}



/* ———— 蒙特卡洛算法 ———— */

int* monte_carlo(int samples, int n, CITY* city_passed) {//n不包括尾部城市
	double min_distance = 1e9;
	int* path = new int[n+1];
	//循环100次，每次生成一个随机解，记录最短路径长度
	for (int i = 0; i < samples; i++) {
		int* city_order = new int[n+1];
		for (int j = 0; j < n; j++) {
			city_order[j] = j;
		}
		//使用Fisher-Yates洗牌算法打乱城市顺序
		for (int j = n-1 ; j > 0; j--) {
			int k = rand() % (j + 1);
			int temp = city_order[j];
			city_order[j] = city_order[k];
			city_order[k] = temp;
		}
		city_order[n] = city_order[0];
		//计算路径长
		double distance = path_length(city_order, n+1, city_passed);
		if (distance < min_distance) {
			min_distance = distance;
			for (int j = 0; j <= n; j++) {
				path[j] = city_order[j];
			}
		}
		delete[] city_order;
	}
	return path;
}



/* ———— 模拟退火算法相关函数 ———— */

//扰动路径产生新解——交换法（影响较小）
void dis_change(int* city_order, int n) {
	int i = rand() % n;
	int j = rand() % n;
	int temp = city_order[i];
	city_order[i] = city_order[j];
	city_order[j] = temp;
	city_order[n] = city_order[0];
}
//扰动路径产生新解——逆转法（影响较大）
void dis_reverse(int* city_order, int n) {
	int i = rand() % n; 
	int j = rand() % n;
	if (i > j) {
		int temp = i;
		i = j;
		j = temp;
	}
	while (i < j) {
		int temp = city_order[i];
		city_order[i] = city_order[j];
		city_order[j] = temp;
		i++;
		j--;
	}
	city_order[n] = city_order[0];
}
//随机选择扰动方法
void disturb(int* city_order, int n) {
	int r = rand() % 10;
	if (r < 8) {
		dis_change(city_order, n);
	}
	else {
		dis_reverse(city_order, n);
	}
}

//Metropolis准则
bool metropolis(double dE, double T) {
	if (dE < 0) {
		return true;
	}
	else {
		double p = exp(-dE / T);
		double r = (double)rand() / RAND_MAX;
		if (r < p) {
			return true;
		}
		else {
			return false;
		}
	}
}



/* ———— 主函数：模拟退火算法 ———— */

int main() {
	srand(static_cast<unsigned int>(time(0)));
	int n;
	cout << "欢迎使用\"CB旅行社\"：您的模拟旅游路线规划管家！" << endl;
	this_thread::sleep_for(chrono::seconds(2));
	cout << "请输入途径城市数量：";
	cin >> n;

	CITY* city_passed = new CITY[n + 1];
	cout << "请输入城市名称和经纬度：" << endl;
	for (int i = 0; i < n; i++) {
		city_passed[i].id = i;
		cin >> city_passed[i].name >> city_passed[i].x >> city_passed[i].y;
	}
	city_passed[n] = city_passed[0];
	cout << "城市信息录入完成！" << endl;
	cout << "请输入你的出发城市：";
	char start_city[20];
	cin >> start_city;

	cout << "————————————————————————————————" << endl;
	thread loading_thread(loading_animation);//开启加载动画线程
	loading = true;

	//使用蒙特卡洛算法生成初始解
	const int samples = 100; //蒙特卡洛算法的样本数
	int* initial_path = monte_carlo(samples, n, city_passed);
	double distance = path_length(initial_path, n + 1, city_passed);//初始解路径长度

	double T = 1000; //初始温度
	double T_min = 1e-8; //终止温度
	double alpha = 0.999; //降温速率
	int L = 100; //每个温度下的迭代次数
	int* city_order = new int[n + 1];
	for (int i = 0; i <= n; i++) {
		city_order[i] = initial_path[i];
	}
	delete[] initial_path;
	initial_path = nullptr;

	//模拟退火过程
	while (T > T_min) {

		//创建链表存储当前温度下所有新解
		Node* head = nullptr;
		//迭代L次
		int k = 0;
		for (int i = 0; i < L; i++) {
			int* city_order_temp = new int[n + 1];
			for (int j = 0; j <= n; j++) {
				city_order_temp[j] = city_order[j];
			}
			disturb(city_order_temp, n);
			double new_distance = path_length(city_order_temp, n + 1, city_passed);
			double dE = new_distance - distance;
			if (metropolis(dE, T)) {
				insert_node(head, new_distance, city_order_temp, n + 1);
				k++;//最终k为当前温度下的新解数量
			}
			delete[] city_order_temp;
		}

		//遍历链表，记录当前最优路径及其路径长度
		if (k > 0) {
			double min_distance = 1e9;
			Node* temp = head;//从头节点开始
			while (temp) {//指针不为空
				if (temp->distance < min_distance) {
					min_distance = temp->distance;
					for (int i = 0; i <= n; i++) {
						city_order[i] = temp->city_order[i];
					}//更新最优路径
				}
				temp = temp->next;
			}
			distance = min_distance;//更新最短路径长度
		}
		city_order[n] = city_order[0];
		//至此，当前温度下的最优路径已经找到
		
		//释放链表内存
		Node* temp = head;
		while (temp) {
			Node* next = temp->next;
			delete temp;
			temp = next;
		}
		head = nullptr;
		
		//降温
		T *= alpha;

	}

	//输出最佳路径以及最短路径长
	loading = false;
	loading_thread.join();
	cout << "————————————————————————————————" << endl;
	cout << "最短旅游路线：" << endl;
	//找到出发城市的位置
	int start = 0;
	for (int i = 0; i <= n; i++) {
		if (strcmp(city_passed[city_order[i]].name, start_city) == 0) {
			start = i;
			break;
		}
	}
	for (int i = start; i <= n; i++) {
		cout << city_passed[city_order[i]].name << " -> ";
	}
	for (int i = 0; i < start; i++) {
		cout << city_passed[city_order[i]].name << " -> ";
	}
	cout << city_passed[city_order[start]].name << endl;
	cout << "最短路径长度：" << fixed << setprecision(2) << distance << "km" << endl;

	delete[] city_passed;
	system("pause");
	return 0;
}