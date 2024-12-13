//����ҵ1����ʹ��ģ���˻��㷨���TSP����
//�ؼ��㣺ģ���˻��㷨�����ؿ����㷨��Fisher-Yatesϴ���㷨������
//�����ߣ�2024303798 �����



/* �������� ͷ�ļ������Լ���ʼ׼�� �������� */

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

//������нṹ��
struct CITY {
	int id;
	char name[20];
	double x;
	double y;
};

//���ض���
bool loading = false;
void loading_animation() {
	const char spinner[] = { '|', '/', '��', '\\' }; // ����֡
	int index = 0; // ����֡����
	while (loading) { // ������δ���ǰ�����������
		cout << "\r�滮�� " << spinner[index] << flush; // ʹ�ûس���������
		index = (index + 1) % 4; // ѭ������֡
		this_thread::sleep_for(chrono::milliseconds(100)); // ���ƶ����ٶ�
	}
	cout << "\r�滮���!" << endl; // ������״̬
}



/* �������� �����ʹ�� �������� */

// ��������ڵ�ṹ
struct Node {
	double distance;        // ·������
	int* city_order;        // ·������˳��
	Node* next;             // ָ����һ���ڵ�
	Node(double dist, int* order, int size) : distance(dist), next(nullptr) {
		city_order = new int[size];
		for (int i = 0; i < size; i++) {
			city_order[i] = order[i];
		}
	}
	~Node() {
		delete[] city_order; // �ͷ�·���ڴ�
	}
};

// ����ڵ㵽����β��
void insert_node(Node*& head, double distance, int* city_order, int size) {
	Node* new_node = new Node(distance, city_order, size);
	if (!head) {//����Ϊ��
		head = new_node;//�½ڵ�Ϊͷ�ڵ�
	}
	else {
		Node* temp = head;
		while (temp->next) {
			temp = temp->next;
		}
		temp->next = new_node;
	}
}



/* �������� ����ļ��� �������� */

//����Haversine��ʽ�������������֮��ľ���
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

//����һ����·���ĳ���
double path_length(int* city_order, int num, CITY* city_passed) {//num����β������,city_order������±�Ϊnum-1
	double distance = 0;
	for (int i = 0; i < num-1 ; i++) {
		distance += hav_distance(city_passed[city_order[i]], city_passed[city_order[i + 1]]);
	}
	return distance;
}



/* �������� ���ؿ����㷨 �������� */

int* monte_carlo(int samples, int n, CITY* city_passed) {//n������β������
	double min_distance = 1e9;
	int* path = new int[n+1];
	//ѭ��100�Σ�ÿ������һ������⣬��¼���·������
	for (int i = 0; i < samples; i++) {
		int* city_order = new int[n+1];
		for (int j = 0; j < n; j++) {
			city_order[j] = j;
		}
		//ʹ��Fisher-Yatesϴ���㷨���ҳ���˳��
		for (int j = n-1 ; j > 0; j--) {
			int k = rand() % (j + 1);
			int temp = city_order[j];
			city_order[j] = city_order[k];
			city_order[k] = temp;
		}
		city_order[n] = city_order[0];
		//����·����
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



/* �������� ģ���˻��㷨��غ��� �������� */

//�Ŷ�·�������½⡪����������Ӱ���С��
void dis_change(int* city_order, int n) {
	int i = rand() % n;
	int j = rand() % n;
	int temp = city_order[i];
	city_order[i] = city_order[j];
	city_order[j] = temp;
	city_order[n] = city_order[0];
}
//�Ŷ�·�������½⡪����ת����Ӱ��ϴ�
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
//���ѡ���Ŷ�����
void disturb(int* city_order, int n) {
	int r = rand() % 10;
	if (r < 8) {
		dis_change(city_order, n);
	}
	else {
		dis_reverse(city_order, n);
	}
}

//Metropolis׼��
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



/* �������� ��������ģ���˻��㷨 �������� */

int main() {
	srand(static_cast<unsigned int>(time(0)));
	int n;
	cout << "��ӭʹ��\"CB������\"������ģ������·�߹滮�ܼң�" << endl;
	this_thread::sleep_for(chrono::seconds(2));
	cout << "������;������������";
	cin >> n;

	CITY* city_passed = new CITY[n + 1];
	cout << "������������ƺ;�γ�ȣ�" << endl;
	for (int i = 0; i < n; i++) {
		city_passed[i].id = i;
		cin >> city_passed[i].name >> city_passed[i].x >> city_passed[i].y;
	}
	city_passed[n] = city_passed[0];
	cout << "������Ϣ¼����ɣ�" << endl;
	cout << "��������ĳ������У�";
	char start_city[20];
	cin >> start_city;

	cout << "����������������������������������������������������������������" << endl;
	thread loading_thread(loading_animation);//�������ض����߳�
	loading = true;

	//ʹ�����ؿ����㷨���ɳ�ʼ��
	const int samples = 100; //���ؿ����㷨��������
	int* initial_path = monte_carlo(samples, n, city_passed);
	double distance = path_length(initial_path, n + 1, city_passed);//��ʼ��·������

	double T = 1000; //��ʼ�¶�
	double T_min = 1e-8; //��ֹ�¶�
	double alpha = 0.999; //��������
	int L = 100; //ÿ���¶��µĵ�������
	int* city_order = new int[n + 1];
	for (int i = 0; i <= n; i++) {
		city_order[i] = initial_path[i];
	}
	delete[] initial_path;
	initial_path = nullptr;

	//ģ���˻����
	while (T > T_min) {

		//��������洢��ǰ�¶��������½�
		Node* head = nullptr;
		//����L��
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
				k++;//����kΪ��ǰ�¶��µ��½�����
			}
			delete[] city_order_temp;
		}

		//����������¼��ǰ����·������·������
		if (k > 0) {
			double min_distance = 1e9;
			Node* temp = head;//��ͷ�ڵ㿪ʼ
			while (temp) {//ָ�벻Ϊ��
				if (temp->distance < min_distance) {
					min_distance = temp->distance;
					for (int i = 0; i <= n; i++) {
						city_order[i] = temp->city_order[i];
					}//��������·��
				}
				temp = temp->next;
			}
			distance = min_distance;//�������·������
		}
		city_order[n] = city_order[0];
		//���ˣ���ǰ�¶��µ�����·���Ѿ��ҵ�
		
		//�ͷ������ڴ�
		Node* temp = head;
		while (temp) {
			Node* next = temp->next;
			delete temp;
			temp = next;
		}
		head = nullptr;
		
		//����
		T *= alpha;

	}

	//������·���Լ����·����
	loading = false;
	loading_thread.join();
	cout << "����������������������������������������������������������������" << endl;
	cout << "�������·�ߣ�" << endl;
	//�ҵ��������е�λ��
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
	cout << "���·�����ȣ�" << fixed << setprecision(2) << distance << "km" << endl;

	delete[] city_passed;
	system("pause");
	return 0;
}