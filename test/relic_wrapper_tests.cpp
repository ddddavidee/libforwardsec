#include <cereal/archives/binary.hpp>
#include <memory>

#include "gtest/gtest.h"
#include "relic_api.h"
using namespace std;
using namespace relicxx;
void rand(ZR &a, PairingGroup &g){
	a= g.randomZR();
}
void rand(G1 &a, PairingGroup &g){
	a= g.randomG1();
}
void rand(G2 &a, PairingGroup &g){
	a= g.randomG2();
}
void rand(GT &a, PairingGroup &g){
	a= g.randomGT();
}
extern 	void pc_param_print();
extern int pc_param_level();
class RelicEnvironment: public ::testing::Environment{
	std::unique_ptr<relicResourceHandle> handle;
 public:
  virtual ~RelicEnvironment() {}
  // Override this to define how to set up the environment.
  virtual void SetUp() {
	  cout << "Attempting to initialize relic..." <<endl << endl <<
			  "\tIf this takes a very long time  you may have ran out of entropy." << endl <<
			  "\tThis sometimes happens on VMs. If you have this issue, either fix " << endl <<
			  "\tyour vm entropy (if possible) or recompile relic with -DRAND=\"UDEV\"" << endl <<
			  "\tor some other option that that is not DEV (i.e. /dev/random)" << endl <<
			  "\t(WARNING many of these may not be cryptographically safe) ."  << endl << endl;
	  handle = std::unique_ptr<relicResourceHandle>(new relicResourceHandle);
	  if(!handle){
		  cerr << "Error. Cannot initialze relic. new relicResourceHandle failed." << endl;
		  throw runtime_error("Error. Cannot initialze relic. new relicResourceHandle failed.");
	  }
	  cout << "Relic intialized"<<endl;
	  cout << "relic curve security level:\t" << pc_param_level();
	  pc_param_print();
  }
	  // Override this to define how to tear down the environment.
  virtual void TearDown() {}
};
::testing::Environment* const foo_env = ::testing::AddGlobalTestEnvironment(new RelicEnvironment);

template <typename T>
class AlgTest : public :: testing::Test{
public:
int a;
//PairingGroup g;
};


typedef ::testing::Types<ZR, G1, G2,GT> numerics;
TYPED_TEST_CASE(AlgTest	, numerics);

class pg : public ::testing::Test {
protected: 
	 virtual void SetUp(){} 
	 PairingGroup group;
	// static PairingGroup _group;
};


#define randabcd(T) 		\
T a = group.random##T(); \
T b = group.random##T(); \
T c = group.random##T(); \
T d = group.random##T(); \
T e = group.random##T(); \
T i;


TYPED_TEST(AlgTest,serialization){
	std::stringstream ss;
	PairingGroup g;

	TypeParam a,b;
	rand(a,g);
	EXPECT_NE(a,b);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(a);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(b);
	}
	EXPECT_EQ(a,b);
}
TYPED_TEST(AlgTest,eq){
	PairingGroup g;
	TypeParam a,b,c,d;
	rand(a,g);rand(b,g);rand(c,g);
	d=a;

	EXPECT_NE(a,b);
	EXPECT_NE(b,a);

	EXPECT_EQ(a,a);
	EXPECT_EQ(b,b);
	EXPECT_EQ(a,d);
	EXPECT_EQ(d,a);
}


TYPED_TEST(AlgTest,identity){
	PairingGroup g;
	TypeParam a,b,c,d;
	rand(d,g);
	EXPECT_EQ(a,b);
	EXPECT_EQ(g.mul(a,b),c);
	EXPECT_NE(a,d);
	EXPECT_EQ(g.mul(a,d),d);
}
TYPED_TEST(AlgTest,random){
	PairingGroup g;
	TypeParam a,b;
	rand(a,g);rand(b,g);
	EXPECT_NE(a,b);
}

TYPED_TEST(AlgTest,MultiplicationIsCommunitive){
	PairingGroup g;
	TypeParam a,b;
	rand(a,g);rand(b,g);
	EXPECT_EQ(g.mul(a,b),g.mul(b,a));
}

TYPED_TEST(AlgTest,MultiplicationIsAssociative){
	PairingGroup g;
	TypeParam a,b,c,d;
	rand(a,g);rand(b,g);rand(c,g);rand(d,g);
	EXPECT_EQ(g.mul(g.mul(a,b),c),g.mul(g.mul(a,c),b));
}
TYPED_TEST(AlgTest,Inverse){
	PairingGroup g;
	TypeParam a,b,i;
	rand(a,g);rand(b,g);
	EXPECT_EQ(g.mul(g.mul(a,b),g.inv(b)),a);
	EXPECT_EQ( g.mul(a,g.inv(a)),i );

}
TYPED_TEST(AlgTest,copy){
	PairingGroup g;
	TypeParam a;
	rand(a,g);
	TypeParam b = a;
	TypeParam c(a); 
	EXPECT_EQ(b,a);
	EXPECT_EQ(c,a);
}
#ifdef RELICXX_MOVE
TYPED_TEST(AlgTest,stdmove){
	PairingGroup g;
	for(unsigned int i=0;i<100;i++){
		TypeParam a;
		rand(a,g);
		TypeParam aa = a;
		ASSERT_EQ(aa,a);
		TypeParam c = std::move(a);
		ASSERT_EQ(c,aa);
	}
}
#endif


TEST_F(pg,G1Identity){
	G1 a,b,c;
	G1 d = group.randomG1();
	EXPECT_EQ(a,b);
	EXPECT_EQ(a+b,c);
	EXPECT_EQ(d,d);
	EXPECT_NE(a,d);
	EXPECT_EQ(a+d,d);
}
TEST_F(pg,G2Identity){
	G2 a,b,c;
	G2 d = group.randomG2();
	EXPECT_EQ(a,b);
	EXPECT_EQ(a+b,c);
	EXPECT_EQ(d,d);
	EXPECT_NE(a,d);
	EXPECT_EQ(a+d,d);
}


TEST_F(pg,GTIdentity){
	GT a,b,c;
	GT d = group.randomGT();
	EXPECT_EQ(a,b);
	EXPECT_EQ(a*b,c);
	EXPECT_NE(a,d);
	EXPECT_EQ(a*d,d);
	EXPECT_EQ(d,d);
}

TEST_F(pg,G1Add){
	randabcd(G1)
	EXPECT_EQ(a+b,b+a) << "not commutative";
	EXPECT_EQ(a+i,a) << "identity is wrong";
	d= a+b;
	d= d+c;
	e= b+c;
	e = e+a;
	EXPECT_EQ(d,e) << "not associative";
	EXPECT_EQ(-a+a,i) << "no inverse";


}
TEST_F(pg,G2Add){
	randabcd(G2)
	EXPECT_EQ(a+b,b+a) << "not commutative";
	EXPECT_EQ(a+i,a) << "identity is wrong";
	d= a+b;
	d= d+c;
	e= b+c;
	e = e+a;
	EXPECT_EQ(d,e) << "not associative";
	EXPECT_EQ(-a+a,i) << "no inverse";

}

TEST_F(pg,GTMul){
	randabcd(GT)
	EXPECT_EQ(a*b,b*a) << "not commutative";
	EXPECT_EQ(a*i,a) << "identity is wrong";
	d= a*b;
	d= d*c;
	e= b*c;
	e = e*a;
	EXPECT_EQ(d,e) << "not associative";
	EXPECT_EQ(-a*a,i) << "no inverse";
}

TEST_F(pg,G1Sub){
	randabcd(G1)
	EXPECT_EQ(-(b-a),a-b) << "not anti-commutative";
	EXPECT_EQ(a-i,a) << "identity is wrong";

	EXPECT_EQ(a-a,i) << "no inverse for subtraction";
}
TEST_F(pg,G2Sub){
	randabcd(G2)
	EXPECT_EQ(-(b-a),a-b) << "not anti-commutative";
	EXPECT_EQ(a-i,a) << "identity is wrong";
	EXPECT_EQ(a-a,i) << "no inverse for subtraction";
}


TEST_F(pg,ZRAdd){
	ZR a = group.randomZR();
	ZR b = group.randomZR();
	ZR c = group.randomZR();
	ZR z = 0;	EXPECT_EQ(a+b,b+a) << "not commutative";
	ZR d,e;
	EXPECT_EQ(a+z,a) << "identity is wrong";
	d= a+b;
	d= d+c;
	e= b+c;
	e = e+a;
	EXPECT_EQ(d,e) << "not associative";
	EXPECT_EQ(-a+a,z) << "no inverse";

}

TEST_F(pg,ZRSub){
	ZR a = group.randomZR();
	ZR b = group.randomZR();
	ZR z = 0;
	EXPECT_EQ(-(b-a),a-b) << "not anti-commutative";
	EXPECT_EQ(a-z,a) << "identity is wrong";
	EXPECT_EQ(a-a,z) << "no inverse for subtraction";
}

TEST_F(pg,ZRMul){
	ZR a = 11;
	ZR b = 17;
	ZR c = 17*11;
	EXPECT_EQ(a*b,c);
}

TEST_F(pg,ZRinv){
	ZR a = group.randomZR();
	ZR b = group.randomZR();
	EXPECT_EQ(a*a.inverse(),1);
	EXPECT_EQ(a*b*a.inverse(),b);
	EXPECT_EQ(a*b*b.inverse(),a);
	EXPECT_EQ((a*b)/a,b);
	EXPECT_EQ((a*b)/b,a);

}

TEST_F(pg,ZRDiv){
	ZR a = group.randomZR();
	ZR b = group.randomZR();
	ZR c = a*b;
	ZR d = group.div(a*b,b);
	EXPECT_EQ(d,a);

}

TEST_F(pg,Pair){
	G1 g1 = group.randomG1();
	G2 g2 = group.randomG2();
	ZR r = group.randomZR();
	GT u;
	EXPECT_NE(group.pair(g1,g2),u) << "pairing degenerate";
	GT e1 = group.pair(group.exp(g1,r),g2);
	GT e2 = group.pair(g1,group.exp(g2,r));
	EXPECT_EQ(e1,e2) << "Not bilnelear: e(g1^r,g2)!=e(g1,g2^r)";

}

TEST_F(pg,GTExp){
	GT a,i;
	GT b= group.randomGT();
	gt_get_gen(a.g);

	ZR n = group.order();
	ZR n1;
	gt_get_ord(n1.z);
	EXPECT_EQ(n1,n);
	EXPECT_EQ(group.exp(a,n),i);
	EXPECT_EQ(group.exp(b,n),i);

}
TEST_F(pg,GTInv){
	GT a = group.randomGT();
	GT b = group.randomGT();
	GT u;
	EXPECT_EQ(a*(-a),u);
}

TEST_F(pg,G1group){
	randabcd(G1)
	EXPECT_EQ(group.mul(a,b),a+b);// multiplicitve vs additive notation

}
TEST_F(pg,G2group){
	randabcd(G2)
	EXPECT_EQ(group.mul(a,b),a+b);// multiplicitve vs additive notation
}
TEST_F(pg,GTgroup){
	randabcd(GT)
	EXPECT_EQ(group.mul(a,b),a*b);
}

TEST_F(pg,DISABLED_PairUnity){
	G1 g1;
	G2 g2;
	GT u;
	EXPECT_EQ(group.pair(g1,g2),u) << "known bug. Should be fixed in relic or our wrapper" ;
}

