#pragma once
#include"mirror.h"
#include<fstream>
class A
{
	template<typename T>
	using sptr = std::shared_ptr<T>;
public:
	virtual ~A() {}
	std::vector<int>a = { 1,2,3,4 };
	std::vector<std::vector<int>>b = { {1,2,3,4,5},{6,7,8,9,10} };
	std::vector<sptr<double>>c = { std::make_shared<double>(1.1),std::make_shared<double>(2.2) ,std::make_shared<double>(3.3) };
	int d = 1000;
	int* e = nullptr;
	std::string f = "none";
	std::unique_ptr<int>g = std::make_unique<int>(999);
	int fun1(std::string a) 
	{
		std::cout << "fun1" << std::endl;
		std::cout <<"\t"<< a << std::endl;
		return 1; 
	};
	
	float fun2(int* a) 
	{
		std::cout << "fun2" << std::endl;
		std::cout << "\t" << *a << std::endl;
		return 2.2f; 
	};
	double fun3(float* a,int size) 
	{
		std::cout << "fun3" << std::endl;
		for (int i = 0; i < size; ++i)
		{
			std::cout << "\t" << a[i] << std::endl;
		}
		return 3.3333333;
	};
private:
	void fun4(const char* str)
	{
		std::cout << "fun4" << std::endl;
		int i = 0;
		std::cout << "\t";
		while (str[i]!='\0')
		{
			std::cout<< str[i];
			++i;
		}
		std::cout << "\n";
	}
	void fun5(std::vector<int>vec)
	{
		std::cout << "fun5" << std::endl;
		std::ranges::for_each(vec, [](int in) {std::cout << "\t" << in << std::endl; });
	}
	void fun6(int val)
	{
		std::cout << "fun6" << std::endl;
		std::cout << "\t" << val << std::endl;
	}
};
class B :virtual public A
{
public:
	int bbb;
};
class C :virtual public A
{
public:
	int ccc;
};
class D :public B, public C
{
public:
	int ddd;
};

void fun7()
{
	std::cout << "fun7" << std::endl;
}
REGISTER_TYPE(A);
REGISTER_TYPE(B);
REGISTER_TYPE(C);
REGISTER_TYPE(D);

REGISTER_CHILD(A, B);
REGISTER_CHILD(A, D);
REGISTER_CHILD(B, D);

REGISTER_MEMBER(A, a);
REGISTER_MEMBER(A, b);
REGISTER_MEMBER(A, c);
REGISTER_MEMBER(A, d);
REGISTER_MEMBER(A, e);
REGISTER_MEMBER(A, f);
REGISTER_MEMBER(A, g);

REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun1);
REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun2);
REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun3);
REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun4);
REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun5);
REGISTER_PRIVATE_MEMBER_FUNCTION(A, fun6);
//REGISTER_FUNCTION(fun7);


using namespace mirror;
void test()
{
	auto& gdata = GetGlobalData();
	auto typeId = GetTypeId<A>();
	auto& typeInfo=typeId.GetInfo();
	A* a = new A();

	auto fun1 = GetMethod<A>("fun1");
	auto fun2 = GetMethod<A>("fun2");
	auto fun3 = GetMethod<A>("fun3");
	auto fun4 = GetMethod<A>("fun4");
	auto fun5 = GetMethod<A>("fun5");
	auto fun6 = GetMethod<A>("fun6");
	const MemberInfo* meba = GetVariable<A>("a");
	const MemberInfo* mebb = GetVariable<A>("b");
	const MemberInfo* mebc = GetVariable<A>("c");
	const MemberInfo* mebd = GetVariable<A>("d");
	const MemberInfo* mebe = GetVariable<A>("e");
	const MemberInfo* mebf = GetVariable<A>("f");
	const MemberInfo* mebg = GetVariable<A>("g");
	std::vector<int>&meb_a = meba->GetRef<std::vector<int>>(a);
	meb_a.push_back(10);
	int* p = new int(10000);
	mebd->Set(a,100.f);
	mebe->Set(a, p);
	mebf->Set(a, "mirror");
	mebg->Set(a, std::make_unique<int>(400));

	fun1.MemberCall(a, { "mirror:fun1"});
	fun2.MemberCall(a, { p});
	std::vector<float>vecfloat = { 1.1f,2.2f,3.3f };
	fun3.MemberCall(a, { vecfloat.data(),vecfloat.size() });
	fun4.MemberCall(a, { "mirror:fun4" });
	std::vector<int>vecint = { 1,2,3,4,5,6,7,8,9};
	fun5.MemberCall(a, { vecint });
	fun6.MemberCall(a, { 100.1f});

	rapidjson::Document doc;
	doc.SetObject();
	TypeInfo::RapidJsonAllocator allocator;
	typeInfo.JSonSerializer(doc, a, allocator);
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);                
	std::cout << "序列化后的JSON内容：" << std::endl;
	std::cout << buffer.GetString() << std::endl;

	std::ofstream out("./serialization.txt");
	out << buffer.GetString();
	out.close();

	std::ifstream in("./serialization.txt");
	rapidjson::IStreamWrapper istream(in);
	doc.SetObject();
	doc.ParseStream(istream);
	A* dsertest = new A();
	typeInfo.JSonDeserializer(doc, dsertest);



	auto& globaldata = GetGlobalData();
	delete a;
	delete p;
	delete dsertest;

}
